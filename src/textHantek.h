/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_textHantek_h
#define INCLUDE_textHantek_h

/* scan Hantek variables */

#include "text.h"
#include "hantekDlg.h"

char const* textHantek_help(void) ;
  
HantekDlg_AdcId         textHantek_AdcId         (Text *text) ;
HantekDlg_ChannelId     textHantek_ChannelId     (Text *text) ;
HantekDlg_CouplingId    textHantek_CouplingId    (Text *text) ;
HantekDlg_FrameId       textHantek_FrameId       (Text *text) ;
HantekDlg_InputId       textHantek_InputId       (Text *text) ;
HantekDlg_MuxAttnId     textHantek_MuxAttnId     (Text *text) ;
HantekDlg_OffsetVoltage textHantek_OffsetVoltage (Text *text) ;
HantekDlg_Prescaler     textHantek_Prescaler     (Text *text) ;
HantekDlg_RelayAttnId   textHantek_RelayAttnId   (Text *text) ;
HantekDlg_SlopeId       textHantek_SlopeId       (Text *text) ;
HantekDlg_TriggerCount  textHantek_TriggerCount  (Text *text) ;
HantekDlg_TriggerLevel  textHantek_TriggerLevel  (Text *text) ;

#endif /* INCLUDE_textHantek_h */
