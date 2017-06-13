/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "hantekDlg.h"
#include <assert.h>
#include <string.h> /* memcpy */
#include "dod.h"

/* usb control transfer (do-or-die) */
static void controlXfer(
    libusb_device_handle *handle,
    uint8_t          requestType,
    uint8_t              request,
    uint16_t               value,
    uint8_t                *data,
    uint16_t              nbytes)
{
    int result = libusb_control_transfer(
	handle,requestType,request,value,0,data,nbytes,100) ;
    if (result < 0)
      	dod_exit("hantekDlg:request=%d:%s",
		 request,libusb_error_name(result)) ;
    if (result != nbytes)
	dod_exit("hantekDlg:request=%d:"
		 "getCalibration:size (%d) mismatch",
		 request,result) ;
}

/* ----[ control 0xa2 ]------------------------------------------------ */

static HantekDlg_Calibration decode(uint8_t const *src,size_t nwords)
{
    HantekDlg_Calibration calibration ;
    /* from big-endian (most significant byte first) to native */
    HantekDlg_OffsetVoltage *dst = calibration.v[0][0][0] ;
    for (size_t i=0 ; i<nwords ; ++i)
    {
	dst[i].i = (uint16_t)((src[0] << 8) | src[1]) ;
	src += 2 ;
    }
    return calibration ;
}

HantekDlg_Calibration hantekDlg_getCalibration(libusb_device_handle *handle)
{
    static uint16_t const nwords = /* i.e. 36 */
	sizeof(((HantekDlg_Calibration*)0)->v) /
	sizeof(((HantekDlg_Calibration*)0)->v[0][0][0][0]) ;
    static uint16_t const nbytes = 2u * nwords ;
    uint8_t data[nbytes] ;
    controlXfer(
	handle,LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR,
	0xa2, /* read eeprom */	0x08, /* eeprom offset */ data,nbytes) ;
    return decode(data,nwords) ;
}

/* ----[ control 0xb3 ] ----------------------------------------------- */

static void hantekDlg_setup(libusb_device_handle *handle)
{
    uint8_t data[2] = {0x0f,0x03} ;
    /* [peculiar] OpenHantek uses instead
          [10] = { 0x0f, bulk-index, bulk-index, bulk-index}
       where the fields 1..3 take a "bulk-index" 
       OpenHantek uses '3' but says other values are possible too 
       [todo] this needs further investigation */

    controlXfer(
	handle,LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR,
	0xb3, /* hantek */ 0, data,sizeof(data)) ;
}
/* ----[ control 0xb4 ] ------------------------ */

static void makeData_setOffset(
    uint8_t           (*data)[6],    
    HantekDlg_OffsetVoltage  ch1,
    HantekDlg_OffsetVoltage  ch2,
    HantekDlg_TriggerLevel level)
{
    (*data)[0] = (uint8_t)  (ch1.i >> 8) ; /* [peculiar] sigrok does "|0x20" */
    (*data)[1] = (uint8_t)   ch1.i       ;
    (*data)[2] = (uint8_t)  (ch2.i >> 8) ; /* [peculiar] sigrok does "|0x20" */
    (*data)[3] = (uint8_t)   ch2.i       ;
    (*data)[4] = (uint8_t)(level.i >> 8) ; /* [peculiar] sigrok does "|0x20" */
    (*data)[5] = (uint8_t) level.i       ;
}

void hantekDlg_setOffset(
    libusb_device_handle *handle,
    HantekDlg_OffsetVoltage     ch1,
    HantekDlg_OffsetVoltage     ch2,
    HantekDlg_TriggerLevel    level) 
{
    uint8_t data[6] ; /* 17 bytes in OpenHantek */
    makeData_setOffset(&data,ch1,ch2,level) ;
    controlXfer(
	handle,LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR,
	0xb4, /* hantek */ 0, data,sizeof(data)) ;
}

/* ----[ control 0xb5 ]------------------------- */

