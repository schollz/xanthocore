# Get the CPU cores
CORES := $(shell grep -c ^processor /proc/cpuinfo)

board_acrostic:
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
	/usr/bin/make -f ./apps/board_acrostic/Makefile -j$(CORES)
	/usr/bin/make -f ./apps/board_acrostic/Makefile program-dfu 


board_barcode:
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
	/usr/bin/make -f ./apps/board_barcode/Makefile -j$(CORES)
	/usr/bin/make -f ./apps/board_barcode/Makefile program-dfu 

	

lib/DaisySP/core:
	git submodule update --init --recursive
	cd lib/DaisySP && make -j$(CORES)
	cd lib/libDaisy && make -j$(CORES)

debug:
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
		exec arm-none-eabi-gdb -x openocd.gdb

clean:
	/usr/bin/make -f ./apps/pod_barcode/Makefile clean
	/usr/bin/make -f ./apps/board_barcode/Makefile clean
