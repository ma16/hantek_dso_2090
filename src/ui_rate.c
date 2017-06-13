/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

/* user-interface: try to estimate the currently active sample-rate */

#include "ui.h"
#include <stdio.h>
#include <string.h>
#include "dod.h"
#include "hantekDlg.h"
#include "optDevice.h"
#include "optHantek.h"
#include "systime.h"

static void count(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf(
	    "arguments: FRAME [-d DURATION] [-t TIMEOUT]\n"
	    "\n"
	    "DURATION : time to count in seconds (1)\n"
	    " TIMEOUT : ignore counts that take more time (5e-4 seconds)\n"
	    ) ;
	return ;
    }

    HantekDlg_FrameId frameId = optHantek_FrameId(opt) ;
    static uint16_t const frameSizeV[] = { 0x100,0x2800,0x8000 } ;
    uint16_t frameSize = frameSizeV[frameId.e] ;

    double duration = dod_strtod(opt_preset(opt,"-d","1.0")) ;
    double timeout  = dod_strtod(opt_preset(opt,"-t","5e-4")) ;
    
    opt_finish(opt) ;
    
    /* captured data is put into a ring-buffer (the frame); the offset
       indicates the present position in the buffer; with a sampling
       rate of 10 Mhz and a frame holding 10k samples, we need to read
       the offset every 1ms - otherwise the offset outruns the frame
       size: the actual time to read the offset (which requires 3x usb 
       transfer) is about 400us; hence measurements beyond 10 MHz are
       getting less and less reliable; also consider that the process 
       execution may be suspended at any time (which also let the 
       offset outrun). */

    double   t_sum = 0 ; /* accumulated time in seconds */
    uint64_t o_sum = 0 ; /* accumulated (total) offset */

    unsigned    n = 0 ; /* number of valid counts */
    unsigned miss = 0 ; /* number of missed counts due to timeout */

    double t_0 = systime_get() ;

    while (true)
    {
	/* start counting / restart after timeout */
	
	double   t_1 = systime_get() ;
	uint16_t o_x = hantekDlg_getStatus(handle).pointer[0] ;
	double   t_x = systime_get() ;

	if (t_x - t_0 > duration)
	    break ;

	if (t_x - t_1 > timeout)
	{
	    ++miss ;
	    continue ;
	}

	while (true)
	{
	    uint16_t o_y = hantekDlg_getStatus(handle).pointer[0] ;
	    double   t_y = systime_get() ;
	    
	    if (t_y - t_0 > duration)
		break ;
	    
	    if (t_y - t_x > timeout)
	    {
		++miss ;
		break ;
	    }
	    
	    o_sum += (o_y >= o_x)
	      ? (uint16_t)(          - o_x + o_y)
	      : (uint16_t)(frameSize - o_x + o_y) ;
	    t_sum += t_y - t_x ;
	    
	    ++n ;

	    o_x = o_y ;
	    t_x = t_y ;
	}
    }

    printf("%.2e (count=%u,missed=%u)\n", (double)o_sum / t_sum,n,miss) ;
}

static void vector(libusb_device_handle *handle,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf("arguments: FRAME\n") ;
	return ;
    }

    uint16_t frameSize = hantekDlg_frameSize(optHantek_FrameId(opt)) ;
    opt_finish(opt) ;

    
    HantekDlg_Status status = hantekDlg_getStatus(handle) ;

    uint32_t sum = 0 ;
    for (int i=0 ; i<0xff-1 ; ++i)
    {
	uint16_t prev = status.pointer[i+0] ;
	uint16_t next = status.pointer[i+1] ;
	sum += (next >= prev) 
	    ? (uint16_t)(          - prev + next)
	    : (uint16_t)(frameSize - prev + next) ;
    }
    printf("%.2e (sum=%u)\n",2.41e+7/(0xff)*sum,sum) ;
}

void ui_rate(libusb_context *libusb,Opt *opt)
{
    if (opt_end(opt) || opt_popIf(opt,"help"))
    {
	printf("Try to determine the current sample-rate\n"
	       "\n"
	       "arguments: DEVICE MODE [help]\n"
	       "\n"
	       "MODE : count   # count frame-pointer-increments and measure time\n"
	       "     | vector  # estimate by pointer-difference (in status query)\n"
	       "\n"
	       "Use vector on high sample-rates, otherwise count.\n"
	       ) ;
	return ;
    }
    libusb_device_handle *handle = optDevice_dso2090(opt,libusb) ;
    
    char const *arg = opt_pop(opt) ;
    if (false) ;

    else if (0 == strcmp(arg, "count"))  count(handle,opt) ;
    else if (0 == strcmp(arg,"vector")) vector(handle,opt) ;
    
    else dod_exit("option not recognized:<%s>",arg) ;

    libusb_close(handle) ;
}