static void makeData_setRelay(
    uint8_t             (*data)[8],    
    HantekDlg_RelayAttnId    attn_ch1,
    HantekDlg_CouplingId coupling_ch1,
    HantekDlg_RelayAttnId    attn_ch2,
    HantekDlg_CouplingId coupling_ch2,
    HantekDlg_ExtInput            ext)
{
    (*data)[0] = 0 ; /* [peculiar] what does this control? */
    
    switch (attn_ch1.e)
    {
    case HantekDlg_RelayAttnId_1x   : (*data)[1] = (uint8_t)~0x04 ;
	                              (*data)[2] = (uint8_t)~0x08 ; break ;
    case HantekDlg_RelayAttnId_10x  : (*data)[1] =           0x04 ;
	                              (*data)[2] = (uint8_t)~0x08 ; break ;
    case HantekDlg_RelayAttnId_100x : (*data)[1] =           0x04 ;
	                              (*data)[2] =           0x08 ; break ;
    default: assert(false) ;
    }

    switch (coupling_ch1.e)
    {
    case HantekDlg_CouplingId_Ac    : (*data)[3] =           0x02 ; break ;
    case HantekDlg_CouplingId_Dc    : (*data)[3] = (uint8_t)~0x02 ; break ;
    default: assert(false) ;
    }
	
    switch (attn_ch2.e)
    {
    case HantekDlg_RelayAttnId_1x   : (*data)[4] = (uint8_t)~0x20 ;
	                              (*data)[5] = (uint8_t)~0x40 ; break ;
    case HantekDlg_RelayAttnId_10x  : (*data)[4] =           0x20 ;
	                              (*data)[5] = (uint8_t)~0x40 ; break ;
    case HantekDlg_RelayAttnId_100x : (*data)[4] =           0x20 ;
	                              (*data)[5] =           0x40 ; break ;
    default: assert(false) ;
    }

    switch (coupling_ch2.e)
    {
    case HantekDlg_CouplingId_Ac    : (*data)[6] =           0x10 ; break ;
    case HantekDlg_CouplingId_Dc    : (*data)[6] = (uint8_t)~0x10 ; break ;
    default: assert(false) ;
    }

    /* index 1..6 control relays, index 7 does not */
    
    if (ext.enabled) (*data)[7] = (uint8_t)~0x01 ; 
    else             (*data)[7] =           0x01 ; 

    /* [peculiar] Values at index 1..7 have a binary effect (either on
       or off). However, the actual numbers are 0x1,0x2,0x4,0x8,0x10,
       0x20,0x40 (plus inversed values).*/
}

void hantekDlg_setRelay(
    libusb_device_handle   *handle,
    HantekDlg_RelayAttnId    attn_ch1,
    HantekDlg_CouplingId coupling_ch1,
    HantekDlg_RelayAttnId    attn_ch2,
    HantekDlg_CouplingId coupling_ch2,
    HantekDlg_ExtInput            ext)
{
    uint8_t data[8] ; /* 17 bytes in OpenHantek */
    makeData_setRelay(
	&data,
	attn_ch1,coupling_ch1,
	attn_ch2,coupling_ch2,
	ext) ;
    controlXfer(
	handle,LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR,
	0xb5, /* hantek */ 0, data,sizeof(data)) ;
}

/* -------------------------------------------------------------------- */

/* usb bulk transfer to send data (do-or-die) */
static void bulkSend(libusb_device_handle *handle,uint8_t *data,size_t nbytes_)
{
    /* libusb uses an int as size-type */
    int nbytes = (int)nbytes_ ;
    if ((nbytes_ != (size_t)nbytes) || (nbytes < 0))
	dod_exit("hantek:bulkSend:buffer too big:%zu",nbytes_) ;
    
    int transferred ;
    int result = libusb_bulk_transfer(
	handle,LIBUSB_ENDPOINT_OUT | 0x02,
	data,nbytes,&transferred,100) ;
    
    if (result < 0)
	/* partial data may have been transferred on timeout */
	dod_exit("hantekDlg:bulkSend:%s (transferred %d bytes)",
		 libusb_error_name(result),transferred) ;
    assert(result == 0 && transferred == nbytes) ;
}

