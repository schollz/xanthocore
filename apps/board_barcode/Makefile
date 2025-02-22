TARGET = xanthocore

USE_DAISYSP_LGPL = 1

# APP_TYPE=BOOT_QSPI
APP_TYPE=BOOT_SRAM

CPP_SOURCES = apps/board_barcode/board_barcode.cpp \
	lib/Config.cpp \
	lib/Follower.cpp \
	lib/softcut/FadeCurves.cpp \
	lib/softcut/ReadWriteHead.cpp \
	lib/softcut/SubHead.cpp \
	lib/softcut/Svf.cpp \
	lib/reverb2/Reverb2.cpp \
	lib/dsp/Oscillator2.cpp \
	lib/barcode/Barcode.cpp \
	lib/softcut/Voices.cpp \
	lib/PCA9552.cpp \
	lib/softcut/Voice.cpp

LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP
USE_FATFS = 1
LDFLAGS += -u _printf_float

CXXFLAGS += $(INCLUDES)
# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile


