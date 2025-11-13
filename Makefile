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
	$(OLED_DIR)/OLED_Driver.cpp $(OLED_DIR)/GUI_Paint.cpp $(OLED_DIR)/font8.cpp \
	$(OLED_DIR)/font12.cpp $(OLED_DIR)/font16.cpp $(OLED_DIR)/font20.cpp $(OLED_DIR)/font24.cpp $(SX1509_DIR)/SX1509.cpp


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

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
