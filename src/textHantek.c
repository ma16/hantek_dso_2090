/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "textHantek.h"
#include <assert.h>
#include <math.h>
#include <stdio.h> /* debugging */
#include <string.h>
#include "dod.h"

HantekDlg_AdcId textHantek_AdcId(Text *text)
{
    int e = (int)text_choice(text,"0","1",NULL) ;
    HantekDlg_AdcId h = { .e = e } ; return h ;
}

HantekDlg_ChannelId textHantek_ChannelId(Text *text)
{
    int e = (int)text_choice(text,"ch2","ch1",NULL) ;
    HantekDlg_ChannelId h = { .e = e } ; return h ;
}

HantekDlg_CouplingId textHantek_CouplingId(Text *text)
{
    int e = (int)text_choice(text,"ac","dc",NULL) ;
    HantekDlg_CouplingId h = { .e = e } ; return h ;
}

HantekDlg_FrameId textHantek_FrameId(Text *text)
{
    int e = (int)text_choice(text,
			     "tiny" ,"256",
			     "small","10k",
			     "large","32k",
			     NULL) ;
    HantekDlg_FrameId h = { .e = e/2 } ; return h ;
}

HantekDlg_InputId textHantek_InputId(Text *text)
{
    int e = (int)text_choice(text,"ch1","ch2","both","none",NULL) ;
    HantekDlg_InputId h = { .e = e } ; return h ;
}

HantekDlg_MuxAttnId textHantek_MuxAttnId(Text *text)
{
    int e = (int)text_choice(text,"1x","2x","5x",NULL) ;
    HantekDlg_MuxAttnId h = { .e = e } ; return h ;
}

HantekDlg_OffsetVoltage textHantek_OffsetVoltage(Text *text)
{
    long long unsigned i = text_llu(text) ;
    if (i > 0xffff)
	dod_exit("text:OffsetVoltage:16-bit value expected") ;
    return (HantekDlg_OffsetVoltage) {(uint16_t)i} ;
}
    
HantekDlg_Prescaler textHantek_Prescaler(Text *text)
{
    HantekDlg_Prescaler h ;

    if (1 == text_ifChoice(text,"=",NULL))
    {
	h.e = HantekDlg_Prescaler_ByDivider ;
	unsigned long long i = text_llu(text) ;
	if (i >= 0x10000)
	    dod_exit("text:prescaler <%llu> too big",i) ;
	h.u.divider = (uint16_t)i ;
    }
    
    unsigned long long e = text_ifChoice(text,"max","50/1","50/2","50/5",NULL) ;
    if (e > 0)
    {
	h.e = HantekDlg_Prescaler_ById ;
	h.u.id.e = (e-1) ;
	return h ;
    }

    if (1 == text_ifChoice(text,"25/",NULL))
    {
	h.e = HantekDlg_Prescaler_ByDivider ;
	unsigned long long i = text_llu(text) ;
	if (i < 2 || 65537 < i)
	    dod_exit("divider <%llu> must be in the range 2..65537",i) ;
	h.u.divider = (uint16_t) ~(i-2) ;
	return h ;
    }

    double f = text_double(text) ;
    if (1 == text_ifChoice(text,"s",NULL))
    {
	h.e = HantekDlg_Prescaler_ByDivider ;
	static double const min = 1.0/(25.0e+6/    2) ;
	static double const max = 1.0/(25.0e+6/65537) ;
	if (f < min || max < f)
	    dod_exit("interval <%.2e> must be in the range (%.2e,%.2e)",f,min,max) ;
	uint16_t div = (uint16_t)floor(25e+6 * f + -2 + .5) ;
	h.u.divider = (uint16_t)~div ;
	return h ;
    }
    
    h.e = HantekDlg_Prescaler_ByDivider ;
    static double const min = 25.0e+6/65537 ;
    static double const max = 25.0e+6/    2 ;
    if (f < min || max < f)
	dod_exit("frequency %.2e is not in the range (%.2e,%.2e)",f,min,max) ;
    uint16_t div = (uint16_t)floor(25e+6 / f + -2 + .5) ;
    h.u.divider = (uint16_t)~div ;
    return h ;
}

HantekDlg_RelayAttnId textHantek_RelayAttnId(Text *text)
{
    int e = (int)text_choice(text,"1x","10x","100x",NULL) ;
    HantekDlg_RelayAttnId h = { .e = e } ; return h ;
}

