
// #define INCLUDE_FVERB3

#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
//
#ifdef INCLUDE_FVERB3
#include "lib/fverb3.h"
#endif

bool button1Pressed = false;
#ifdef INCLUDE_FVERB3
FVerb3 fverb3;
#endif
uint8_t DMA_BUFFER_MEM_SECTION buffer_spi[4];
#define INCLUDE_AUDIO_PROFILING 1
#define RP2040_I2C_ADDRESS 0x28
#define AUDIO_BLOCK_SIZE 128
#define AUDIO_SAMPLE_RATE 48000
#define CROSSFADE_PREROLL 4800
#ifdef INCLUDE_FVERB3
#define MAX_SIZE (8388608)
#else
#define MAX_SIZE (8388608 * 2)
#endif
#define CYCLES_AVAILBLE \
  1066666  // (400000000 * AUDIO_BLOCK_SIZE / AUDIO_SAMPLE_RATE)
using namespace daisy;
using namespace daisysp;

DaisyPod hw;
DaisySeed daisyseed;
Metro metroPrintTimer;
Metro metroUpdateControls;

float DSY_SDRAM_BSS tape_linear_buffer[MAX_SIZE];

size_t audiocallback_sample_num = 0;
float cpu_needed = 0.0f;
float cpu_max_needed = 0.0f;
uint32_t audiocallback_time_needed = 0;
float cpu_usage_running[30] = {0};
size_t cpu_usage_index = 0;
float inl[AUDIO_BLOCK_SIZE];
float inr[AUDIO_BLOCK_SIZE];
float outl[AUDIO_BLOCK_SIZE];
float outr[AUDIO_BLOCK_SIZE];
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
#ifdef INCLUDE_AUDIO_PROFILING
  // measure - start
  DWT->CYCCNT = 0;
  audiocallback_sample_num = size / 2;
#endif

  // Deinterleave input samples
  for (size_t i = 0; i < size; i += 2) {
    inl[i / 2] = in[i];      // Left channel
    inr[i / 2] = in[i + 1];  // Right channel
  }

#ifdef INCLUDE_FVERB3
  fverb3.compute(AUDIO_BLOCK_SIZE, inl, inr, outl, outr);
#endif

  // Interleave output samples
  for (size_t i = 0; i < size; i += 2) {
    out[i] = outl[i / 2] / 2;      // Left channel output
    out[i + 1] = outl[i / 2] / 2;  // Duplicate left channel to right
  }

  if (metroPrintTimer.Process()) {
    // print hello

    daisyseed.PrintLine("cpu needed: %2.1f ", cpu_needed);
  } else if (metroUpdateControls.Process()) {
  }

#ifdef INCLUDE_AUDIO_PROFILING
  audiocallback_time_needed = DWT->CYCCNT;
  cpu_needed = (float)audiocallback_time_needed / CYCLES_AVAILBLE * 100.0f;
#endif
}

int main(void) {
  // hw.Init(false): 74.7% avg, 90.3% max
  // hw.Init(true): 79.9% avg, 96.6% max
  hw.Init(true);
  daisyseed.StartLog(false);

#ifdef INCLUDE_FVERB3
  fverb3.init(AUDIO_SAMPLE_RATE);
#endif

  // clear tape_linear_buffer
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));

#ifdef INCLUDE_AUDIO_PROFILING
  // setup measurement
  // https://forum.electro-smith.com/t/solved-how-to-do-mcu-utilization-measurements/1236
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->LAR = 0xC5ACCE55;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif

  metroPrintTimer.Init(1.0f, AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);
  metroUpdateControls.Init(30.0f, AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);

  hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
  hw.StartAudio(AudioCallback);
  hw.StartAdc();

  while (1) {
    hw.ProcessDigitalControls();
    // update controls
    if (hw.button1.RisingEdge() && !button1Pressed) {
      // print
      daisyseed.PrintLine("button1 pressed");
      button1Pressed = true;
      hw.led1.Set(0, 1, 0);
      hw.UpdateLeds();
    } else if (hw.button1.RisingEdge() && button1Pressed) {
      daisyseed.PrintLine("button1 released");
      button1Pressed = false;
      hw.led1.Set(0, 0, 0);
      hw.UpdateLeds();
    }
    System::Delay(6);
  }
}
