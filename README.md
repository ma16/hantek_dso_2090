# hantek_dso_2090 -- A console application for Hantek's DSO-2090

This projects is intended to dig a bit deeper into the device details.

Have a look into src/hantekDlg.h for device information.

```
$ ./dso2090 
options: acquire | calibrate | device | direct | help | rate
```

## device
```
$ ./dso2090 device scan
Hantek DSO-2090 (w/o firmware) on address <1:7> at path <1.1.4>
$ ./dso2090 device upload 1:7 hantek-dso-2090.fw 
wait for device to pop (this may take 2 seconds)... success
$ ./dso2090 device scan
Hantek DSO-2090 (with firmware) on address <1:8> at path <1.1.4>
```
See sigrok.org/wiki/Hantek_DSO-2xxx/52xx on how to get the firmware file.

## direct
```
$ ./dso2090 direct
arguments: DEVICE MODE [help]

MODE : capture  # tell device to capture data
     | fetch    # transfer frame from device to host
     | get      # query status or calibration
     | set      # tell device to change configuration
     | trigger  # tell device to arm or release trigger
```

## rate
```
$ ./dso2090 rate
Try to determine the current sample-rate

arguments: DEVICE MODE [help]

MODE : count   # count frame-pointer-increments and measure time
     | vector  # estimate by pointer-difference (in status query)

Use vector on high sample-rates, otherwise count.
```

## calibrate
```
$ ./dso2090 calibrate
arguments: DEVICE NRECORDS ADC INPUT FRAME PRESCALER COUPLING

NRECORDS: the number of records to sample per offset-voltage
```

## acquire
```
Acquire a frame.
options: DEVICE RELEASE CONFIG [CALIBRATION]
    RELEASE : immediately (yes) or wait for trigger signal (no)
     CONFIG : file with configuration data
CALIBRATION : file with calibration data
```

## help
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
