/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_ui_h
#define INCLUDE_ui_h

/* user interface */

#include "opt.h"
#include <libusb.h>

void ui_acquire  (libusb_context*,Opt*) ;
void ui_calibrate(libusb_context*,Opt*) ;
void ui_device   (libusb_context*,Opt*) ;
void ui_direct   (libusb_context*,Opt*) ;
void ui_rate     (libusb_context*,Opt*) ;

#endif /* INCLUDE_ui_h */
