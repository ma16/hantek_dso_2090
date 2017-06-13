/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "cypress.h"
#include "dod.h"
#include <stdbool.h>

static void configure(struct libusb_device_handle *h,int configuration)
{
    int result = libusb_kernel_driver_active(h,0) ;
    /* libusb: "This functionality is not available on Windows." */
    if (result < 0)
	dod_exit("cypress:libusb_kernel_driver_active():%s",
		 libusb_error_name(result)) ;
    if (1 == result)
    {
	result = libusb_detach_kernel_driver(h,0) ;
	if (result < 0)
	    /* there might be problems on APPLE */
	    dod_exit("cypress:libusb_detach_kernel_driver:%s",
		     libusb_error_name(result)) ;
    }
    result = libusb_set_configuration(h,configuration) ;
    if (result < 0)
	dod_exit("cypress:libusb_set_configuration:%s",
		 libusb_error_name(result)) ;
}
    
static void reset(struct libusb_device_handle *h,bool on)
{
    uint8_t mode = on ? 1 : 0 ;
    int result = libusb_control_transfer(
	h,LIBUSB_REQUEST_TYPE_VENDOR,
	0xa0,0xe600,0x0000,&mode,1,100) ;
    if (result < 0)
	dod_exit("cypress:libusb_control_transfer:%s",
		 libusb_error_name(result)) ;
}

static void install(libusb_device_handle *h,char const *buffer,uint16_t length)
{
    uint16_t offset = 0 ;
    while (offset < length)
    {
	/* firmware is uploaded in chunks of 0x1000 bytes */
	uint16_t csize = 0x1000 ; 
	if (csize > length - offset)
	    csize = (uint16_t)(length - offset) ;

	int result = libusb_control_transfer(
	    h,LIBUSB_REQUEST_TYPE_VENDOR,
	    0xa0,offset,0x0000,(uint8_t*)buffer+offset,csize,100) ;
	if (result < 0)
	    dod_exit("cypress:libusb_control_transfer:%s",
		     libusb_error_name(result)) ;
	
	offset = (uint16_t)(offset + csize) ;
    }
}

void cypress_upload(libusb_device_handle *h,
		    int       configuration,
		    char const      *buffer,
		    uint16_t         length)
{
    configure(h,configuration) ;
    reset    (h,         true) ;
    install  (h,buffer,length) ;
    reset    (h,        false) ;
}
