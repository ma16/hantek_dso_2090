/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

/* user-interface: a demo to acquire data */

#include "ui.h"
#include <stdio.h>
#include <string.h>
#include "dod.h"
#include "hantekExt.h"
#include "optDevice.h"
#include "systime.h"
#include "textHantek.h"

void ui_acquire(libusb_context *libusb,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "Acquire a frame.\n"
	    "options: DEVICE RELEASE CONFIG [CALIBRATION]\n"
	    "    RELEASE : immediately (yes) or wait for trigger signal (no)\n"
	    "     CONFIG : file with configuration data\n"
	    "CALIBRATION : file with calibration data\n"
	    ) ;
	return ;
    }

    libusb_device_handle *handle = optDevice_dso2090(opt,libusb) ;
    bool release = opt_bool(opt) ;
    HantekExt_Config c = hantekExt_loadConfig(opt_pop(opt)) ;

    if (!opt_end(opt))
    {
	/* use calibration to replace voltage offset */
	HantekExt_Calibration d = hantekExt_loadCalibration(opt_pop(opt)) ;
	int rix = c.channel[0].relayAttn.e , mix = c.channel[0].muxAttn.e ;
	c.channel[0].offset = d.v[0][rix][mix] ;
	rix = c.channel[1].relayAttn.e ; mix = c.channel[1].muxAttn.e ;
	c.channel[1].offset = d.v[1][rix][mix] ;
    }
    opt_finish(opt) ;

    /* [todo] at this point we might want to 
       abort an already released trigger, if any */
    
    /* capture and... */
    hantekExt_xferConfig(handle,&c) ;
    uint16_t pointer = hantekExt_capture(handle,release).pointer[0] ;
    uint16_t frameSize = hantekDlg_frameSize(c.frame) ;
    if (pointer >= frameSize)
	/* see hantekExt_capture() for details */
	dod_exit("capture-pointer beyond frame-size:%u",pointer) ;
    /* [todo] introduce a progress indicator */

    /* ...fetch data */
    uint8_t buffer[2u*hantekDlg_maxFrameSize] ; /* 2 bytes per record */
    memset(buffer,0xff,sizeof(buffer)) ; /* preset to ease [debugging] */
    hantekDlg_fetch(handle,buffer,2u*frameSize) ;

    /* dump data: [pointer,frame-end) + [frame-start,pointer) */
    size_t offset = 2u * (pointer+1u) ;
    size_t nbytes = 2u * frameSize - offset ;
    size_t nwritten = fwrite(buffer+offset,1,nbytes,stdout) ;
    if (nwritten != nbytes)
	dod_exit("fwrite(%zu):an error occurred after writing %zu bytes",
		 nbytes,nwritten) ;
    nwritten = fwrite(buffer,1,offset,stdout) ;
    if (nwritten != offset)
	dod_exit("fwrite(%u):an error occurred after writing %zu bytes",
		 2u*frameSize,nbytes+nwritten) ;
    /* ...we might want to write data to file instead to stdout */
    
    libusb_close(handle) ;
    /* [todo] 
       -- support multi-frames / rolling data
       -- support time-stamp for dumped data
       -- support pre-trigger samples */
}

