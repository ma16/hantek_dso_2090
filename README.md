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

In some (rare) situations it may be useful to determine the sample-rate the device is currently using. The configuration rate needs to estimated since configuration data cannot be read back from the device.

### Example

Determine the sample-rate after firmware-upload.

```
$ ./dso2090 direct auto get status
  0* 00ac 00ae 00b0 00b2 00b4 00b6 00b8 00ba 00bc 00be 00c0 00c2 00c5 00c7 00c9 
00cb 00cd 00cf 00d1 00d3 00d5 00d7 00d9 00db 00de 00e0 00e2 00e4 00e6 00e8 00ea 
00ec 00ee 00f0 00f2 00f4 00f7 00f9 00fb 00fd 00ff 0001 0003 0005 0007 0009 000b 
000d 000f 0012 0014 0016 0018 001a 001c 001e 0020 0022 0024 0026 0028 002b 002d 
002f 0031 0033 0035 0037 0039 003b 003d 003f 0041 0044 0046 0048 004a 004c 004e 
0050 0052 0054 0056 0058 005a 005d 005f 0061 0063 0065 0067 0069 006b 006d 006f 
0071 0073 0076 0078 007a 007c 007e 0080 0082 0084 0086 0088 008a 008c 008f 0091 
0093 0095 0097 0099 009b 009d 009f 00a1 00a3 00a6 00a8 00aa 00ac 00ae 00b0 00b2 
00b4 00b6 00b8 00ba 00bc 00be 00c1 00c3 00c5 00c7 00c9 00cb 00cd 00cf 00d1 00d3 
00d5 00d7 00da 00dc 00de 00e0 00e2 00e4 00e6 00e8 00ea 00ec 00ee 00f0 00f3 00f5 
00f7 00f9 00fb 00fd 00ff 0001 0003 0005 0007 0009 000c 000e 0010 0012 0014 0016 
0018 001a 001c 001e 0020 0022 0025 0027 0029 002b 002d 002f 0031 0033 0035 0037 
0039 003b 003e 0040 0042 0044 0046 0048 004a 004c 004e 0050 0052 0054 0057 0059 
005b 005d 005f 0061 0063 0065 0067 0069 006b 006d 0070 0072 0074 0076 0078 007a 
007c 007e 0080 0082 0084 0086 0088 008b 008d 008f 0091 0093 0095 0097 0099 009b 
009d 009f 00a1 00a4 00a6 00a8 00aa 00ac 00ae 00b0 00b2 00b4 00b6 00b8 00ba 00bd 
```
The device is currently sampling data. It appears to use tiny frames since the pointer does not move beyond 0x100.

```
$ ./dso2090 rate auto vector tiny
5.00e+07 (sum=529)
```
Since the pointer moves very fast (by 529 records) the suggested sample-rate is 50 MS/s.

```
$ ./dso2090 rate auto count tiny
3.18e+05 (count=2512,missed=41)
```
This method provides only reasonable results if the pointer does not wrap around between status queries (which it however does here).

## Option: calibrate

The device already provides calibration data. This function however determines offset-voltages by trying. The channels have to be connected with ground.

```
$ ./dso2090 calibrate
arguments: DEVICE NRECORDS ADC INPUT FRAME PRESCALER COUPLING

NRECORDS: the number of records to sample per offset-voltage
```

### Example

A line of calibration data (see below) displays five groups (from left-to-right):
* the attenuations,
* the lowest offset-voltage Vos_L (in brackets) that makes the ADC produce a value above zero
* the highest offset-voltage Vos_ML (in brackets) that makes the ADC produce a value below 0x80
* the lowest offset-voltage Vos_MR (in brackets) that makes the ADC produce a value above 0x80
* the highest offset-voltage Vos_H (in brackets) that makes the ADC produce a value below 0xff

The average ADC output is shown as floating point numbers (from left-to-right):
* the average ADC output for Vos_L and for (Vos_L+1)
* the average ADC output for Vos_ML and Vos_MR
* the average ADC output for (Vos_H-1) and for Vos_H

Determine calibration data for channel 1 (ch1) on ADC 1 at 1 MS/s:
```
$ ./dso2090 calibrate auto 0x100 1 both tiny 25/25 dc
  1x 1x | [0x1d] 0.047,2.098 ... [0x54] 126.586,128.973 [0x55] ... 250.980,253.355 [0x8b]
  1x 2x | [0x10] 0.098,1.605 ... [0x47] 126.703,128.867 [0x48] ... 250.855,253.332 [0x7e]
  1x 5x | [0x0b] 0.125,2.156 ... [0x42] 126.824,128.848 [0x43] ... 251.070,253.422 [0x79]
 10x 1x | [0x1d] 0.910,2.906 ... [0x54] 127.125,129.266 [0x55] ... 252.219,254.477 [0x8b]
 10x 2x | [0x10] 0.586,2.422 ... [0x47] 127.004,129.168 [0x48] ... 251.270,254.055 [0x7e]
 10x 5x | [0x0b] 0.312,2.344 ... [0x42] 126.945,128.961 [0x43] ... 251.141,253.941 [0x79]
100x 1x | [0x1d] 1.000,3.043 ... [0x54] 127.164,129.289 [0x55] ... 252.172,254.625 [0x8b]
100x 2x | [0x10] 0.578,2.613 ... [0x47] 127.008,129.172 [0x48] ... 251.320,254.086 [0x7e]
100x 5x | [0x0b] 0.270,2.320 ... [0x42] 126.973,128.938 [0x43] ... 251.195,253.922 [0x79]
```

