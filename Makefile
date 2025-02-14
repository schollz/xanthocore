
building:
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
	/usr/bin/make -f ./Makefile.1 -j32

upload:
	/usr/bin/make -f ./Makefile.1 program-dfu 
	
prereqs:
	git submodule update --init --recursive
	cd DaisySP && make -j32
	cd libDaisy && make -j32