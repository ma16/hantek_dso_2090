/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_cypress_h
#define INCLUDE_cypress_h

/* upload firmware to cypress FX2 series chips */

#include <libusb.h>

void cypress_upload(libusb_device_handle *h,
		    int       configuration,
		    char const      *buffer,
		    uint16_t         length) ;
/* limits the length to uint16_t since the value field of the setup 
   packet, which holds the firmware offset, is only 16 bit wide. */

#endif /* INCLUDE_cypress_h */
