# Project Name
TARGET = SynthV10

# Sources
CPP_SOURCES = SynthV10.cpp DisplayOLED.cpp Mcp23017.cpp Parameters.cpp Button.cpp Encoder_mcp.cpp MidiHandler.cpp Voice.cpp

USE_DAISYSP_LGPL=1
LDFLAGS += -u _printf_float
APP_TYPE = BOOT_NONE

# Library Locations
LIBDAISY_DIR = ../DaisyExamples/libDaisy
DAISYSP_DIR = ../DaisyExamples/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
