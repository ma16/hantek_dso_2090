# BSD 2-Clause License, see https://github.com/ma16/hantek_dso_2090 

INCLUDE=-I/usr/include/libusb-1.0

CFLAGS=\
	-fno-strict-aliasing \
	-g \
	-O2 \
	-pedantic \
	-std=c99 \
	-Wall \
	-Wconversion \
	-Werror=implicit-function-declaration \
	-Wextra \
	-Wmissing-prototypes \
	-Wstrict-aliasing 

LDFLAGS=-lm -lusb-1.0

CC=gcc $(INCLUDE) $(CFLAGS)

LD=gcc

LSRC=\
	cypress.c \
	device.c \
	dod.c \
	hantekDlg.c \
	hantekExt.c \
	opt.c \
	optDevice.c \
	optHantek.c \
	stdc.c \
	systime.c \
	text.c \
	textHantek.c \
	ui_acquire.c \
	ui_calibrate.c \
	ui_device.c \
	ui_direct.c \
	ui_rate.c \
	usbExt.c

BSRC=dso2090.c

-include Makefile.default

