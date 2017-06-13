/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_hantekDlg_h
#define INCLUDE_hantekDlg_h

/* protocol (dialog) between host and Hantek device */

#include <assert.h>
#include <stdbool.h>
#include "usb.h"
#include "usbExt.h"

/* --------------------------------------------------------------------
 * List of Hantek's USB Commands (protocol dialogs)
 * -------------------------------------------------------------------- */

/*
Control Commands (the index is the USB's _bRequest_ field):

-- A2 query calibration
-- B3 setup (used as prolog for all bulk commands)
-- B4 configure offset-voltage & trigger-level
-- B5 configure relay-attenuation, input-coupling & ext-input

Bulk Commands (the index is the first byte of the packet):

-- 00 configure filter
-- 01 configure channel-input, frame-type, prescaler, trigger-count,
                trigger-channel & trigger-slope
-- 02 release trigger
-- 03 start capturing of data
-- 04 enable trigger
-- 05 fetch frame (transfer from device to host)
-- 06 query status
-- 07 configure multiplexer-attenuation
*/

/* -------------------------------------------------------------------- 
 * Data Acquisition                                      
 * -------------------------------------------------------------------- */

/* (1) begin capturing of data */
void hantekDlg_capture(libusb_device_handle*) ;
/*
The captured data (i.e. records with two ADC values) are written into a 
"frame". The frame is a ring-buffer. That is, the device maintains a 
pointer to where the last sampled record was written to. The pointer 
is incremented before each write and wraps around to the begin when the 
end of the frame is reached. Previously sampled values are going to be 
overwritten.
*/

/* (2) arm trigger */
void hantekDlg_enableTrigger(libusb_device_handle*) ;

/* 
Before the "trigger" can be released, it needs to be enabled.
The trigger may then be released either by command or by the 
incoming signal. 
*/

/* (3) release trigger (by command) */
void hantekDlg_releaseTrigger(libusb_device_handle*) ;

/*
The device continues to capture data after the trigger has been 
released. However, once the trigger is released, the device starts the 
trigger-counter. The device samples exactly as many records as 
configured for the trigger-counter and stop capturing thereafter.
*/

/* (4) transfer the frame from the device to the host */
void hantekDlg_fetch(libusb_device_handle*,uint8_t *buffer,size_t nbytes) ;

/* 
The frame can be read by the host at any time. Due to the nature of a 
ring-buffer, the host needs to determine at which offset the most 
recent captured record is located. This is difficult if sampling is 
still in progress: There is a time gap between querying the pointer to 
the most recent record and the time when actually reading the frame. 
The pointer will have moved. 

There appears to be no dedicated command to stop sampling. The only way 
to do so seems to release the trigger and wait for the trigger-counter 
until data capturing has finished (and so the pointer stops moving).
*/

/* --------------------------------------------------------------------
 * ADC and Frame
 * -------------------------------------------------------------------- */

/*
The DSO employs two analog-digital-converter (ADC) to sample data. 
*/

typedef struct
{
    enum
    {
	HantekDlg_AdcId_0 = 0,
	HantekDlg_AdcId_1 = 1,
    } e ;
}
HantekDlg_AdcId ;

/*
Each ADC provides an 8-bit resolution (256 levels) which is exactly 
one byte. For example, if the  voltage range is 8 Vpp, each level 
respresents 31.25 mV.

[todo] does 0xff represent 8V or 7.968,75V?

The two ADC bytes are placed into a record. 

  -- The ADC # 0 value occupies the  first byte (at record-index 0). 
  -- The ADC # 1 value occupies the second byte (at record-index 1). 

The records are written into a "frame". The frame-size is configurable. 
Three different frame sizes are available:
*/

typedef struct
{
    enum
    {
	HantekDlg_FrameId_Tiny  = 0, 
	HantekDlg_FrameId_Small = 1, 
	HantekDlg_FrameId_Large = 2, 
    } e ;
}
HantekDlg_FrameId ;

/* return the frame-size in records, each record occupies 2 bytes */
static inline uint16_t hantekDlg_frameSize(HantekDlg_FrameId id)
{
    switch (id.e)
    {
    case HantekDlg_FrameId_Tiny  : return       256;
    case HantekDlg_FrameId_Small : return 10 * 1024 ;
    case HantekDlg_FrameId_Large : return 32 * 1024 ;
    }
    assert(false) ; abort() ;
}

static uint32_t const hantekDlg_maxFrameSize = 32 * 1024 ;

/*
The host can query the device at any time to transfer the frame. The 
transfer starts at the lowest frame address regardless the position of
the record pointer.

Let's assume the host does only want to know the last sampled record: 
Since the record pointer can be at any position, it may be necessary to 
transfer the whole frame. In those situations it might be suitable to 
chose a tiny frame-size beforehand. 
*/

/* --------------------------------------------------------------------
 * The Device Status
 * -------------------------------------------------------------------- */

/*
The device can be queried for its current status at any time. There are
the device state and the pointer to the last sampled record.
*/

typedef struct
{
    enum
    {
	/* sampling in progress; trigger not released */
	HantekDlg_State_PreTrigger  = 0,
	/* sampling in progress; trigger released; counting records */
	HantekDlg_State_PostTrigger = 1,
	/* counting finished; sampling stopped */
	HantekDlg_State_Ready       = 2,
    } e ;
}
HantekDlg_State ;

typedef struct 
{
    HantekDlg_State  state ;
    uint16_t pointer[0xff] ;
}
HantekDlg_Status ;

/* 
There are actually 255 pointer values. The first is the pointer to the 
record with the most recent sample. Subsequent pointers appear to 
decay; each with approximately 4ns increment into the past. 
*/

HantekDlg_Status hantekDlg_getStatus(libusb_device_handle *handle) ;

/*
[caveats] 
If a frame is transferred while sampling is still in progress, a 
subsequent query may deliver weird state values (>128). This appears
only to happen for none-tiny-frames.

[caveats] 
If we switch from a larger to a smaller frame, the pointer values may 
still remain in the domain of the larger frame (until wrapped around). 
Is this only a transmission problem or are the samples actually located 
beyond reach of the smaller frame? [todo]
*/

/* --------------------------------------------------------------------
 * The Trigger Count
 * -------------------------------------------------------------------- */

/* 
The implementation of the counter is somewhat peculiar: As soon as the 
trigger is released, the counter increments with each sampled record 
until it reaches the value 0x8,0000 (2^19). At this moment the sampling 
is stopped (the record pointer doesn't change anymore) and the capture 
state becomes Ready. 

So the counter may be (much) higher than there are actual records in 
the frame. In the worst case (with ~380/s and counter=0) it takes 
1,379 seconds until the sampling stops.
*/

typedef struct
{
    uint32_t i ; /* lower 24-bit are sent; lower 19-bit are effective */
}
HantekDlg_TriggerCount ;

static HantekDlg_TriggerCount const hantekDlg_minTriggerCount = { 0x7ffff } ; 
/* ...to sample one record; 
   it appears there is no way to sample 0 records after release */

/* for convenience: arm the counter with a sensible 
   value that is within possible frame-sizes 1..0x8000 */
static inline HantekDlg_TriggerCount hantekDlg_triggerCount(uint16_t count)
{
    return (HantekDlg_TriggerCount) { 0x80000u - count } ;
}

/* --------------------------------------------------------------------
 * The Signal Source (Input)
 * -------------------------------------------------------------------- */

/* 
There are two ADCs and two input-channels. As default, Channel:1 feeds 
ADC:1 and Channel:2 feeds Adc:0 (!) However, it is also possible to
feed both ADCs by Channel:1 or both ADCs by Channel:2.
*/

typedef struct
{
    enum
    {
	HantekDlg_InputId_Ch1  = 0, /* Channel:1->ADC:0+1 */
	HantekDlg_InputId_Ch2  = 1, /* Channel:2->ADC:0+1 */
	HantekDlg_InputId_Both = 2, /* Channel:1->ADC:1,Channel:2->ADC:0 */
	HantekDlg_InputId_None = 3, /* no channel */
    } e ;
}
HantekDlg_InputId ; /* channels-to-acquire-data-from */

/* --------------------------------------------------------------------
 * The Sample Rate (Prescaler)
 * -------------------------------------------------------------------- */

/* 
The ADC supports a maximum sample-rate of 50 mega-samples-per-second 
(MS/s). A prescaler is available to lower the sample-rate. However, 
both ADCs sample always at the same rate. 

There are actually two types of prescaler. 

The first type provides just three fixed sample rates: 50 MS/s,25 MS/s 
and 10 MS/s:
*/

typedef struct
{
    enum
    {
	HantekDlg_PrescalerId_Max   = 0, /* 50 MS/s */ 
	HantekDlg_PrescalerId_Base1 = 1, /* 50 MS/s */
	HantekDlg_PrescalerId_Base2 = 2, /* 25 MS/s */
	HantekDlg_PrescalerId_Base5 = 3, /* 10 MS/s */
    } e ;
/*
The identifiers 0 and 1 are used for the sample-rate by the DSO-2090. 
Other sample-rates may be available on other Hantek DSOs.
*/
}
HantekDlg_PrescalerId ;

/* 
The second type of prescaler employs a divider in the range of 2 to 
65,537. It uses a 25 MHz clock source. Possible sample-rates are:

    divider =     2 -> resulting sample_rate = 1.25e+7
    divider =     3 -> resulting sample_rate = 8.33e+6
    divider =     4 -> resulting sample_rate = 6.25e+6
    divider =     5 -> resulting sample_rate = 5.00e+6
    ...
    divider = 65537 -> resulting sample-rate = 3.81e+2

The 2^16 different divider values are somewhat [peculiar] encoded as
follows:

    divider := ~ ( base / 2 / sample_rate ) - 2)

    where 
    * "base" corresponds to HantekDlg_PrescalerId_Base1 (i.e. 50 MHz)
    * "~" is the operator to inverse all bits

    For example:

        0xffff := ~ (5.00e+7 / 2 / 1.25e+7 ) - 2)
        0xfffe := ~ (5.00e+7 / 2 / 8.33e+6 ) - 2)
        0xfffd := ~ (5.00e+7 / 2 / 6.25e+6 ) - 2)
        0xfffc := ~ (5.00e+7 / 2 / 5.00e+6 ) - 2)
	...
        0x0000 := ~ (5.00e+7 / 2 / 3.81e+2 ) - 2) 

The client has to chose which type of prescaler to use:
*/

typedef struct
{
    enum
    {
	HantekDlg_Prescaler_ById      = 0,
	HantekDlg_Prescaler_ByDivider = 1,
    } e ;
    union
    {
	HantekDlg_PrescalerId id ;
	uint16_t         divider ;
    } u ;
}
HantekDlg_Prescaler ;


/*
Both ADC sample at the same rate. However, it seems there is a slight
displacement _when_ the sample is taken. The displacement between ADC:0
and ADC:1 appears to be about 10ns regardless whether both ADC sample
the signal of the same or different channels [todo]. This enables an 
effective sampling rate of 100 MHz when both ADC sample the same signal
source at 50 MHz.

[caveats] 
OpenHantek uses "fastRate" switch. This switch however seems to have
no for the DSO-2090 that was used in a test.

[caveats] 
A comment in the OpenHantek sources suggest that "the sampling-rate 
is divided by 1000" for tiny frames. However, that seems not to be true 
for the DSO-2090 that was used in a test. 
*/

/* --------------------------------------------------------------------
 * The Signal Attenuation (gain control)
 * -------------------------------------------------------------------- */

/* 
The device supports 9 voltage ranges. The smallest range is 80 mVpp
which is 10 mV/div. So the smallest sample resolution (level) is about 
0.3 mV. 

The effective voltage range is configured by two relays and a 4052 
analog multiplexer. The relays are used to engage or bypass two /10 
attenuators into the signal path. A resistor divider network is used 
to implement another /1 /2 /5 attenuation sequence which is tapped by 
the multiplexer. 
*/

typedef struct
{
    enum
    {
	HantekDlg_RelayAttnId_1x   = 0,
	HantekDlg_RelayAttnId_10x  = 1,
	HantekDlg_RelayAttnId_100x = 2,
    } e ;
/* 
Since there are two relays two bypass /10 attenuators, switching
either one of them off results in a 10x attenuation (both: 100x).
*/
}
HantekDlg_RelayAttnId ; 

static unsigned const HantekDlg_RelayAttnChoices = 3 ;

typedef struct
{
    enum
    {
	HantekDlg_MuxAttnId_1x = 0,
	HantekDlg_MuxAttnId_2x = 1,
	HantekDlg_MuxAttnId_5x = 2,
    } e ;
}
HantekDlg_MuxAttnId ; 

static unsigned const HantekDlg_MuxAttnChoices = 3 ;

/* 
The effective voltage ranges are as follows:

    Relay  Mux     Vpp  mV / div
    -----  ---  ------  --------
       1x   1x    0.08        10
       1x   2x    0.16        20
       1x   5x    0.4         50
      10x   1x    0.8        100
      10x   2x    1.6        200
      10x   5x    4          500
     100x   1x    8        1.000
     100x   2x   16        2.000
     100x   5x   40        5.000 
*/

/* --------------------------------------------------------------------
 * Signal Coupling
 * -------------------------------------------------------------------- */

/* 
The input signal may either be coupled in as direct current (DC) or as 
alternating current (AC). There is no ground coupling (as you'll find 
with most oscilloscopes). 
*/

typedef struct
{
    enum
    {
	HantekDlg_CouplingId_Ac  = 0,
	HantekDlg_CouplingId_Dc  = 1,
    } e ;
}
HantekDlg_CouplingId ;

/* --------------------------------------------------------------------
 * Trigger Release
 * -------------------------------------------------------------------- */

/*
The trigger can either be released by the incoming signal. The signal 
source can be one of the two channels (not both).
*/

typedef struct
{
    enum
    {
	HantekDlg_ChannelId_Ch2 = 0, /* Ch2 before Ch1 on purpose */
	HantekDlg_ChannelId_Ch1 = 1,        /* ...see hantekDlg.c */
    } e ;
}
HantekDlg_ChannelId ;

/*
The trigger can be released either by a rising or by a falling edge:
*/

typedef struct
{
    enum
    {
	HantekDlg_SlopeId_Rise = 0,
	HantekDlg_SlopeId_Fall = 1,
    } e ;
}
HantekDlg_SlopeId ;

/*
The trigger is released when the signal raises above or falls below 
a pre-configured ADC level:
*/

typedef struct 
{
    uint16_t i ;
}
HantekDlg_TriggerLevel ;    

/* 
Only 8-bit values seemm to be recognized by the DSO-2090 even though 
the domain is 16-bit.

It appears that the signal has to rise/fall about four (ADC 
quantisation) levels to release the trigger [todo]. It also appears
that the configured trigger-level requires some spacing (to the
min/max levels of the actual signal) to take effect [todo]. Anyways,
if the signal changes only a few levels, the trigger won't be 
released.

For AC mode, a trigger-level of 0x80 should be a safe bet. For DC 
mode, it should be MIN+(MAX-MIN)/2; where MIN and MAX correspond to 
the actual ADC (quantisation) levels. 

The trigger can also be released by the ext-input signal (the device
has an input connector marked as "EXT." besides "Ch1" and "Ch2").
*/

typedef struct
{
    bool enabled ;
}
HantekDlg_ExtInput ;

