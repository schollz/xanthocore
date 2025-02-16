# Get the CPU cores
CORES := $(shell grep -c ^processor /proc/cpuinfo)

building: lib/DaisySP/core
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
	/usr/bin/make -f ./Makefile.1 -j$(CORES)

# upload: building
# 	/usr/bin/make -f ./Makefile.1 program-dfu 

upload: building
	/usr/bin/make -f ./Makefile.1 program-dfu 

lib/DaisySP/core:
	git submodule update --init --recursive
	cd lib/DaisySP && make -j$(CORES)
	cd lib/libDaisy && make -j$(CORES)

debug:
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
		exec arm-none-eabi-gdb -x openocd.gdb

clean:
	/usr/bin/make -f ./Makefile.1 clean