Determine calibration data for channel 1 (ch1) on ADC 1 at 50 MS/s:
```
$ ./dso2090 calibrate auto 0x100 1 both tiny 50/1 dc
  1x 1x | [0x1e] 0.078,1.551 ... [0x55] 126.660,128.941 [0x56] ... 251.750,254.031 [0x8c]
  1x 2x | [0x11] 0.266,2.324 ... [0x48] 126.852,129.207 [0x49] ... 252.160,254.211 [0x7f]
  1x 5x | [0x0c] 0.516,2.699 ... [0x43] 127.086,129.430 [0x44] ... 252.273,254.586 [0x7a]
 10x 1x | [0x1e] 1.090,3.395 ... [0x54] 125.656,128.055 [0x55] ... 252.895,254.926 [0x8c]
 10x 2x | [0x11] 0.879,3.188 ... [0x48] 127.473,129.730 [0x49] ... 252.488,254.797 [0x7f]
 10x 5x | [0x0c] 0.703,2.984 ... [0x43] 127.434,129.445 [0x44] ... 252.406,254.793 [0x7a]
100x 1x | [0x1e] 1.145,3.383 ... [0x54] 125.535,128.223 [0x55] ... 252.988,254.949 [0x8c]
100x 2x | [0x11] 0.895,3.059 ... [0x48] 127.238,129.719 [0x49] ... 252.441,254.812 [0x7f]
100x 5x | [0x0c] 0.719,2.992 ... [0x43] 127.281,129.473 [0x44] ... 252.555,254.770 [0x7a]
```

Determine calibration data for channel 2 (ch2) on ADC 0  at 1 MS/s:
```
$ ./dso2090 calibrate auto 0x100 0 both tiny 25/25 dc
  1x 1x | [0x14] 0.004,1.035 ... [0x4c] 127.078,129.633 [0x4d] ... 251.754,254.059 [0x83]
  1x 2x | [0x0a] 0.035,1.828 ... [0x42] 127.438,129.984 [0x43] ... 251.898,254.141 [0x79]
  1x 5x | [0x07] 0.309,2.309 ... [0x3f] 127.977,130.398 [0x40] ... 252.332,254.676 [0x76]
 10x 1x | [0x14] 0.109,1.977 ... [0x4b] 126.000,128.254 [0x4c] ... 250.102,252.707 [0x82]
 10x 2x | [0x0a] 0.312,2.238 ... [0x41] 125.988,128.234 [0x42] ... 252.445,254.973 [0x79]
 10x 5x | [0x07] 0.441,2.684 ... [0x3e] 125.914,128.223 [0x3f] ... 252.367,254.895 [0x76]
100x 1x | [0x14] 0.137,2.055 ... [0x4b] 126.043,128.527 [0x4c] ... 250.387,252.547 [0x82]
100x 2x | [0x0a] 0.363,2.250 ... [0x41] 126.039,128.266 [0x42] ... 252.656,254.969 [0x79]
100x 5x | [0x07] 0.473,2.652 ... [0x3e] 125.980,128.188 [0x3f] ... 252.648,254.867 [0x76]
```

Determine calibration data for channel 2 (ch2) on ADC 0  at 50 MS/s:
```
$ ./dso2090 calibrate auto 0x100 0 both tiny 50/1 dc
  1x 1x | [0x15] 0.012,1.305 ... [0x4d] 127.262,129.977 [0x4e] ... 252.996,254.996 [0x84]
  1x 2x | [0x0b] 0.426,2.734 ... [0x42] 126.332,128.863 [0x43] ... 251.305,253.961 [0x79]
  1x 5x | [0x08] 1.109,3.508 ... [0x3f] 126.898,129.457 [0x40] ... 251.625,254.105 [0x76]
 10x 1x | [0x15] 0.477,2.801 ... [0x4c] 126.488,129.195 [0x4d] ... 251.602,254.473 [0x83]
 10x 2x | [0x0b] 0.957,3.117 ... [0x42] 126.934,129.266 [0x43] ... 252.121,254.129 [0x79]
 10x 5x | [0x08] 1.176,3.883 ... [0x3f] 127.031,129.797 [0x40] ... 251.996,254.309 [0x76]
100x 1x | [0x15] 0.539,2.934 ... [0x4c] 126.508,129.238 [0x4d] ... 252.012,254.191 [0x83]
100x 2x | [0x0b] 0.996,3.301 ... [0x42] 127.051,129.426 [0x43] ... 252.012,254.207 [0x79]
100x 5x | [0x08] 1.234,3.855 ... [0x3f] 127.031,129.832 [0x40] ... 252.047,254.254 [0x76]
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
