# Targets
all: output.raw build
	./waf build -j8
	./build/main

build:
	./waf configure

# Convert FLAC to raw float
output.raw:
	sox amen.mp3 1.wav
	sox 1.wav -r 48000 -c 1 -b 16 -e float output.raw

# Play processed audio
plot: clean all
	./plot.py 

# Clean up
clean:
	rm -rf build *.raw .waf* .lock* out.wav 1.wav