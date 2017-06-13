/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_optDevice_h
#define INCLUDE_optDevice_h

/* open device by command line argument */

#include <libusb.h>
#include "opt.h"

char const* optDevice_help(void) ;
  
libusb_device_handle* optDevice_cypress(Opt *opt,libusb_context *libusb) ;
libusb_device_handle* optDevice_dso2090(Opt *opt,libusb_context *libusb) ;

#endif /* INCLUDE_optDevice_h */
