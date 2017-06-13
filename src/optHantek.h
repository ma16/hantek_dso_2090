/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_optHantek_h
#define INCLUDE_optHantek_h

/* read Hantek-specific command line options */

#include <libusb.h>
#include "hantekDlg.h"
#include "opt.h"

HantekDlg_AdcId         optHantek_AdcId         (Opt *opt) ;
HantekDlg_ChannelId     optHantek_ChannelId     (Opt *opt) ;
HantekDlg_CouplingId    optHantek_CouplingId    (Opt *opt) ;
HantekDlg_FrameId       optHantek_FrameId       (Opt *opt) ;
HantekDlg_InputId       optHantek_InputId       (Opt *opt) ;
HantekDlg_MuxAttnId     optHantek_MuxAttnId     (Opt *opt) ;
HantekDlg_OffsetVoltage optHantek_OffsetVoltage (Opt *opt) ;
HantekDlg_Prescaler     optHantek_Prescaler     (Opt *opt) ;
HantekDlg_RelayAttnId   optHantek_RelayAttnId   (Opt *opt) ;
HantekDlg_SlopeId       optHantek_SlopeId       (Opt *opt) ;
HantekDlg_TriggerCount  optHantek_TriggerCount  (Opt *opt) ;
HantekDlg_TriggerLevel  optHantek_TriggerLevel  (Opt *opt) ;

#endif /* INCLUDE_optHantek_h */