HantekDlg_TriggerCount textHantek_TriggerCount(Text *text)
{
    HantekDlg_TriggerCount count ;

    if (1 == text_ifChoice(text,"=",NULL))
    {
	long long unsigned i = text_llu(text) ;
	if ((1u<<24) <= i)
	  dod_exit("text:invalid 24-bit trigger-count:<%.*s>",(int)text->len,text->p) ;
	count.i = (uint32_t)i ;
	return count ;
    }

    unsigned e = text_ifChoice(text,
			       "tiny", "256",
			       "small","10k",
			       "large","32k",
			       NULL) ;
    if (e > 0)
    {
	switch ((e-1)/2)
	{
	case 0: return hantekDlg_triggerCount(    256) ;
	case 1: return hantekDlg_triggerCount(10*1024) ;
	case 2: return hantekDlg_triggerCount(32*1024) ;
	}
	assert(false) ; abort() ;
    }

    long long unsigned i = text_llu(text) ;
    if (i<1 || 0x8000<i)
	dod_exit("text:trigger count <%llu> must be in range (1,0x8000)",i) ;
	
    return hantekDlg_triggerCount((uint16_t)i) ;
}

HantekDlg_SlopeId textHantek_SlopeId(Text *text)
{
    int e = (int)text_choice(text,"rise","fall",NULL) ;
    HantekDlg_SlopeId h = { .e = e } ; return h ;
}

HantekDlg_TriggerLevel textHantek_TriggerLevel(Text *text)
{
    long long unsigned i = text_llu(text) ;
    if (i > 0xffff)
	dod_exit("text:TriggerLevel:16-bit value expected") ;
    return (HantekDlg_TriggerLevel) {(uint16_t)i} ;
}

char const* textHantek_help()
{
    static char const s[] =
	"ADC: two analog-to-digital converters\n"
	"    '0'  # ADC-0\n"
	"    '1'  # ADC-1\n"
	"\n"
	"CHANNEL: two input channels\n"
	"    'ch1'  # channel-1\n"
	"    'ch2'  # channel-2\n" 
	"\n"
	"COUPLING: per channel\n"
	"    'ac'  # alternating current\n"
	"    'dc'  #      direct current\n" 
	"\n"
	"FRAME: three frame sizes are available\n"
	"     '256' |   '10k' |   '32k'\n" 
	"    'tiny' | 'small' | 'large'\n" 
	"\n"
	"INPUT: define ADC's signal source\n"
	"    'ch1'   # channel-1 goes to both ADCs\n"
	"    'ch2'   # channel-2 goes to both ADCs\n"
	"    'both'  # channel-1 to ADC-1, channel-2 to ADC-0\n"
	"    'none'\n" 
	"\n"
	"MUX_ATTN: attenuation switched by multiplexer\n"
	"    '1x' | '2x' | '5x'\n" 
	"\n"
	"OFFSET_VOLTAGE: raise channel's signal voltage\n"
	"    0..0xFFFF\n" 
	"\n"
	"PRESCALER: set sample-rate\n"
	"    'max'                     # 50 MHz\n"
	"    '50/1'                    # 50 MHz\n"
	"    '50/2'                    # 25 MHz\n"
	"    '50/5'                    # 10 MHz\n"
	"    '25/' 2..65537            # in MHz\n"
	"    3.81e+02 .. 1.25e+07      # in MHz\n"
	"    8.00e-08 .. 2.62e-03 's'  # as f := 1/s\n"
	"    '=' 0..0xFFFF             # internal encoding\n"  
	"\n"
	"RELAY_ATTN : attenuation switched by relays\n"
	"    '1x' | '10x' | '100x'\n" 
	"\n"
	"TRIGGER_COUNT: (should match FRAME size)\n"
	"    1..0x8000\n"
	"     '256' |   '10k' |   '32k'\n" 
	"    'tiny' | 'small' | 'large'\n"
	"    '=' 0..0xFFFFFF\n" 
	"\n"
	"SLOPE: to release the trigger\n"
	"    'rise' | 'fall'\n" 
	"\n"
	"TRIGGER_LEVEL: to release the trigger\n"
	"    0..0xFFFF\n" ;

    return s ;
}
