/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "hantekExt.h"
#include <string.h>
#include "dod.h"
#include "systime.h"
#include "textHantek.h"

/* expect '$tag' */
static void expect(Text *text,char const *tag)
{
    text_choice(text,tag,NULL) ;
}

/* expect '$tag' '=' */
static void prolog(Text *text,char const *tag)
{
    text_clear (text) ;
    expect     (text,tag) ;
    text_clear (text) ;
    expect     (text,"=") ;
    text_clear (text) ;
}

static HantekExt_ChannelConfig scanChannel(Text *text,char const *tag)
{
    HantekExt_ChannelConfig c ;
    
    text_clear(text) ;
    expect(text,tag) ;
    text_clear(text) ;
    
    expect(text,"{") ; 
    
    prolog(text,"coupling") ;
    c.coupling = textHantek_CouplingId(text) ;
    text_space(text) ;
    
    prolog(text,"filter") ;
    c.filter = text_bool(text) ;
    text_space(text) ;
    
    prolog(text,"mux") ;
    c.muxAttn = textHantek_MuxAttnId(text) ;
    text_space(text) ;
    
    prolog(text,"offset") ;
    c.offset = textHantek_OffsetVoltage(text) ;
    text_space(text) ;
    
    prolog(text,"relay") ;
    c.relayAttn = textHantek_RelayAttnId(text) ;
    
    text_clear(text) ;
    expect(text,"}") ;
    
    return c ;
}

static HantekExt_TriggerConfig scanTrigger(Text *text)
{
    HantekExt_TriggerConfig c ;
    
    text_clear(text) ;
    expect(text,"trigger") ;
    text_clear(text) ;

    expect(text,"{") ; 
    
    prolog(text,"channel") ;
    c.channel = textHantek_ChannelId(text) ;
    text_space(text) ;
    
    prolog(text,"count") ;
    c.count = textHantek_TriggerCount(text) ;
    text_space(text) ;
    
    prolog(text,"ext") ;
    c.ext.enabled = text_bool(text) ;
    text_space(text) ;
    
    prolog(text,"level") ;
    c.level= textHantek_TriggerLevel(text) ;
    text_space(text) ;
    
    prolog(text,"slope") ;
    c.slope = textHantek_SlopeId(text) ;

    text_clear(text) ;
    expect(text,"}") ;
    
    return c ;
}

HantekExt_Config hantekExt_loadConfig(char const *path)
{
    char buffer[0x1000] ;
    size_t len = dod_loadFile(path,buffer,sizeof(buffer)) ;
    Text text = text_init(buffer,len) ;
    HantekExt_Config c ;

    prolog(&text,"frame") ;
    c.frame = textHantek_FrameId(&text) ;
    text_space(&text) ;
    
    prolog(&text,"input") ;
    c.input = textHantek_InputId(&text) ;
    text_space(&text) ;
    
    prolog(&text,"rate") ;
    c.prescaler = textHantek_Prescaler(&text) ;
    text_space(&text) ;

    c.channel[0] = scanChannel(&text,"ch1") ;
    c.channel[1] = scanChannel(&text,"ch2") ;

    c.trigger = scanTrigger(&text) ;

    prolog(&text,"ext.filter") ;
    c.extFilter = text_bool(&text) ;
    
    text_clear(&text) ;
    text_finish(&text) ;
    
    return c ;
}

void hantekExt_xferConfig(
    libusb_device_handle *handle,HantekExt_Config const *config)
{
    HantekDlg_Filter filter =
    {
	.ch1 = config->channel[0].filter,
	.ch2 = config->channel[1].filter,
	.ext = config->extFilter,
    } ;
    hantekDlg_setFilter(handle,filter) ;

    hantekDlg_setInput(
	handle,
        config->          input,
	config->          frame,
	config->      prescaler,
	config->trigger.  count,
	config->trigger.channel,
	config->trigger.  slope) ;
    
    hantekDlg_setMux(
	handle,
	config->channel[0].muxAttn,
	config->channel[1].muxAttn) ;
    
    hantekDlg_setOffset(
	handle,
	config->channel[0].offset,
	config->channel[1].offset,
	config->trigger.level) ;
    
    hantekDlg_setRelay(
	handle,
	config->channel[0].relayAttn,
	config->channel[0]. coupling,
	config->channel[1].relayAttn,
	config->channel[1]. coupling,
	config->trigger.ext) ;
}

static void scanCalibration(
    Text *text,const char *tag,HantekDlg_OffsetVoltage (*v)[3][3])
{
    text_clear(text) ; expect(text,tag) ; text_clear(text) ; expect(text,"{") ;
								    
    for (unsigned /* relay */ rix=0 ; rix<3 ; ++rix)
    {
	for (unsigned /* mux */ mix=0 ; mix<3 ; ++mix)
	{
	    prolog(text,"offset") ;
	    (*v)[rix][mix] = textHantek_OffsetVoltage(text) ;
	    text_space(text) ;
	}
    }
    expect(text,"}") ; 
}

HantekExt_Calibration hantekExt_loadCalibration(char const *path)
{
    char buffer[0x1000] ;
    size_t len = dod_loadFile(path,buffer,sizeof(buffer)) ;
    Text text = text_init(buffer,len) ;
    HantekExt_Calibration c ;

    scanCalibration(&text,"ch1",&c.v[0]) ;
    scanCalibration(&text,"ch2",&c.v[1]) ;

    text_clear(&text) ;
    text_finish(&text) ;
    
    return c ;
}

HantekDlg_Status hantekExt_capture(libusb_device_handle *handle,bool release)
{
    /* device parameters including trigger-counter 
       must have been set-up by the caller beforehand */
    hantekDlg_capture(handle) ;
    hantekDlg_enableTrigger(handle) ;
    if (release)
      hantekDlg_releaseTrigger(handle) ;
    HantekDlg_Status status = hantekDlg_getStatus(handle) ;
    while (status.state.e != HantekDlg_State_Ready)
    {
	/* this may take a while depending on the 
	   configured sample-rate and trigger-position */
	if (status.state.e != HantekDlg_State_PostTrigger)
	{
	    /* [defect] what can cause this? it happened in 
	       roll-mode, couldn't be replicated though */
	    dod_exit("state <%d> after released trigger",status.state.e) ;
	}
	/* [todo] we might want to provide some feed-back if blocked */
	systime_sleep(1e-3) ; /* prevent busy loop */
	status = hantekDlg_getStatus(handle) ;
	/* [todo] we might wanna add some time-out here */
    }
    return status ;
    /* [defect] The pointer[0] may be beyond the frame-size. This 
       happens when switching from a larger frame to a smaller frame 
       since the pointer appears to remain in the larger frame until 
       wrap-around (replicated). Also, this defect occurs sometimes 
       even if the frame-size got not switched which is troubling. */
    
}
