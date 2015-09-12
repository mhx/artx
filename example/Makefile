# -*- makefile -*-

ARTX_ROOT = ..

# MCU name
MCU = atmega32u4

# Main Oscillator Frequency
F_OSC = 16000000

# Build for debug / release.
BUILD = release

# Target file name (without extension).
TARGET = example

# Compiler configuration
CDEFS = -DARTX_CONFIG_H=artx_config.h
CINCS = -I.

# List C source files here. (C dependencies are automatically generated.)
SRC = example.c

# Optimization level
OPT = s

# Programmer config
AVRDUDE_PROGRAMMER = avr109
AVRDUDE_PORT = /dev/ttyACM0    # programmer connected to serial device

include $(ARTX_ROOT)/artx.mk