/*
[caveats]
The trigger is supposed to be either released by a channel signal (see 
HantekDlg_ChannelId) or by the input signal (see HantekDlg_ExtInput). If 
latter is enabled, the channel-signal is (supposed to be) ignored. 

[todo]
Is the ext-input also edge-based? Does the trigger-level apply? How?
*/

/* --------------------------------------------------------------------
 * Input Filter
 * -------------------------------------------------------------------- */

/*
There seems to be some kind of filter for the input signal. Hoever, 
OpenHantek states in its sources that the command to enable/disable the
filter is ignored by the device. It appears to have no effect with the
tested DSO-2090 either. [todo]
*/

typedef struct
{
    bool ch1 ;
    bool ch2 ;
    bool ext ;
}
HantekDlg_Filter ;

/* --------------------------------------------------------------------
 * Device Calibration
 * -------------------------------------------------------------------- */

/* 
An ideal ADC should produce a digital zero-value when the analog input
is zero (i.e. the actual input and the reference voltage are the same 
with respect to ground). Unfortunately, an unwanted offset-voltage may
superimpose the analog input which leads to a non-zero digital output.

The actual value of the unwanted offset-voltage depends on the input 
channel and also on the chosen attenuation. In order to counter the 
interference, the device offers its own DAC-controlled offset-voltage 
(Vos). Vos should be set to a value that reverses the effects of the 
unwanted offset-voltage.
*/

typedef struct 
{
    uint16_t i ;
    /* Only the lower 8-bit value seems to be recognized 
       by the DSO-2090 even though the domain is 16-bit. */
}
HantekDlg_OffsetVoltage ;    

/*
Note that the Vos value does not represent a specific voltage-amount. 
Instead, Vos should be considered as means to adjust the voltage. 

Below you see the results for the calibration of a device.

    ADC:1 (ch1) (frame=large,rate=25/25,coupling=dc)

      Atten.|  Vos |     ADC     |  Vos |     ADC
    --------+------+-------------+------+------------
      1x 1x | 0x1d | 0.013 1.088 | 0x1d | 0.036,1.302
      1x 2x | 0x10 | 0.158 1.734 | 0x10 | 0.076,1.878 
      1x 5x | 0x0b | 0.339 2.263 | 0x0b | 0.149,2.133
     10x 1x | 0x1d | 0.127 2.017 | 0x1d | 0.149,2.142
     10x 2x | 0x0f | 0.000 0.463 | 0x10 | 0.278,2.291 
     10x 5x | 0x0b | 0.492 2.511 | 0x0b | 0.292,2.281
    100x 1x | 0x1d | 0.232 2.338 | 0x1d | 0.335,2.362 
    100x 2x | 0x10 | 0.576 2.595 | 0x10 | 0.352,2.375
    100x 5x | 0x0b | 0.560 2.592 | 0x0b | 0.299,2.279 
    --------+------+-------------+------+------------
            |    1st run         |    2nd run        

    ADC:0 (ch2) (frame=large,rate=25/25,coupling=dc)

      Atten.|  Vos |     ADC     |  Vos |     ADC
    --------+------+-------------+------+------------
      1x 1x | 0x14 | 0.007,1.082 | 0x14 | 0.007 1.117 
      1x 2x | 0x0a | 0.818,2.958 | 0x0a | 0.431 2.438
      1x 5x | 0x06 | 0.000,1.175 | 0x07 | 0.912 3.076
     10x 1x | 0x14 | 0.048,1.960 | 0x14 | 0.076 2.004 
     10x 2x | 0x0a | 1.008,3.220 | 0x0a | 0.818 2.943
     10x 5x | 0x06 | 0.000,1.248 | 0x07 | 0.962 3.145
    100x 1x | 0x14 | 0.091,2.022 | 0x14 | 0.123 2.043
    100x 2x | 0x0a | 0.986,3.171 | 0x0a | 0.831 2.934  
    100x 5x | 0x06 | 0.000,1.177 | 0x07 | 0.964 3.150
    --------+------+-------------+------+------------
            |    1st run         |    2nd run        

    The calibration was conducted as follows: 

    (1) The initial Vos value is 0x00.
    (2) A frame is captured and the average ADC value is computed.
    (3) If the average is zero, Vos is incremented and step (2) is
        repeated.
    (4) A record is created: Vos, average(Vos), average(Vos+1)

The results show (besides others) that the offsets fluctuate. 
Offsets may be (slightly) different for other sampling rates. 

Note that changing Vos can also be imagined as turning the y-pos switch 
on an oscilloscope. Actually, when measuring AC, Vos needs to be set-up
to make a zero voltage input a digital output of 0x80.

The DSO holds calibration data of its own. For each attenuation, there
are two Vos values: 

    - Min to cover the analog range (0,+Vpp] 
    - Max to cover the analog range [-Vpp,0)

    Below you find a device's read-out (Min,Max):

                     ch1                     ch2          
           ----------------------- -----------------------
      1x | (1d,8f) (10,82) ( b,7c) (16,81) ( a,79) ( 8,77)
     10x | (1e,8f) (11,82) ( b,7c) (13,84) ( a,7b) ( 7,78)
    100x | (1e,8f) (11,82) ( b,7c) (14,85) ( a,7b) ( 7,77)
           ------- ------- ------- ------- ------- -------
              1x      2x      5x       1x     2x      5x  

    For DC coupling you may want set Vos to Min.
    For AC coupling you may want set Vos to Min+(Max-Min)/2.
*/

