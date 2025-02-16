
#define INCLUDE_FVERB3

#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
//
#ifdef INCLUDE_FVERB3
#include "lib/fverb3.h"
#endif
//
#include "lib/barcode/Barcode.h"
using namespace softcut;

Barcode barcode;
bool button1Pressed = false;
#ifdef INCLUDE_FVERB3
FVerb3 fverb3;
#endif
uint8_t DMA_BUFFER_MEM_SECTION buffer_spi[4];
#define INCLUDE_AUDIO_PROFILING 1
#define RP2040_I2C_ADDRESS 0x28
#define AUDIO_BLOCK_SIZE 128
#define AUDIO_SAMPLE_RATE 32000  // 32000 or 48000
#define CROSSFADE_PREROLL 4800
double CYCLES_AVAILBLE =
    ((400000000.0d * (double)AUDIO_BLOCK_SIZE) / (double)AUDIO_SAMPLE_RATE);
#ifdef INCLUDE_FVERB3
#define MAX_SIZE (2 << 22)
#else
#define MAX_SIZE (2 << 22)
#endif
using namespace daisy;
using namespace daisysp;

DaisyPod hw;
DaisySeed daisyseed;
Metro metroPrintTimer;
Metro metroUpdateControls;
float reverbWet = 0.5;
float reverbDry = 1.0 - reverbWet;

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
    outl[i / 2] = 0;
    outr[i / 2] = 0;
  }

  barcode.process(inl, inr, outl, outr, AUDIO_BLOCK_SIZE);
  for (size_t i = 0; i < AUDIO_BLOCK_SIZE; i++) {
    inl[i] = outl[i];
    inr[i] = outr[i];
  }

#ifdef INCLUDE_FVERB3
  fverb3.compute(AUDIO_BLOCK_SIZE, inl, inr, outl, outr);

  // // Downsample to 24000 sample rate
  // float downsampled_inl[AUDIO_BLOCK_SIZE / 2];
  // float downsampled_inr[AUDIO_BLOCK_SIZE / 2];
  // for (size_t i = 0; i < AUDIO_BLOCK_SIZE / 2; i++) {
  //   downsampled_inl[i] = inl[i * 2];
  //   downsampled_inr[i] = inr[i * 2];
  // }

  // // Process with FVerb3
  // float downsampled_outl[AUDIO_BLOCK_SIZE / 2];
  // float downsampled_outr[AUDIO_BLOCK_SIZE / 2];
  // fverb3.compute(AUDIO_BLOCK_SIZE / 2, downsampled_inl, downsampled_inr,
  //                downsampled_outl, downsampled_outr);

  // // Upsample back to 48000 sample rate
  // for (size_t i = 0; i < AUDIO_BLOCK_SIZE / 2; i++) {
  //   outl[i * 2] = downsampled_outl[i];
  //   outl[i * 2 + 1] = downsampled_outl[i];
  //   outr[i * 2] = downsampled_outr[i];
  //   outr[i * 2 + 1] = downsampled_outr[i];
  // }

#endif

  // for (unsigned int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
  //   float outl_, outr_;
  //   reverb.Process(outl[i], outr[i], &outl_, &outr_);
  //   outl[i] = reverbWet * outl_ + reverbDry * outl[i];
  //   outr[i] = reverbWet * outr_ + reverbDry * outr[i];
  // }

  // Interleave output samples
  for (size_t i = 0; i < size; i += 2) {
    out[i] = outl[i / 2];      // Left channel
    out[i + 1] = outr[i / 2];  // Right channel
  }

  if (metroPrintTimer.Process()) {
    // print hello

    daisyseed.PrintLine("cpu needed: %2.1f %2.1f", cpu_needed,
                        barcode.VoicePosition(0));
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
  fverb3.init(AUDIO_SAMPLE_RATE / 2);
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

  metroPrintTimer.Init(10.0f, AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);
  metroUpdateControls.Init(30.0f, AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);

  System::Delay(3000);
  // print starting
  daisyseed.PrintLine("Loading barcode...");
  barcode.init(tape_linear_buffer, MAX_SIZE, AUDIO_SAMPLE_RATE,
               AUDIO_BLOCK_SIZE);

  daisyseed.PrintLine("Starting audio...");
  hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
  if (AUDIO_SAMPLE_RATE == 32000) {
    SaiHandle::Config sai_config;
    sai_config.sr = SaiHandle::Config::SampleRate::SAI_32KHZ;
    hw.SetAudioSampleRate(sai_config.sr);
  }
  hw.StartAudio(AudioCallback);
  hw.StartAdc();
  daisyseed.PrintLine("Audio started");
  while (1) {
    hw.ProcessDigitalControls();
    // update controls
    if (hw.button1.RisingEdge() && !button1Pressed) {
      // print
      daisyseed.PrintLine("button1 pressed");
      button1Pressed = true;
      hw.led1.Set(1, 0, 0);
      barcode.ToggleRecording(true);
    } else if (hw.button1.RisingEdge() && button1Pressed) {
      daisyseed.PrintLine("button1 released");
      button1Pressed = false;
      hw.led1.Set(0, 0, 0);
      barcode.ToggleRecording(false);
    }
    if (barcode.Barcoding()) {
      hw.led2.Set(0, 1, 0);
    } else if (barcode.Recording()) {
      hw.led2.Set(1, 0, 0);
    } else {
      hw.led2.Set(0, 0, 0);
    }
    hw.UpdateLeds();
    System::Delay(6);
  }
}
