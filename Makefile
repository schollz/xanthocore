# Get the CPU cores
CORES := $(shell grep -c ^processor /proc/cpuinfo)

building:
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
	/usr/bin/make -f ./Makefile.1 -j$(CORES)

upload:
	/usr/bin/make -f ./Makefile.1 program-dfu 

prereqs:
	git submodule update --init --recursive
	cd lib/DaisySP && make -j$(CORES)
	cd lib/libDaisy && make -j$(CORES)

clean:
	/usr/bin/make -f ./Makefile.1 clean