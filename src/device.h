/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_device_h
#define INCLUDE_device_h

/* check vendor and product id of the Hantek device
   it is 'Cypress' before firmware upload and 'Dso2090' after */

#include <stdbool.h>
#include <libusb.h>

bool device_isCypress(libusb_device *device) ;
bool device_isDso2090(libusb_device *device) ;

#endif /* INCLUDE_device_h */
