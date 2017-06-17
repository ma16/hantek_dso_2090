# A console application for Hantek's DSO-2090

This projects is intended to dig a bit deeper into the device details.

Have a look into src/hantekDlg.h for device information.

## The Tool
```
$ ./dso2090 
options: acquire | calibrate | device | direct | help | rate
```

## Option: device
```
$ ./dso2090 device help
Scan for devices and upload firmware.
arguments: MODE
MODE : scan                # list all Hantek DSO-2090
     | upload DEVICE FILE  # upload FILE to DEVICE
```
See sigrok.org/wiki/Hantek_DSO-2xxx/52xx on how to get the firmware file.

### Example

```
$ ./dso2090 device scan
Hantek DSO-2090 (w/o firmware) on address <1:7> at path <1.1.4>
$ ./dso2090 device upload 1:7 hantek-dso-2090.fw 
wait for device to pop (this may take 2 seconds)... success
$ ./dso2090 device scan
Hantek DSO-2090 (with firmware) on address <1:8> at path <1.1.4>
```

## Option: direct
```
$ ./dso2090 direct
Initiate a device dialog.
arguments: DEVICE MODE [help]
MODE : capture  # tell device to capture data
     | fetch    # transfer frame from device to host (stdout)
     | get      # query status or calibration
     | set      # tell device to change configuration
     | trigger  # tell device to arm or release trigger
```

### Example

Display the calibration data:
```
$ ./dso2090 direct auto get calibration
                 ch1                     ch2          
       ----------------------- -----------------------
  1x | (1d,8f) (10,82) ( b,7c) (16,81) ( a,79) ( 8,77)
 10x | (1e,8f) (11,82) ( b,7c) (13,84) ( a,7b) ( 7,78)
100x | (1e,8f) (11,82) ( b,7c) (14,85) ( a,7b) ( 7,77)
       ------- ------- ------- ------- ------- -------
          1x      2x      5x       1x     2x      5x  
```
There are 18 couples of OFFSET_VOLTAGEs. Each of them dedicated to a certain channel and an attenuation.

Set multiplexer attenuation:
```
$ ./dso2090 direct auto set mux 2x 5x
```
* For channel 'ch1' set MUX_ATTN to '2x'.
* For channel 'ch2' set MUX_ATTN to '5x'.

Set relay attenuation et al.:
```
$ ./dso2090 direct auto set relay 100x dc 10x ac off
```
* For channel 'ch1' set RELAY_ATTN to '100x'. This results in 16 Vpp (together with MUX_ATTN=2x)
* For channel 'ch2' set RELAY_ATTN to '10x'.  This results in 4 Vpp (together with MUX_ATTN=5x).

The command sets also AC/DC coupling.
* For channel 'ch1' set COUPLING to 'dc' (direct current).
* For channel 'ch2' set COUPLING to 'ac' (alternating current).

The command sets also the external trigger-input to 'off'. However, the actual value doesn't matter since we're going to trigger manually.


Set offset-voltage et al.:
```
$ ./dso2090 direct auto set offset 0x11 0x3f 0x80
```
* For channel 'ch1' set OFFSET_VOLTAGE to 0x11. (see calibration index 2x/100x)
* For channel 'ch2' set OFFSET_VOLTAGE to 0x3f. (see calibration index 5x/10x)

Since 'ch2' uses AC-coupling, the midpoint of (7,78) is chosen.

The command sets also the TRIGGER_LEVEL to 0x80. However, the actual value doesn't matter since we're going to trigger manually.

Set input-selection et al.:
```
$ ./dso2090 direct auto set input both tiny 25/25 tiny ch1 rise
```
* Use 'both' channels to sample data.
* Sample records into a 'tiny' frame (256 records).
* Sample with 1 MS/s (25/25).
* Sample 0x100 records (a complete 'tiny' frame) after trigger-release.

The command sets also the trigger-source ('ch1') and signal-edge ('rise'). However, the actual values don't matter since we're going to trigger manually.

