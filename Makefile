# Project Name
TARGET = SynthV10

# Sources
CPP_SOURCES = SynthV10.cpp Parameters.cpp MidiHandler.cpp Voice.cpp SX1509_extender.cpp \
	$(OLED_DIR)/DEV_Config.cpp $(OLED_DIR)/OLED_Driver.cpp $(OLED_DIR)/GUI_Paint.cpp $(OLED_DIR)/font8.cpp \
	$(OLED_DIR)/font12.cpp $(OLED_DIR)/font16.cpp $(OLED_DIR)/font20.cpp $(OLED_DIR)/font24.cpp $(SX1509_DIR)/SX1509.cpp


C_SOURCES = $(OLED_DIR)/ImageData.c

USE_DAISYSP_LGPL=1
LDFLAGS += -u _printf_float
APP_TYPE = BOOT_NONE

# Add OLED directory to include paths
C_INCLUDES += -I$(OLED_DIR)

# Library Locations
LIBDAISY_DIR = ../DaisyExamples/libDaisy
DAISYSP_DIR = ../DaisyExamples/DaisySP
OLED_DIR = OLED_1.5_Daisy_Seed
SX1509_DIR = SX1509_Daisy_Seed

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
