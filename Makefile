# Project Name
TARGET = LoopyTunes

# Sources
CPP_SOURCES = LoopyTunes.cpp DSP/Mixer.cpp Track.cpp DSP/FX/Delay.cpp DSP/FX/Waveshaper.cpp DSP/FX/PitchShift.cpp DSP/FX/Reverb/Reverb.cpp revmodel.cpp comb.cpp allpass.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy/
DAISYSP_DIR = ../../../DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Optimisation
OPT = -Os

# Boot management
LDSCRIPT ?= ./STM32H750IB_my_qspi.lds
USBPID = $(DAISY_PID)
FLASH_ADDRESS ?= $(QSPI_ADDRESS)
C_DEFS += -DBOOT_APP

# Debug
DEBUG = 1

# Flags
LDFLAGS += -u _printf_float