typedef struct
{
    HantekDlg_OffsetVoltage v[2][3][3][2] ;
    /* index 0: (ch1,ch2)
       index 1: relays attenuation: (1,10,100)
       index 2: multiplexer attenuation: (1,2,5)
       index 3: range: (top,bottom) */
}
HantekDlg_Calibration ;

HantekDlg_Calibration hantekDlg_getCalibration(libusb_device_handle*) ;

/*
[caveats]
The device's internal calibration data differ from the values produced 
by the manual calibration run.

[todo]
Measure offset fluctuation and offset dependencies on sample-rate.

[todo]
What does actually happen when Vos rises above 0xff?
*/

/* --------------------------------------------------------------------
 * Configuration Commands
 * -------------------------------------------------------------------- */

/*
The definition of the five configuration commands and their arguments 
appears to be 'random' (in a way). It is difficult to group them in a
'logical' way. So they are gathered below:
*/

void hantekDlg_setFilter(
    libusb_device_handle*,
    HantekDlg_Filter filter) ;

void hantekDlg_setInput(
    libusb_device_handle*,
    HantekDlg_InputId       inputId,
    HantekDlg_FrameId       frameId,
    HantekDlg_Prescaler   prescaler, 
    HantekDlg_TriggerCount    count,
    HantekDlg_ChannelId   channelId,
    HantekDlg_SlopeId       slopeId) ;

void hantekDlg_setMux(
    libusb_device_handle*,
    HantekDlg_MuxAttnId ch1,
    HantekDlg_MuxAttnId ch2) ;

void hantekDlg_setOffset(
    libusb_device_handle*,
    HantekDlg_OffsetVoltage ch1,
    HantekDlg_OffsetVoltage ch2,
    HantekDlg_TriggerLevel level) ;

void hantekDlg_setRelay(
    libusb_device_handle*,
    HantekDlg_RelayAttnId attn_ch1,HantekDlg_CouplingId coupling_ch1,
    HantekDlg_RelayAttnId attn_ch2,HantekDlg_CouplingId coupling_ch2,
    HantekDlg_ExtInput ext) ;

/* 
References:

OpenHantek by OliverHaag:
github.com/OpenHantek/openhantek

"Inside a Hantek DSO-2090" by Fabio Baltieri:
fabiobaltieri.com/2013/07/10/inside-a-hantek-dso-2090-usb-oscilloscope
[sigrok]

Sigrok:
sigrok.org/wiki/Hantek_DSO-2090
*/

#endif /* INCLUDE_hantekDlg_h */
