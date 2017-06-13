/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

/* user-interface: try to estimate the offset-voltage 
   (instead of using the pre-defined Hantek values */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dod.h"
#include "hantekExt.h"
#include "optDevice.h"
#include "optHantek.h"
#include "textHantek.h"
#include "ui.h"
#include "usbExt.h"
#include "systime.h"

/* transfer sample from device to host */
static uint8_t* fetch(libusb_device_handle *handle,size_t nbytes)
{
    uint8_t *buffer = dod_malloc(nbytes) ;
    hantekDlg_fetch(handle,buffer,nbytes) ;
    return buffer ;
}

/* compute the average sample level (for both adc) */
static void mean(uint16_t      pointer, 
		 uint16_t     nrecords, /* range: pointer - nrecords */
		 uint16_t    frameSize, /* need to detect wrap-around */
		 uint8_t const *buffer,
		 double         *mean0, /* result adc-0 */
		 double         *mean1) /* result adc-1 */
{
    /* move pointer from the (current) end-record 
       back to the start-record of the sample range */
    assert(nrecords <= frameSize) ;
    uint32_t offset = (pointer >= nrecords)
	? 2u * (uint16_t)(          - nrecords + pointer)
	: 2u * (uint16_t)(frameSize - nrecords + pointer) ;

    uint32_t sum0 = 0, sum1 = 0 ;
    for (uint32_t i=0 ; i<nrecords ; ++i)
    {
	sum0 += buffer[offset] ; ++offset ;
	sum1 += buffer[offset] ; ++offset ;
	if (offset == 2u * frameSize)
	    offset = 0 ;
    }
    (*mean0) = 1.0 * sum0 / nrecords ;
    (*mean1) = 1.0 * sum1 / nrecords ;
    /* [future] min and max values might also be of interest */
}    

/* sample data and return the average sample level for the given adc */
static double getMean(libusb_device_handle *handle,
		      HantekDlg_OffsetVoltage  vos, 
		      uint16_t            nrecords,
		      HantekDlg_AdcId          adc,
		      uint16_t           frameSize) /* to detect wrap-around */
{
    hantekDlg_setOffset(handle,vos,vos,(HantekDlg_TriggerLevel){0}) ;
    /* ...overwrites also the trigger-level; 
       doesn't matter though since we trigger anyway ourselves */

    uint16_t pointer = hantekExt_capture(handle,true).pointer[0] ;
    while (pointer >= frameSize)
	/* see hantekExt_capture() for details */
	pointer = hantekExt_capture(handle,true).pointer[0] ;

    uint8_t *buffer = fetch(handle,2u*frameSize) ;
    double mean0,mean1 ;
    mean(++pointer,nrecords,frameSize,buffer,&mean0,&mean1) ;
    free(buffer) ;
    switch (adc.e)
    {
    case HantekDlg_AdcId_0: return mean0 ;
    case HantekDlg_AdcId_1: return mean1 ;
    }
    assert(false) ; abort() ;
}

/* helper: a simple progress indicator (rotating bar-character) */
static void showProgress(unsigned i)
{
    switch (i & 3)
    {
    case 0: printf("|") ; break ;
    case 1: printf("/") ; break ;
    case 2: printf("-") ; break ;
    case 3: printf("\\") ; break ;
    }
    fflush(stdout) ;
}

/* determine the offset-voltage that makes the ADC-level raise from 
     0x00 to 0x01; we start with vos=0xff and increment it after each 
     loop (so approaching the 0->1 threshold from left; 
   return the lowest offset-voltage which produces an ADC-(average)-
     level above 0x00; return the produced ADC-level in _a_ and also 
     the ADC-level for (vos+1) in _b_ */
static HantekDlg_OffsetVoltage approachLeft(
    libusb_device_handle *handle,
    uint16_t            nrecords,
    HantekDlg_AdcId             adc,
    uint16_t           frameSize,
    double                    *a,
    double                    *b)
{
    HantekDlg_OffsetVoltage vos = { 0 } ;
    (*b) = getMean(handle,vos,nrecords,adc,frameSize) ;
    do {
	++vos.i ;
	(*a) = (*b) ;
	showProgress(vos.i) ;
	(*b) = getMean(handle,vos,nrecords,adc,frameSize) ;
	printf("\b") ;
    }
    while (((*a) == 0.0) && (vos.i != 0xff)) ;
    --vos.i ;
    return vos ;
}

/* determine the offset-voltage that makes the ADC-level raise from 
     0x7f to 0x80; the given vos is increment it after each loop (so 
     approaching the 7F->80 threshold from left; 
   return the highest offset-voltage which produces an ADC-(average)-
     level below 0x80; return the produced ADC-level in _a_ and also 
     the ADC-level for (vos+1) in _b_ */
static HantekDlg_OffsetVoltage approachMiddle(
    HantekDlg_OffsetVoltage     vos,
    libusb_device_handle *handle,
    uint16_t            nrecords,
    HantekDlg_AdcId             adc,
    uint16_t           frameSize,
    double                    *a,
    double                    *b)
{
    (*b) = getMean(handle,vos,nrecords,adc,frameSize) ;
    do {
	++vos.i ;
	(*a) = (*b) ;
	showProgress(vos.i) ;
	(*b) = getMean(handle,vos,nrecords,adc,frameSize) ;
	printf("\b") ;
    }
    while (((*b) < 128.0) && (vos.i != 0xff)) ;
    --vos.i ;
    return vos ;
}

/* determine the offset-voltage that makes the ADC-level drop from 
     0xff to 0xfe; we start with vos=0xff and increment it after each 
     loop (so approaching the FF->FE threshold from right; 
   return the highets offset-voltage which produces an ADC-(average)-
     level below 0xff; return the produced ADC-level in _a_ and also 
     the ADC-level for (vos-1) in _b_ */
static HantekDlg_OffsetVoltage approachRight(
    libusb_device_handle *handle,
    uint16_t            nrecords,
    HantekDlg_AdcId             adc,
    uint16_t           frameSize,
    double                    *a,
    double                    *b)
{
    HantekDlg_OffsetVoltage vos = { 0xff } ;
    (*b) = getMean(handle,vos,nrecords,adc,frameSize) ;
    do {
	--vos.i ;
	(*a) = (*b) ;
	showProgress(vos.i) ;
	(*b) = getMean(handle,vos,nrecords,adc,frameSize) ;
	printf("\b") ;
    }
    while (((*a) == 255.0) && (vos.i != 0x00)) ;
    ++vos.i ;
    return vos ;
}

/* Print the vos-values and average-levels for following thresholds:
   0x00->0x01, 0x7f->0x80 and 0xff->0xfe for the given ADC. 
   This function executes a series of data-acquisitions in order to 
   sample (measure) and compute these thresholds. All device parameters
   besides offset-voltage have to be set-up by the caller beforehand. */
static void printRange(
    libusb_device_handle *handle,
    uint16_t            nrecords,
    HantekDlg_AdcId             adc,
    uint16_t           frameSize)
{
    double a,b ; HantekDlg_OffsetVoltage vos ;
    
    vos = approachLeft(handle,nrecords,adc,frameSize,&a,&b) ;
    printf("[0x%02x] %.3f,%.3f ",vos.i,a,b) ;

    vos = approachMiddle(vos,handle,nrecords,adc,frameSize,&a,&b) ;
    printf("... [0x%02x] %.3f,%.3f [0x%02x] ",vos.i,a,b,vos.i+1) ;

    vos = approachRight(handle,nrecords,adc,frameSize,&a,&b) ;
    printf("... %.3f,%.3f [0x%02x]\n",b,a,vos.i) ;
}

typedef struct
{
    uint16_t          nrecords ;
    HantekDlg_AdcId           adc ;
    HantekDlg_CouplingId coupling ;    
    HantekDlg_FrameId       frame ;
    HantekDlg_InputId       input ;
    HantekDlg_Prescaler prescaler ;
    HantekDlg_TriggerCount  count ;
}
Config ;

static Config scan(Opt *opt)
{
    Config c ;
    c.nrecords  = opt_uint16             (opt) ;
    
    c.adc       = optHantek_AdcId        (opt) ;
    c.input     = optHantek_InputId      (opt) ;
    c.frame     = optHantek_FrameId      (opt) ;
    c.prescaler = optHantek_Prescaler    (opt) ;
    c.coupling  = optHantek_CouplingId   (opt) ;

    if (c.nrecords > hantekDlg_frameSize(c.frame))
	dod_exit("number of records (%u) exceed frame-size (%u)",
		 c.nrecords,hantekDlg_frameSize(c.frame)) ;
    c.count = hantekDlg_triggerCount(c.nrecords) ;
    
    return c ;
}

static void calibrate(libusb_device_handle *handle,Config const *config)
{
    for (HantekDlg_RelayAttnId rix={0} ; rix.e<HantekDlg_RelayAttnChoices ; ++rix.e)
    {
	hantekDlg_setRelay(
	    handle,
	    rix,config->coupling,
	    rix,config->coupling,
	    (HantekDlg_ExtInput){false}) ; /* ignore ext_trigger */

	for (HantekDlg_MuxAttnId mix={0} ; mix.e<HantekDlg_MuxAttnChoices ; ++mix.e)
	{
	    hantekDlg_setMux(handle,mix,mix) ;
		
	    static char const * relay[] = { "  1x"," 10x","100x" } ;
	    static char const *   mux[] = {   "1x",  "2x",  "5x" } ;
		
	    printf("%s %s | ",relay[rix.e],mux[mix.e]) ; fflush(stdout) ;
	    printRange(handle,config->nrecords,config->adc,
		       hantekDlg_frameSize(config->frame)) ;
	}
    }
}

void ui_calibrate(libusb_context *libusb,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: "
	    "DEVICE NRECORDS ADC INPUT FRAME PRESCALER COUPLING\n"
	    "\n"
	    "NRECORDS: the number of records to sample per offset-voltage\n"
	    ) ;
	return ;
    }
    
    libusb_device_handle *handle = optDevice_dso2090(opt,libusb) ;
    Config c = scan(opt) ;
    opt_finish(opt) ;

    /* wait till trigger counted down [todo] reset/timeout */
    HantekDlg_Status status = hantekDlg_getStatus(handle) ;
    while (status.state.e == HantekDlg_State_PostTrigger)
	status = hantekDlg_getStatus(handle) ;

    hantekDlg_setInput(
	handle,
	c.input,
	c.frame,
	c.prescaler,
	c.count,
	(HantekDlg_ChannelId){0},
	(HantekDlg_SlopeId)  {0}) ;

    calibrate(handle,&c) ;

    libusb_close(handle) ;
}
