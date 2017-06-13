/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "usbExt.h"
#include <assert.h>
#include <string.h>
#include "dod.h"

/* ----------------------------------------------------------------- */

libusb_context* usbExt_init()
{
    libusb_context *libusb ;
    int result = libusb_init(&libusb) ;
    if (result < 0)
	dod_exit("libusb_init:%s",libusb_error_name(result)) ;
    return libusb ;
}

/* ----------------------------------------------------------------- */

bool usbExt_browse(libusb_context *libusb,
		   bool (*callback)(void*,libusb_device*),
		   void *cb_data) 
{
    libusb_device **deviceV ;
    ssize_t n = libusb_get_device_list(libusb,&deviceV) ;
    if (n < 0)
    {
	int error = (int)n ; /* ssize_t vs int */
	dod_exit("libusb_get_device_list:%s",libusb_error_name(error)) ;
    }

    bool stopped = false ;
    for (size_t i=0 ; deviceV[i] && !stopped ; ++i)
    {
	stopped = callback(cb_data,deviceV[i]) ;
    }
    
    libusb_free_device_list(deviceV,1) ;
    return stopped ;
}

/* ----------------------------------------------------------------- */

uint16_t usbExt_vendor(libusb_device *device)
{
    struct libusb_device_descriptor dev_dsc;
    int result = libusb_get_device_descriptor(device,&dev_dsc);
    assert(result == 0) ; (void)result ;
    return dev_dsc.idVendor ;
}

uint16_t usbExt_product(libusb_device *device)
{
    struct libusb_device_descriptor dev_dsc;
    int result = libusb_get_device_descriptor(device,&dev_dsc);
    assert(result == 0) ; (void)result ;
    return dev_dsc.idProduct ;
}

UsbExt_Addr usbExt_addr(libusb_device *device)
{
    uint8_t bus = libusb_get_bus_number    (device) ;
    uint8_t dev = libusb_get_device_address(device) ;
    UsbExt_Addr addr ;
    dod_snprintf(addr.s,sizeof(addr.s),"%d:%d",bus,dev) ;
    return addr ;
}

UsbExt_Path usbExt_path(libusb_device *device)
{
    UsbExt_Path path ;
    dod_snprintf(path.s,sizeof(path.s),"%d",libusb_get_bus_number(device)) ;
    
    uint8_t portV[7] ;
    int nports = libusb_get_port_numbers(device,portV,sizeof(portV)) ;
    if (nports < 0)
	dod_exit("libusb_get_port_numbers:%s",libusb_error_name(nports)) ;
    if (nports == 0)
	return path ;

    for (int i=0 ; i<nports ; ++i)
    {
	size_t len = strlen(path.s) ;
	dod_snprintf(path.s+len,sizeof(path.s)-len,".%d",portV[i]) ;
    }
    return path ;
}

/* ----------------------------------------------------------------- */

libusb_device* usbExt_findAddr(libusb_context *libusb,UsbExt_Addr const *addr)
{
    libusb_device **deviceV ;
    ssize_t n = libusb_get_device_list(libusb,&deviceV) ;
    if (n < 0)
    {
	int error = (int)n ; /* ssize_t vs int */
	dod_exit("libusb_get_device_list:%s",libusb_error_name(error)) ;
    }

    libusb_device *device = NULL ;
    for (size_t i=0 ; deviceV[i]!=NULL ; ++i)
    {
	UsbExt_Addr addr_ = usbExt_addr(deviceV[i]) ;
	if (0 == strcmp(addr->s,addr_.s))
	{
	    if (device != NULL)
		dod_exit("usbExt:ambiguous address <%s>",addr->s) ;
	    device = deviceV[i] ;
	}
	else libusb_unref_device(deviceV[i]) ;
    }
    
    libusb_free_device_list(deviceV,0) ;
    return device ;
}

/* ----------------------------------------------------------------- */

libusb_device* usbExt_findPath(libusb_context *libusb,UsbExt_Path const *path)
{
    libusb_device **deviceV ;
    ssize_t n = libusb_get_device_list(libusb,&deviceV) ;
    if (n < 0)
    {
	int error = (int)n ; /* ssize_t vs int */
	dod_exit("libusb_get_device_list:%s",libusb_error_name(error)) ;
    }

    libusb_device *device = NULL ;
    for (size_t i=0 ; deviceV[i]!=NULL ; ++i)
    {
	UsbExt_Path path_ = usbExt_path(deviceV[i]) ;
	if (0 == strcmp(path->s,path_.s))
	{
	    if (device != NULL)
		dod_exit("usbExt:ambiguous usb-path <%s>",path->s) ;
	    device = deviceV[i] ;
	}
	else libusb_unref_device(deviceV[i]) ;
    }
    
    libusb_free_device_list(deviceV,0) ;
    return device ;
}

/* ----------------------------------------------------------------- */

libusb_device_handle* usbExt_open(libusb_device *device)
{
    libusb_device_handle *handle ;
    int result = libusb_open(device,&handle) ;
    if (result < 0)
	dod_exit("libusb_open:%s",libusb_error_name(result)) ;
    return handle ;
}

libusb_device_handle* usbExt_openAddr(libusb_context  *libusb,
				      UsbExt_Addr const *addr)
{
    libusb_device *device = usbExt_findAddr(libusb,addr) ;
    if (device == NULL)
	return NULL ;
    libusb_device_handle *handle = usbExt_open(device) ;
    libusb_unref_device(device) ;
    return handle ;
}

libusb_device_handle* usbExt_openPath(libusb_context  *libusb,
				      UsbExt_Path const *path)
{
    libusb_device *device = usbExt_findPath(libusb,path) ;
    if (device == NULL)
	return NULL ;
    libusb_device_handle *handle = usbExt_open(device) ;
    libusb_unref_device(device) ;
    return handle ;
}

/* ----------------------------------------------------------------- */
