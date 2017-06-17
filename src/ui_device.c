/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

/* user-interface: scan for Hantek devices and upload firmware */

#include "ui.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cypress.h"
#include "device.h"
#include "dod.h"
#include "opt.h"
#include "optDevice.h"
#include "ui.h"
#include "usbExt.h"
#include "systime.h"

/* print device address if it is Hantek DSO-2090 */
static bool scanDevice(void *self,libusb_device *device)
{
    /* this callback is invoked by usbExt_browse() for each USB device */
    (void)self ;
    
    char const *s ;
    if      (device_isCypress(device)) s = "(w/o firmware)" ;
    else if (device_isDso2090(device)) s = "(with firmware)" ;
    else return false ;
    
    UsbExt_Addr addr = usbExt_addr(device) ;
    UsbExt_Path path = usbExt_path(device) ;
    printf("Hantek DSO-2090 %s on address <%s> at path <%s>\n",s,addr.s,path.s) ;
    
    return false ;
}

/* scan for devices (with or without Hantek firmware) */
static void scan(libusb_context *libusb,Opt *opt)
{
    opt_finish(opt) ;
    usbExt_browse(libusb,scanDevice,NULL) ;
    /* The "lsusb" command on Linux can be used too, e.g.:

       Bus 001 Device 044: ID 04b4:2090 Cypress Semiconductor Corp.
       Bus 001 Device 053: ID 04b5:2090 ROHM LSI Systems USA, LLC 

       The vendor-id 0x4b5 is used by Hantek firmware. 
       The product-id is 0x2090 is any case (for Cypress and Hantek)
    */
}

/* check whether device popped up after firmware uploads */
static bool findPath(libusb_context *libusb,UsbExt_Path const *path)
{
    libusb_device_handle *handle = usbExt_openPath(libusb,path) ;
    if (handle == NULL)
	return false ;
    libusb_device *device = libusb_get_device(handle) ;
    bool success = device_isDso2090(device) ;
    libusb_close(handle) ;
    return success ;
}

/* upload Hantek firmware to Cypress FX2 chip */
static void upload(libusb_context *libusb,Opt *opt)
{
    libusb_device_handle *handle = optDevice_cypress(opt,libusb) ; 
    /* we need also the port (path) since the device-
       address changes when uploading the firmware */
    UsbExt_Path path = usbExt_path(libusb_get_device(handle)) ;

    /* firmware's image-size (currently ~5k) must not exceed uint16_t */
    char image[0xffff] ;
    uint16_t size = (uint16_t)dod_loadFile(
	opt_pop(opt),image,sizeof(image)) ;

    cypress_upload(handle,/*configuration-index*/1,image,size)  ;
    libusb_close(handle) ;
	
    printf("wait for device to pop (this may take 2 seconds)... ") ;
    fflush(stdout) ;
    double t0 = systime_get() ;
    /* the device needs a moment to disappear; if we try to find the 
       new device immediately, we may run into a LIBUSB_ERROR_IO when
       opening */
    systime_sleep(0.5) ;
    bool found = findPath(libusb,&path) ;
    while (!found && (systime_get()-t0) < 5.0)
    {
	/* try for no more than 5 seconds in 0.1 second intervals */
	systime_sleep(0.1) ;
	found = findPath(libusb,&path) ;
    }
    printf(found ? "success\n" : "timed-out\n") ;
    /* the DSO is active and samples data at 
       50 MHz in tiny frames after firmware upload  */
}

void ui_device(libusb_context *libusb,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: MODE\n"
	    "MODE : scan                # list all Hantek DSO-2090\n"
	    "     | upload DEVICE FILE  # upload FILE to DEVICE\n"
	    ) ;
	return ;
    }

    char const *arg = opt_pop(opt) ;
    if (false) ;

    else if (0 == strcmp(arg,    "scan"))    scan(libusb,opt) ;
    else if (0 == strcmp(arg,  "upload"))  upload(libusb,opt) ;
    
    else dod_exit("option not recognized:<%s>",arg) ;
}
