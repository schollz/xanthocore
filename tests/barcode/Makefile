# Targets
all: output.raw build
	sox -r 48000 -c 2 -b 32 -e float  output.raw unprocessed.wav
	./waf build -j8
	./build/main
	sox -r 48000 -c 2 -b 32 -e float  processed.raw processed.wav

build:
	./waf configure

# Convert FLAC to raw float
output.raw:
	# sox fss_ecv2_138_vocal_phrase_wet_let_me_show_you_low_Cm_key_bpm200_138_beats16_.flac 1.wav pad 0 30
	sox strega.mp3 -r 48000 1.wav 
	sox 1.wav -r 48000 -c 2 -b 16 -e float output.raw

# Play processed audio
play: all
	sox processed.wav out.wav norm 	
	play out.wav

# Clean up
clean:
	rm -rf build unprocessed.wav output.raw processed.raw processed.wav .waf* .lock* out.wav 1.wav