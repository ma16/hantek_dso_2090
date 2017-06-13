/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_hantekExt_h
#define INCLUDE_hantekExt_h

/* convenience functions that are based on hantekDlg */

#include <stdbool.h>
#include "hantekDlg.h"

/* gather all the device configuration parameters... */
typedef struct
{
    HantekDlg_CouplingId   coupling ;
    bool                     filter ;
    HantekDlg_MuxAttnId     muxAttn ;
    HantekDlg_OffsetVoltage  offset ;
    HantekDlg_RelayAttnId relayAttn ;
}
HantekExt_ChannelConfig ;

typedef struct
{
    HantekDlg_ChannelId  channel ;
    HantekDlg_TriggerCount count ;
    HantekDlg_ExtInput       ext ;
    HantekDlg_TriggerLevel level ;
    HantekDlg_SlopeId      slope ;
}
HantekExt_TriggerConfig ;

typedef struct
{
    HantekDlg_FrameId            frame ;
    HantekDlg_InputId            input ;
    HantekDlg_Prescaler      prescaler ;
    HantekExt_ChannelConfig channel[2] ;
    HantekExt_TriggerConfig    trigger ;
    bool                     extFilter ;
}
HantekExt_Config ;

/* ...to load together from file and... */
HantekExt_Config hantekExt_loadConfig(char const *path) ;

/* ...to set-up the device with a single call */
void hantekExt_xferConfig(libusb_device_handle*,HantekExt_Config const*) ;

/* gather offset voltage (calibration)... */
typedef struct
{
    HantekDlg_OffsetVoltage v[2][3][3] ;
    /* index 0: (ch1,ch2)
       index 1: relays attenuation: (1,10,100)
       index 2: multiplexer attenuation: (1,2,5) */
}
HantekExt_Calibration ;

/* ...to load together from file */
HantekExt_Calibration hantekExt_loadCalibration(char const *path) ;

/* make device capture a full (trigger-count) sample */
HantekDlg_Status hantekExt_capture(libusb_device_handle *handle,bool release) ;

#endif /* INCLUDE_hantekExt_h */