/* usb bulk transfer to receive data (do-or-die) */
static void bulkRecv(libusb_device_handle *handle,uint8_t *data,size_t nbytes_)
{
    /* libusb uses an int as size-type */
    int nbytes = (int)nbytes_ ;
    if ((nbytes_ != (size_t)nbytes) || (nbytes < 0))
	dod_exit("hantek:bulkRecv:buffer too big:%zu",nbytes_) ;
    
    memset(data,0,nbytes_) ;

   /* data will automatically be split by libusb if they're larger 
      than the endpoint's maximum-packet-size (often 512 bytes). */
    
    int transferred ; 
    int result = libusb_bulk_transfer(
	handle,LIBUSB_ENDPOINT_IN | 0x06,
	data,nbytes,&transferred,100) ;
    
    /* Note: An "error" occurs if the requested size doesn't match the 
       received size:
       --LIBUSB_ERROR_TIMEOUT if the device sends less than the 
         requested number of bytes. This "error" is already covered. 
	 The function above simply returns what is available on a
	 timeout.
       --LIBUSB_TRANSFER_OVERFLOW if the buffer is not a multiple 
         of the endpoint's maximum packet size _and_ if more data
	 is received than would fit into the buffer. The client has to 
	 deal with this kind of problem, e.g. by allocating a multiple
	 of the maximum packet size. 
       see: http://libusb.sourceforge.net/api-1.0/packetoverflow.html 
       --LIBUSB_ERROR_IO on very large buffers */
    
    if (result < 0)
	/* partial data may have been transferred on timeout */
	dod_exit("hantekDlg:bulkRecv:%s (transferred %d bytes)",
		 libusb_error_name(result),transferred) ;
    assert(result == 0 && transferred == nbytes) ;
}

/* ----[ bulk 0 ]------------------------------------------------------ */

static void makeData_setFilter(
    uint8_t (*data)[8],
    bool    filter_ch1,
    bool    filter_ch2,
    bool    filter_ext)
{
    (*data)[0] =    0 ; /* hantek command code */
    (*data)[1] = 0x0f ; /* [peculiar] another magic number? */

    (*data)[2] = 0 ;
    if (!filter_ch1) (*data)[2] |= 0x80 ; /* bit=1 to disable */
    if (!filter_ch2) (*data)[2] |= 0x40 ; /* bit=1 to disable */
    if (!filter_ext) (*data)[2] |= 0x20 ; /* bit=1 to disable */

    (*data)[3] = 0 ; 
    (*data)[4] = 0 ;
    (*data)[5] = 0 ;
    (*data)[6] = 0 ;
    (*data)[7] = 0 ;
}

void hantekDlg_setFilter(libusb_device_handle *handle,HantekDlg_Filter filter)
{
    hantekDlg_setup(handle) ;
    uint8_t data[8] ; 
    makeData_setFilter(&data,filter.ch1,filter.ch2,filter.ext) ;
    bulkSend(handle,data,sizeof(data)) ;
}

/* ----[ bulk 1 ]------------------------------- */

static void makeData_setInput(
    uint8_t           (*data)[12],
    HantekDlg_InputId     inputId,
    HantekDlg_FrameId     frameId,
    HantekDlg_Prescaler prescaler, 
    HantekDlg_TriggerCount  count,
    HantekDlg_ChannelId channelId,
    HantekDlg_SlopeId     slopeId)
{
    (*data)[0]  = 1 ; /* hantek command code */
    (*data)[1]  = 0 ;
    
    (*data)[2]  = channelId.e ; 
    (*data)[2] |= (uint8_t)(frameId.e << 2) ;

    if (prescaler.e == HantekDlg_Prescaler_ById)
    {
	(*data)[2] |= (uint8_t)(prescaler.u.id.e << 5) ;
    }
    else
    {
	assert(prescaler.e == HantekDlg_Prescaler_ByDivider) ;
	(*data)[2] |= 0x4 << 5 ;
	(*data)[4] = (uint8_t) prescaler.u.divider       ;
	(*data)[5] = (uint8_t)(prescaler.u.divider >> 8) ;
    }

    (*data)[3]  = (uint8_t)((slopeId.e << 3) /*| 0x04*/ | inputId.e) ;
    /* [peculiar] OpenHantek uses bit:2 for 
       "fast-rate" which seems to have no effect */

    (*data)[ 6] = (uint8_t) count.i        ;
    (*data)[ 7] = (uint8_t)(count.i >>  8) ;
    (*data)[ 8]  = 0 ;
    (*data)[ 9]  = 0 ;
    (*data)[10] = (uint8_t)(count.i >> 16) ;
    (*data)[11]  = 0 ;
}

