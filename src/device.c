/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "device.h"
#include "usbExt.h"

bool device_isCypress(libusb_device *device)
{
    return
        (0x2090 == usbExt_product(device)) &&
	(0x04b4 == usbExt_vendor (device)) ;
}

bool device_isDso2090(libusb_device *device)
{
    return
        (0x2090 == usbExt_product(device)) &&
	(0x04b5 == usbExt_vendor (device)) ;
}
