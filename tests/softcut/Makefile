# Compiler and flags
CXX = g++
CXXFLAGS = -I../../lib/softcut -O2 -Wall

# Source files
SOFTCUT_SRC = ../../lib/softcut/FadeCurves.cpp \
              ../../lib/softcut/ReadWriteHead.cpp \
              ../../lib/softcut/SubHead.cpp \
              ../../lib/softcut/Svf.cpp \
              ../../lib/softcut/Voice.cpp \
			  ../../lib/barcode/Barcode.cpp

# Object files
SOFTCUT_OBJ = $(SOFTCUT_SRC:.cpp=.o)

# Targets
all: build

# Convert FLAC to raw float
output.raw:
	sox fss_ecv2_138_vocal_phrase_wet_let_me_show_you_low_Cm_key_bpm200_138_beats16_.flac -r 48000 -c 2 -b 16 -e float  output.raw

# Compile Softcut sources into object files
../../lib/softcut/%.o: ../../lib/softcut/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main program
main: main.cpp $(SOFTCUT_OBJ)
	$(CXX) $(CXXFLAGS) -o main main.cpp $(SOFTCUT_OBJ)

# Build everything
build: output.raw main
	./main
	sox -r 48000 -c 2 -b 32 -e float  output.raw unprocessed.wav
	sox -r 48000 -c 2 -b 32 -e float  processed.raw processed.wav

# Play processed audio
play: build
	play processed.wav

# Clean up
clean:
	rm -f main output.raw processed.raw processed.wav $(SOFTCUT_OBJ)