Capture frame (release trigger manually):
```
$ ./dso2090 direct auto capture ; \
  ./dso2090 direct auto trigger enable ; \
  ./dso2090 direct auto trigger release ; 
```
* Start sampling.
* Enable the trigger.
* Release the trigger (so we don't depend on a certain signal value/shape).

Query status:
```
$ ./dso2090 direct auto get status
  2* 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 00b9 
```
The state '2*' says the sampling has finished. (This is only the first line of the ouput.)

Fetch the frame (binary to stdout):

```
$ ./dso2090 direct auto fetch | od -Ax -txC
000000 e7 37 e7 37 e7 37 e7 37 e8 36 e8 36 e8 36 e8 36
000010 e8 36 e8 36 e8 36 e8 36 e8 36 e8 36 e7 36 e8 37
000020 e8 36 e8 36 e8 36 e8 37 e9 36 e8 36 e8 36 e7 37
000030 e8 36 e8 36 e8 36 1e 06 1d 05 1c 04 1b 04 1b 04
000040 1b 04 1b 04 1a 03 1b 03 1a 03 1a 03 1a 03 1a 03
000050 1a 03 1a 03 1a 02 19 02 1a 03 1a 03 1a 03 1a 03
000060 19 03 19 03 19 03 19 03 19 03 1a 03 1a 03 1a 03
000070 1a 02 1a 02 1a 03 1a 03 19 03 1a 03 19 03 19 03
000080 19 03 19 03 19 03 19 03 1a 03 1a 03 1a 03 1a 03
000090 1a 03 19 02 1a 03 19 03 1a 03 e3 33 e4 34 e6 35
0000a0 e6 35 e6 36 e6 36 e6 36 e7 36 e7 37 e7 37 e7 36
0000b0 e7 36 e8 36 e7 36 e8 36 e8 35 e8 36 e7 36 e7 36
0000c0 e8 36 e8 36 e8 37 e7 37 e8 36 e8 37 e8 36 e9 37
0000d0 e9 36 e9 36 e8 36 e9 36 e8 36 e8 36 e8 36 e9 36
0000e0 e8 36 e8 36 e8 36 e8 36 e8 36 e8 36 e8 36 e8 37
0000f0 e9 36 e8 36 e8 36 e8 36 e8 35 e8 36 e8 36 1e 06
000100 1d 06 1c 05 1c 04 1b 04 1b 04 1b 04 1a 03 1a 03
000110 1a 03 1a 03 1a 03 1a 03 1a 03 1a 02 1a 03 1a 03
000120 1a 03 19 03 19 03 19 03 19 03 19 03 19 03 19 03
000130 19 03 19 03 1a 03 1a 03 19 03 19 03 19 02 1a 02
000140 19 03 1a 03 19 03 19 03 1a 03 19 03 19 03 19 03
000150 19 03 1a 03 1a 03 19 02 1a 02 1a 03 1a 03 e8 37
000160 e8 36 e8 37 e8 36 e8 36 e8 36 e8 36 e8 36 1e 06
000170 1d 05 1c 04 1b 04 1b 04 1b 03 1b 03 1b 03 1a 04
000180 1a 03 1b 03 1a 03 1a 03 1a 03 1a 03 1a 03 19 03
000190 1a 02 1a 03 1a 03 1a 02 1a 03 19 03 1a 03 19 03
0001a0 1a 03 19 03 1a 03 19 03 1a 03 1a 03 19 03 1a 03
0001b0 19 02 1a 02 1a 03 1a 03 19 03 19 03 19 03 19 03
0001c0 19 03 19 03 19 03 19 03 19 03 19 03 1a 03 19 03
0001d0 1a 03 e3 33 e5 34 e5 34 e6 35 e6 35 e6 35 e6 35
0001e0 e7 36 e7 37 e7 36 e7 36 e8 36 e8 37 e7 36 e8 36
0001f0 e8 36 e8 36 e8 36 e8 36 e8 36 e8 36 e8 36 e7 37
000200
```
The input signal is a square wave at 10 kHz with about ~3.3 Vpp. So about 50 low values alternated with about 50 high values (1 MS/s on 10 kHz signal).

* ADC-0 (here channel-2) writes the first byte of a record. The value alternates between 0x19 and 0xe9 which represent -1.61 V and +1.64V (in the given 4 Vpp AC range).
* ADC-1 (here channel-1) writes the second byte of a record. The value alternates between 0x02 and 0x37 which represent 0.13V and 3.44 V (in the given 16 Vpp DC range).

## Option: rate
```
$ ./dso2090 rate
Try to determine the current sample-rate

arguments: DEVICE MODE [help]

MODE : count   # count frame-pointer-increments and measure time
     | vector  # estimate by pointer-difference (in status query)

Use vector on high sample-rates, otherwise count.
```

## Option: calibrate
```
$ ./dso2090 calibrate
arguments: DEVICE NRECORDS ADC INPUT FRAME PRESCALER COUPLING

NRECORDS: the number of records to sample per offset-voltage
```

## Option: acquire
```
$ ./dso2090 acquire
Acquire a frame.
options: DEVICE RELEASE CONFIG [CALIBRATION]
    RELEASE : immediately (yes) or wait for trigger signal (no)
     CONFIG : file with configuration data
CALIBRATION : file with calibration data
```

## Option: help
```
$ ./dso2090 help
USB-device selection

DEVICE: the USB device to use
    'auto'  # try to find device automatically
    [-f] BUS ':' 'ADDR'     # e.g. '1:42'
    [-f] BUS ['.' PORT]...  # e.g. '1.2.3'
use -f to ignore the vendor- and product-id

List of Hantek parameters

ADC: two analog-to-digital converters
    '0'  # ADC-0
    '1'  # ADC-1

CHANNEL: two input channels
    'ch1'  # channel-1
    'ch2'  # channel-2

COUPLING: per channel
    'ac'  # alternating current
    'dc'  #      direct current

FRAME: three frame sizes are available
     '256' |   '10k' |   '32k'
    'tiny' | 'small' | 'large'

INPUT: define ADC's signal source
    'ch1'   # channel-1 goes to both ADCs
    'ch2'   # channel-2 goes to both ADCs
    'both'  # channel-1 to ADC-1, channel-2 to ADC-0
    'none'

MUX_ATTN: attenuation switched by multiplexer
    '1x' | '2x' | '5x'

OFFSET_VOLTAGE: raise channel's signal voltage
    0..0xFFFF

PRESCALER: set sample-rate
    'max'                     # 50 MHz
    '50/1'                    # 50 MHz
    '50/2'                    # 25 MHz
    '50/5'                    # 10 MHz
    '25/' 2..65537            # in MHz
    3.81e+02 .. 1.25e+07      # in MHz
    8.00e-08 .. 2.62e-03 's'  # as f := 1/s
    '=' 0..0xFFFF             # internal encoding

RELAY_ATTN : attenuation switched by relays
    '1x' | '10x' | '100x'

TRIGGER_COUNT: (should match FRAME size)
    1..0x8000
     '256' |   '10k' |   '32k'
    'tiny' | 'small' | 'large'
    '=' 0..0xFFFFFF

SLOPE: to release the trigger
    'rise' | 'fall'

TRIGGER_LEVEL: to release the trigger
    0..0xFFFF
```
