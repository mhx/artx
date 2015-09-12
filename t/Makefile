# -*- makefile -*-

ARTX_ROOT = ..

# MCU name
MCU =

# Main Oscillator Frequency
F_OSC = 1000000

# Build for debug / release.
BUILD = release

# Target file name (without extension).
TARGET =

# Compiler configuration
CDEFS = -DARTX_CONFIG_H=testconfig.h -DARTX_UNDER_TEST
CINCS = -I.

# List C source files here. (C dependencies are automatically generated.)
SRC = artxtest.c

# Optimization level
OPT = s

include $(ARTX_ROOT)/artx.mk