void hantekDlg_setInput(
    libusb_device_handle *handle,
    HantekDlg_InputId       inputId,
    HantekDlg_FrameId       frameId,
    HantekDlg_Prescaler   prescaler, 
    HantekDlg_TriggerCount    count,
    HantekDlg_ChannelId   channelId,
    HantekDlg_SlopeId       slopeId)
{
    hantekDlg_setup(handle) ;
    uint8_t data[12] ; 
    makeData_setInput(
	&data,
	inputId,
	frameId,
	prescaler,
	count,
	channelId,
	slopeId) ;
    bulkSend(handle,data,sizeof(data)) ;
}

/* ----[ bulk 2 ]------------------------------- */

void hantekDlg_releaseTrigger(libusb_device_handle *handle)
{
    hantekDlg_setup(handle) ;
    uint8_t data[] = {2,0} ; /* hantek command code */
    bulkSend(handle,data,sizeof(data)) ;
}

/* ----[ bulk 3 ]------------------------------- */

void hantekDlg_capture(libusb_device_handle *handle)
{
    hantekDlg_setup(handle) ;
    uint8_t data[] = {3,0} ; /* hantek command code */
    bulkSend(handle,data,sizeof(data)) ;
}

/* ----[ bulk 4 ]------------------------------- */

void hantekDlg_enableTrigger(libusb_device_handle *handle)
{
    hantekDlg_setup(handle) ;
    uint8_t data[] = {4,0} ; /* hantek command code */
    bulkSend(handle,data,sizeof(data)) ;
}

/* ----[ bulk 5 ]------------------------------- */

void hantekDlg_fetch(libusb_device_handle *handle,uint8_t *buffer,size_t nbytes)
{
    hantekDlg_setup(handle) ;
    uint8_t data[] = {5,0} ; /* hantek command code */
    bulkSend(handle,data,sizeof(data)) ;
    bulkRecv(handle,buffer,nbytes) ;
}

/* ----[ bulk 6 ]------------------------------- */

static HantekDlg_Status getStatus(uint8_t (*buffer)[0x200])
{
    HantekDlg_Status status ;
    status.state.e = (*buffer)[0] ;
    
    /* The pointers are 16-bit values where...
       ...each set-bit inverts all right-hand bits. [peculiar] */

    uint8_t const *p = &(*buffer)[2] ;
    for (size_t i=0 ; i<0xff ; ++i)
    {
	uint16_t pointer = *p++ ;
	pointer |= (uint16_t)((*p++) << 8) ;
	uint16_t mask = 1 ;
	while (mask != 0)
	{
	    if (pointer & mask)
		pointer ^= (uint16_t)(mask - 1) ;
	    mask = (uint16_t)(mask << 1) ;
	}
	status.pointer[i] = pointer ;
    }
    
    return status ;
}

HantekDlg_Status hantekDlg_getStatus(libusb_device_handle *handle)
{
    hantekDlg_setup(handle) ;
    uint8_t data[] = {6,0} ; /* hantek command code */
    bulkSend(handle,data,sizeof(data)) ;
    uint8_t buffer[0x200] ;
    bulkRecv(handle,buffer,sizeof(buffer)) ;
    return getStatus(&buffer) ;
}
 
/* ----[ bulk 7 ]------------------------------- */

static void makeData_setMux(
    uint8_t        (*data)[8],
    HantekDlg_MuxAttnId attn_ch1,
    HantekDlg_MuxAttnId attn_ch2)
{
    (*data)[0] =    7 ; /* hantek command code */
    (*data)[1] = 0x0f ; /* [peculiar] another magic number? */
    
    (*data)[2] = (uint8_t) (0x30 | (attn_ch2.e << 2) | attn_ch1.e) ;
    /* [peculiar] what stands 0x30 for? */
    
    (*data)[3] = 0 ; 
    (*data)[4] = 0 ;
    (*data)[5] = 0 ;
    (*data)[6] = 0 ;
    (*data)[7] = 0 ;
}

void hantekDlg_setMux(
    libusb_device_handle *handle,
    HantekDlg_MuxAttnId ch1,
    HantekDlg_MuxAttnId ch2)
{
    hantekDlg_setup(handle) ;
    uint8_t data[8] ; 
    makeData_setMux(&data,ch1,ch2) ;
    bulkSend(handle,data,sizeof(data)) ;
}
