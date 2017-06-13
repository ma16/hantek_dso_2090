/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "optDevice.h"
#include <assert.h>
#include <string.h>
#include "device.h"
#include "dod.h"
#include "usbExt.h"

/* -------------------------------------------------------------------- */

static UsbExt_Addr to_addr(char const *s)
{
    UsbExt_Addr addr ;
    if (strlen(s) >= sizeof(addr.s))
	dod_exit("opt:usb-addr too large:<%s>",s) ;
    strcpy(addr.s,s) ;
    return addr ;
    /* [improve] make sure bus and address are numbers*/
}

static UsbExt_Path to_path(char const *s)
{
    UsbExt_Path path ;
    if (strlen(s) >= sizeof(path.s))
	dod_exit("opt:usb-path too large:<%s>",s) ;
    strcpy(path.s,s) ;
    return path ;
    /* [improve] make sure bus and ports are numbers*/
}

/* locate a device by its unique device-address or port-path */
static libusb_device* locate(libusb_context *libusb,char const *s)
{
    if (strchr(s,':'))
    {
	/* colon as number separator */
	UsbExt_Addr addr = to_addr(s) ;
	return usbExt_findAddr(libusb,&addr) ;
    }
    else
    {
	/* periods as number separator */
	UsbExt_Path path = to_path(s) ;
	return usbExt_findPath(libusb,&path) ;
    }
}

/* -------------------------------------------------------------------- */

typedef struct
{
    bool(*match)(libusb_device*) ;
    libusb_device_handle *handle ;
}
Find ;

static bool find_id_callback(void *self,libusb_device *device)
{
    /* this is a callback from usbExt_browse() */
    Find *find = self ;
    if (find->match(device))
    {
	if (find->handle != NULL)
	    dod_exit("found more than one matching usb-device") ;
	find->handle = usbExt_open(device) ;
    }
    return false ;
}


/* find certain device; this fails if no or 
   if more than one of those devices exist */
static libusb_device_handle* find_id(
    libusb_context *libusb,bool(*match)(libusb_device*))
{
    Find find = { .match=match,.handle=NULL } ;
    usbExt_browse(libusb,find_id_callback,&find) ;
    if (find.handle == NULL)
	dod_exit("no matching usb-device found") ;
    return find.handle ;
}

/* -------------------------------------------------------------------- */

/* open device: 'auto' | '-f'? (ADDR | PATH) */
static libusb_device_handle* optDevice(
    Opt *opt,libusb_context *libusb,bool(*match)(libusb_device*))
{
    char const *s = opt_pop(opt) ;
    if (0 == strcmp(s,"auto"))
	return find_id(libusb,match) ;
    
    bool force = (0 == strcmp(s,"-f")) ;
    if (force)
	s = opt_pop(opt) ;
    libusb_device *device = locate(libusb,s) ;
    if (device == NULL)
	dod_exit("usb-device not found:<%s>",s) ;
    if (!force && !match)
	dod_exit("usb-device at %s does not match",s) ;
    libusb_device_handle *handle = usbExt_open(device) ;
    libusb_unref_device(device) ;
    return handle ;
}

/* -------------------------------------------------------------------- */

libusb_device_handle* optDevice_cypress(Opt *opt,libusb_context *libusb)
{
    return optDevice(opt,libusb,&device_isCypress) ;
}

libusb_device_handle* optDevice_dso2090(Opt *opt,libusb_context *libusb)
{
    return optDevice(opt,libusb,&device_isDso2090) ;
}

/* -------------------------------------------------------------------- */

char const* optDevice_help()
{
    static char const s[] =
	"DEVICE: the USB device to use\n"
	"    'auto'  # try to find device automatically\n"
	"    [-f] BUS ':' 'ADDR'     # e.g. '1:42'\n"
	"    [-f] BUS ['.' PORT]...  # e.g. '1.2.3'\n"
	"use -f to ignore the vendor- and product-id\n" ;
    return s ;
}
