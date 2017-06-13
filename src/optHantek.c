/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "optHantek.h"
#include <string.h>
#include "dod.h"
#include "textHantek.h"

HantekDlg_AdcId optHantek_AdcId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_AdcId h = textHantek_AdcId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:AdcId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_ChannelId optHantek_ChannelId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_ChannelId h = textHantek_ChannelId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:ChannelId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_CouplingId optHantek_CouplingId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_CouplingId h = textHantek_CouplingId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:CouplingId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_FrameId optHantek_FrameId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_FrameId h = textHantek_FrameId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:FrameId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_InputId optHantek_InputId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_InputId h = textHantek_InputId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:InputId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_MuxAttnId optHantek_MuxAttnId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_MuxAttnId h = textHantek_MuxAttnId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:MuxAttnId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_OffsetVoltage optHantek_OffsetVoltage(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_OffsetVoltage h = textHantek_OffsetVoltage(&text) ;
    if (!text_end(&text))
	dod_exit("opt:OffsetVoltage:trailing characters:<%s>",s) ;
    return h ;
}
    
HantekDlg_Prescaler optHantek_Prescaler(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_Prescaler h = textHantek_Prescaler(&text) ;
    if (!text_end(&text))
	dod_exit("opt:Prescaler:trailing characters:<%s>",s) ;
    return h ;
}
    
HantekDlg_RelayAttnId optHantek_RelayAttnId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_RelayAttnId h = textHantek_RelayAttnId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:RelayAttnId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_TriggerCount optHantek_TriggerCount(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_TriggerCount h = textHantek_TriggerCount(&text) ;
    if (!text_end(&text))
	dod_exit("opt:TriggerCount:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_SlopeId optHantek_SlopeId(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_SlopeId h = textHantek_SlopeId(&text) ;
    if (!text_end(&text))
	dod_exit("opt:SlopeId:trailing characters:<%s>",s) ;
    return h ;
}

HantekDlg_TriggerLevel optHantek_TriggerLevel(Opt *opt)
{
    char const *s = opt_pop(opt) ;
    Text text = text_init(s,strlen(s)) ;
    HantekDlg_TriggerLevel h = textHantek_TriggerLevel(&text) ;
    if (!text_end(&text))
	dod_exit("opt:TriggerLevel:trailing characters:<%s>",s) ;
    return h ;
}
