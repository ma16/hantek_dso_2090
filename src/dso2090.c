#include <stdio.h>
#include <string.h>
#include "dod.h"
#include "opt.h"
#include "optDevice.h"
#include "ui.h"
#include "textHantek.h"

int main(int argc,char *argv[])
{
    Opt opt = opt_init(argc-1,argv+1) ;
    
    if (opt_end(&opt))
    {
	printf("options: acquire | calibrate | device | direct | help | rate\n") ;
	return 0 ;
    }
    if (opt_popIf(&opt,"help"))
    {
	printf("USB-device selection\n\n%s\n",optDevice_help()) ;
	printf("List of Hantek parameters\n\n%s",textHantek_help()) ;
	return 0 ;
    }
    libusb_context *libusb = usbExt_init() ;

    char const *arg = opt_pop(&opt) ;
    if (false) ;

    else if (0 == strcmp(arg,   "acquire"))   ui_acquire(libusb,&opt) ;
    else if (0 == strcmp(arg, "calibrate")) ui_calibrate(libusb,&opt) ;
    else if (0 == strcmp(arg,    "device"))    ui_device(libusb,&opt) ;
    else if (0 == strcmp(arg,    "direct"))    ui_direct(libusb,&opt) ;
    else if (0 == strcmp(arg,      "rate"))      ui_rate(libusb,&opt) ;
    
    else dod_exit("option not recognized:<%s>",arg) ;
    
    libusb_exit(libusb) ;
    return 0 ;
}

