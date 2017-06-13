/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

/* user-interface: one dialog for each Hantek command */

#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dod.h"
#include "optDevice.h"
#include "optHantek.h"
#include "textHantek.h"

/* ----[ direct.set ] ------------------------------------------------- */

static void setAttn(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: CH1 CH2\n"
	    "CH1,CH2 : MUX_ATTN\n") ;
	return ;
    }

    HantekDlg_MuxAttnId ch1 = optHantek_MuxAttnId(opt) ;
    HantekDlg_MuxAttnId ch2 = optHantek_MuxAttnId(opt) ;
    opt_finish(opt) ;

    hantekDlg_setMux(handle,ch1,ch2) ;
}

static void setFilter(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: CH1 CH2 EXT\n"
	    "CH1,CH2,EXT : BOOL\n"
	    ) ;
	return ;
    }

    HantekDlg_Filter filter ;
    filter.ch1 = opt_bool(opt) ;
    filter.ch2 = opt_bool(opt) ;
    filter.ext = opt_bool(opt) ;
    opt_finish(opt) ;
    
    hantekDlg_setFilter(handle,filter) ;
    /* [todo] find out whether this command has any effect */
}

static void setInput(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf("arguments: INPUT FRAME PRESCALER TRIGGER_COUNT CHANNEL SLOPE\n") ;
	return ;
    }

    HantekDlg_InputId         input = optHantek_InputId      (opt) ;
    HantekDlg_FrameId         frame = optHantek_FrameId      (opt) ;
    HantekDlg_Prescaler   prescaler = optHantek_Prescaler    (opt) ;
    HantekDlg_TriggerCount    count = optHantek_TriggerCount (opt) ;
    HantekDlg_ChannelId     channel = optHantek_ChannelId    (opt) ;
    HantekDlg_SlopeId         slope = optHantek_SlopeId      (opt) ;

    opt_finish(opt) ;
    
    hantekDlg_setInput(handle,input,frame,prescaler,count,channel,slope) ;
}

static void setOffset(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: CH1 CH2 TRIGGER_LEVEL\n"
	    "CH1,CH2 : OFFSET_VOLTAGE\n"
	    ) ;
	
	return ;
    }

    HantekDlg_OffsetVoltage ch1 = optHantek_OffsetVoltage(opt) ;
    HantekDlg_OffsetVoltage ch2 = optHantek_OffsetVoltage(opt) ;
    HantekDlg_TriggerLevel level = optHantek_TriggerLevel(opt) ;
    opt_finish(opt) ;

    hantekDlg_setOffset(handle,ch1,ch2,level) ;
}

static void setRelay(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: CH1 CH2 EXT_INPUT\n"
	    "CH1,CH2   : RELAY_ATTN COUPLING\n"
	    "EXT_INPUT : BOOL\n"
	    ) ;
	return ;
    }

    HantekDlg_RelayAttnId    attn_ch1 = optHantek_RelayAttnId (opt) ;
    HantekDlg_CouplingId coupling_ch1 = optHantek_CouplingId  (opt) ;
    HantekDlg_RelayAttnId    attn_ch2 = optHantek_RelayAttnId (opt) ;
    HantekDlg_CouplingId coupling_ch2 = optHantek_CouplingId  (opt) ;
    HantekDlg_ExtInput            ext = { .enabled = opt_bool(opt) } ;
    
    opt_finish(opt) ;

    hantekDlg_setRelay(handle,attn_ch1,coupling_ch1,attn_ch2,coupling_ch2,ext) ;
}

static void set(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: MODE [help]\n"
	    "MODE : filter  # input filter\n"
	    "     |  input  # miscelleaneous settings\n"
	    "     |    mux  # multiplexer attenuation\n"
	    "     | offset  # offset-volate and trigger-level\n"
	    "     |  relay  # relay-setting and external-input\n"
	    ) ;
	return ;
    }

    char const *arg = opt_pop(opt) ;
    if (false) ;
    
    else if (0 == strcmp(arg,   "attn"))   setAttn(handle,opt) ;
    else if (0 == strcmp(arg, "filter")) setFilter(handle,opt) ;
    else if (0 == strcmp(arg,  "input"))  setInput(handle,opt) ;
    else if (0 == strcmp(arg, "offset")) setOffset(handle,opt) ;
    else if (0 == strcmp(arg,  "relay"))  setRelay(handle,opt) ;
    
    else dod_exit("option not recognized:<%s>",arg) ;
}

