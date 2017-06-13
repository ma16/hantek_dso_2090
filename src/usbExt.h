/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_usbExt_h
#define INCLUDE_usbExt_h

/* some useful extensions of libusb for this project (e.g. do-or-die) */

#include <libusb.h>
#include <stdbool.h>

/* do-or-die */
libusb_context* usbExt_init(void) ;

/* browse thru all usb devices and invoke the given callback on each;
       the callback should return true to stop further calls;
   returns true if browsing was prematurely stopped by callback */
bool usbExt_browse(libusb_context*,
		   bool(*callback)(void*,libusb_device*),
		   void *cb_data) ;

/* do-or-die */
libusb_device_handle* usbExt_open(libusb_device *device) ;

uint16_t usbExt_vendor (libusb_device *device) ;
uint16_t usbExt_product(libusb_device *device) ;

typedef struct 
{
    char s[32] ; /* 3 + 7*4 +1 */
    /* a string of the form: /BUS ('.' PORT)* /
       e.g. "1" or "2.1" or "1.2.3.4.1.2.3.4" */
  
}
UsbExt_Path ;

UsbExt_Path usbExt_path(libusb_device *device) ;

typedef struct 
{
    char s[8] ; /* 3+1+3 +1 */
    /* a string of the form: /BUS ':' DEV_ADDR/ e.g. "2:42" */
}
UsbExt_Addr ;

UsbExt_Addr usbExt_addr(libusb_device *device) ;

libusb_device* usbExt_findAddr(libusb_context*,UsbExt_Addr const*) ;
libusb_device* usbExt_findPath(libusb_context*,UsbExt_Path const*) ;

libusb_device_handle* usbExt_openAddr(libusb_context*,UsbExt_Addr const*) ;
libusb_device_handle* usbExt_openPath(libusb_context*,UsbExt_Path const*) ;

#endif /* INCLUDE_usbExt_h */
