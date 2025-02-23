# Targets
all: build
	./waf build -j8
	./build/main

build:
	./waf configure

# Clean up
clean:
	rm -rf build *wav *raw .waf* .lock-waf*