/* ----[ direct.get ] ---------------------------------------------- */

static void getCalibration(libusb_device_handle *handle)
{
    HantekDlg_Calibration offsetV = hantekDlg_getCalibration(handle) ;

    printf(
	"                 ch1                     ch2          \n"
	"       ----------------------- -----------------------\n") ;
    for (unsigned rix=0 ; rix<HantekDlg_RelayAttnChoices ; ++rix)
    {
	static char const *name[] = { "  1x"," 10x","100x" } ;
	printf("%s |",name[rix]) ;
	for (unsigned /* channel */ cix=0 ; cix<2 ; ++cix)
	{
	    for (unsigned mix=0 ; mix<HantekDlg_MuxAttnChoices ; ++mix)
	    {
		HantekDlg_OffsetVoltage const *p = offsetV.v[cix][rix][mix] ;
		printf(" (%2x,%2x)",p[0].i,p[1].i) ;
	    }
	}
	printf("\n") ;
    } 
    printf(
	"       ------- ------- ------- ------- ------- -------\n"
	"          1x      2x      5x       1x     2x      5x  \n") ;

    /* for example:
                        ch1                     ch2          
	     ----------------------- -----------------------
         1x | (1d,8f) (10,82) ( b,7c) (16,81) ( a,79) ( 8,77)
        10x | (1e,8f) (11,82) ( b,7c) (13,84) ( a,7b) ( 7,78)
       100x | (1e,8f) (11,82) ( b,7c) (14,85) ( a,7b) ( 7,77)
             ------- ------- ------- ------- ------- -------
                1x      2x      5x       1x     2x      5x  */
}

static void getStatus(libusb_device_handle *handle)
{
    HantekDlg_Status status = hantekDlg_getStatus(handle) ;

    printf(" %2d* ",status.state.e) ;
    for (size_t i=1 ; i<0x100 ; )
    {
	printf("%04x ",status.pointer[i-1]) ;
	if (0 == (++i % 0x10))
	    printf("\n") ;
    }

    /* for example (the first value* is the state):
         0* 00cf 00d1 00d3 00d5 00d7 00d9 00db 00de 00e0 00e2 00e4 00e6 00e8 00ea 00ec 
       00ee 00f0 00f2 00f4 00f6 00f9 00fb 00fd 00ff 0001 0003 0005 0007 0009 000b 000d 
       000f 0012 0014 0016 0018 001a 001c 001e 0020 0022 0024 0026 0028 002b 002d 002f 
       0031 0033 0035 0037 0039 003b 003d 003f 0041 0044 0046 0048 004a 004c 004e 0050 
       0052 0054 0056 0058 005a 005d 005f 0061 0063 0065 0067 0069 006b 006d 006f 0071 
       0073 0076 0078 007a 007c 007e 0080 0082 0084 0086 0088 008a 008c 008f 0091 0093 
       0095 0097 0099 009b 009d 009f 00a1 00a3 00a5 00a8 00aa 00ac 00ae 00b0 00b2 00b4 
       00b6 00b8 00ba 00bc 00be 00c1 00c3 00c5 00c7 00c9 00cb 00cd 00cf 00d1 00d3 00d5 
       00d7 00d9 00dc 00de 00e0 00e2 00e4 00e6 00e8 00ea 00ec 00ee 00f0 00f3 00f5 00f7 
       00f9 00fb 00fd 00ff 0001 0003 0005 0007 0009 000c 000e 0010 0012 0014 0016 0018 
       001a 001c 001e 0020 0022 0024 0027 0029 002b 002d 002f 0031 0033 0035 0037 0039 
       003b 003e 0040 0042 0044 0046 0048 004a 004c 004e 0050 0052 0054 0056 0059 005b 
       005d 005f 0061 0063 0065 0067 0069 006b 006d 0070 0072 0074 0076 0078 007a 007c 
       007e 0080 0082 0084 0086 0088 008b 008d 008f 0091 0093 0095 0097 0099 009b 009d 
       009f 00a1 00a4 00a6 00a8 00aa 00ac 00ae 00b0 00b2 00b4 00b6 00b8 00ba 00bd 00bf 
       00c1 00c3 00c5 00c7 00c9 00cb 00cd 00cf 00d1 00d3 00d6 00d8 00da 00dc 00de 00e0 */
}

