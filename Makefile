# Project Name
TARGET = SynthV10

# Sources
CPP_SOURCES = main.cpp \
			  midi_handler.cpp \
			  display.cpp \
			  parameters.cpp \
			  voice.cpp \
			  effects.cpp \
			  menu.cpp \
			  oscillator.cpp \
			  sx1509_expander.cpp \
			  log_uart.cpp \
			  reverb.cpp \
	$(OLED_DIR)/OLED_Driver.cpp $(OLED_DIR)/GUI_Paint.cpp \
	$(SX1509_DIR)/SX1509.cpp \
	$(FONTS_DIR)/8-bold.cpp $(FONTS_DIR)/12-bold.cpp $(FONTS_DIR)/16-bold.cpp $(FONTS_DIR)/20-bold.cpp $(FONTS_DIR)/24-bold.cpp \
	$(FONTS_DIR)/Regular_8.cpp $(FONTS_DIR)/Regular_12.cpp $(FONTS_DIR)/Regular_16.cpp $(FONTS_DIR)/Regular_20.cpp $(FONTS_DIR)/Regular_24.cpp

C_SOURCES = imageData.c

LDSCRIPT = ./Linker_sram.lds

USE_DAISYSP_LGPL=1
LDFLAGS += -u _printf_float
APP_TYPE = BOOT_SRAM

OPT = -Os    # Optimize for size замість -O2
CPPFLAGS += -ffunction-sections -fdata-sections  # Кожна функція в окрему секцію
LDFLAGS += -Wl,--gc-sections 

# Add OLED directory to include paths
C_INCLUDES += -I$(OLED_DIR)

# Library Locations
LIBDAISY_DIR = ../lib/libDaisy
DAISYSP_DIR = ../lib/DaisySP
OLED_DIR = OLED_Greyscale_Daisy
SX1509_DIR = SX1509_Daisy_Seed
FONTS_DIR = OLED_Greyscale_Daisy/Fonts

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
