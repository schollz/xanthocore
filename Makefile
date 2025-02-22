# Get the CPU cores
CORES := $(shell grep -c ^processor /proc/cpuinfo)

pod_barcode: APP_NAME=pod_barcode
pod_barcode: build_app


board_barcode: APP_NAME=board_barcode
board_barcode: build_app


upload_board_barcode: APP_NAME=board_barcode UPLOAD=program-dfu
upload_board_barcode: build_app

upload_pod_barcode: APP_NAME=pod_barcode UPLOAD=program-dfu
upload_pod_barcode: build_app

build_app: lib/DaisySP/core
	LD_LIBRARY_PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/arm-none-eabi/lib:$$LD_LIBRARY_PATH \
	PATH=$(HOME)/bin/gcc-arm-none-eabi-10-2020-q4-major/bin:$$PATH \
	/usr/bin/make -f ./apps/$(APP_NAME)/Makefile -j$(CORES) $(UPLOAD)


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