static void get(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf("arguments: calibration | status\n") ;
	return ;
    }

    char const *arg = opt_pop(opt) ; opt_finish(opt) ;
    if (false) ;
    
    else if (0 == strcmp(arg,"calibration")) getCalibration(handle) ;
    else if (0 == strcmp(arg,     "status"))      getStatus(handle) ;
    
    else dod_exit("option not recognized:<%s>",arg) ;
}

/* ----[ direct.fetch ] ----------------------------------------------- */

static void fetch(libusb_device_handle *handle,Opt *opt)
{
    if (opt_popIf(opt,"help"))
    {
	printf("arguments: [-n NBYTES]\n") ;
	return ;
    }

    long long unsigned i = dod_strtoull(opt_preset(opt,"-n","0x200")) ;
    size_t n = (size_t)i ;
    if (i != n)
	dod_exit("size_t's domain exceeded:%llu",i) ;
    opt_finish(opt) ;

    uint8_t *buffer = dod_malloc(n) ;
    hantekDlg_fetch(handle,buffer,n) ;
    
    size_t nwritten = fwrite(buffer,1,n,stdout) ;
    if (nwritten != n)
	dod_exit("fwrite(%zu):an error occurred after writing %zu bytes",n,nwritten) ;

    free(buffer) ;
    /* [todo] If fetching while data-capture is in progress, the next
       status query returns weird results: The first 38 bytes (state +
       18x pointer) hold random (?) data. This happens with small and
       large frames. With tiny frames however, the status query is ok. */
}

/* ----[ direct.trigger ] ------------------------------------------ */

static void trigger(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf("arguments: enable | release\n") ;
	return ;
    }

    size_t choice = opt_choice(opt,"enable","release",NULL) ;
    opt_finish(opt) ;

    switch (choice)
    {
    case 0:  hantekDlg_enableTrigger(handle) ; break ;
    case 1: hantekDlg_releaseTrigger(handle) ; break ;
    default: assert(false) ;
    }
}

/* ----[ direct.capture ] --------------------------------------------- */

static void capture(libusb_device_handle *handle,Opt *opt)
{
    if (opt_popIf(opt,"help"))
    {
	printf("no arguments supported\n") ;
	return ;
    }
    opt_finish(opt) ;
    hantekDlg_capture(handle) ;
}

/* ----[ ui.direct ] -------------------------------------------------- */

void ui_direct(libusb_context *libusb,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf("arguments: DEVICE MODE [help]\n"
	       "\n"
	       "MODE : capture  # tell device to capture data\n"
	       "     | fetch    # transfer frame from device to host\n"
	       "     | get      # query status or calibration\n"
	       "     | set      # tell device to change configuration\n"
	       "     | trigger  # tell device to arm or release trigger\n") ;
	return ;
    }
    libusb_device_handle *handle = optDevice_dso2090(opt,libusb) ;
    
    char const *arg = opt_pop(opt) ;
    if (false) ;

    else if (0 == strcmp(arg, "capture")) capture(handle,opt) ;
    else if (0 == strcmp(arg,   "fetch"))   fetch(handle,opt) ;
    else if (0 == strcmp(arg,     "get"))     get(handle,opt) ;
    else if (0 == strcmp(arg,     "set"))     set(handle,opt) ;
    else if (0 == strcmp(arg, "trigger")) trigger(handle,opt) ;
    
    else dod_exit("option not recognized:<%s>",arg) ;

    libusb_close(handle) ;
}
