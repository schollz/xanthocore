
#define INCLUDE_FVERB3

#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
//
#ifdef INCLUDE_FVERB3
#include "../../lib/fverb3.h"
#endif
//
#include "../../lib/App.h"
#include "../../lib/barcode/Barcode.h"
#include "../../lib/softcut/Utilities.h"
using namespace softcut;

App *app;
Barcode *barcode;
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
CpuLoadMeter loadMeter;
Metro metroPrintTimer;
LinearRamp knob1Slew;
LinearRamp knob2Slew;
float mainWet = 0.5;
float reverbWet = 0.5;
float reverbDry = 1.0 - reverbWet;
bool do_update_leds = false;

float DSY_SDRAM_BSS tape_linear_buffer[MAX_SIZE];

bool button1Pressed = false;
bool button2Pressed = false;
bool updateDigitalOrAnalog = false;
static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out, size_t size) {
#ifdef INCLUDE_AUDIO_PROFILING
  // measure - start
  // DWT->CYCCNT = 0;
  // audiocallback_sample_num = size / 2;
  loadMeter.OnBlockStart();
#endif

  // compute controls
  if (updateDigitalOrAnalog) {
    hw.ProcessDigitalControls();
    if (hw.button1.RisingEdge() && !button1Pressed) {
      button1Pressed = true;
      barcode->ToggleRecording(true);
    } else if (hw.button1.RisingEdge() && button1Pressed) {
      button1Pressed = false;
      barcode->ToggleRecording(false);
    }
    if (hw.button2.RisingEdge() && !button2Pressed) {
      button2Pressed = true;
      barcode->TogglePlaying(true);
    } else if (hw.button2.RisingEdge() && button2Pressed) {
      button2Pressed = false;
      barcode->TogglePlaying(false);
    }
  } else {
    hw.ProcessAnalogControls();
    knob1Slew.setTarget(hw.knob1.Process());
    knob2Slew.setTarget(hw.knob2.Process());
    app->setMainWet(knob1Slew.update());
    reverbWet = knob2Slew.update();
    fverb3.set_wet(reverbWet * 100);
    fverb3.set_dry((1.0 - reverbWet) * 100);
  }
  updateDigitalOrAnalog = !updateDigitalOrAnalog;

  app->Process(in, out, AUDIO_BLOCK_SIZE);

#ifdef INCLUDE_FVERB3
  fverb3.compute(out, AUDIO_BLOCK_SIZE);
#endif

#ifdef INCLUDE_AUDIO_PROFILING
  loadMeter.OnBlockEnd();
#endif
}

int main(void) {
  // hw.Init(false): 74.7% avg, 90.3% max
  // hw.Init(true): 79.9% avg, 96.6% max
  hw.Init(true);
  daisyseed.StartLog(false);

#ifdef INCLUDE_FVERB3
  fverb3.init(AUDIO_SAMPLE_RATE);
  fverb3.set_input_diffusion_2(80);
  fverb3.set_tail_density(80);
  fverb3.set_decay(75);
#endif

  // clear tape_linear_buffer
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));

  knob1Slew = LinearRamp(AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE / 2, 0.2f);
  knob2Slew = LinearRamp(AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE / 2, 0.2f);

  metroPrintTimer.Init(10.0f, 1000);

  // print starting
  daisyseed.PrintLine("Loading barcode...");
  app = new Barcode();
  barcode = static_cast<Barcode *>(app);
  app->Init(tape_linear_buffer, MAX_SIZE, AUDIO_SAMPLE_RATE, AUDIO_BLOCK_SIZE);
  app->registerCallback(
      static_cast<int>(Barcode::CallbackType::ON_RECORD_START),
      []() { daisyseed.PrintLine("Recording started"); });
  app->registerCallback(static_cast<int>(Barcode::CallbackType::ON_RECORD_STOP),
                        []() {
                          daisyseed.PrintLine("Recording stopped");
                          hw.led1.Set(0, 0, 0);
                          do_update_leds = true;
                        });
  app->registerCallback(
      static_cast<int>(Barcode::CallbackType::ON_RECORD_START), []() {
        daisyseed.PrintLine("Recording");
        hw.led1.Set(1, 0, 0);
        do_update_leds = true;
      });
  app->registerCallback(static_cast<int>(Barcode::CallbackType::ON_PLAY_START),
                        []() {
                          daisyseed.PrintLine("Playing started");
                          hw.led2.Set(0, 1, 0);
                          do_update_leds = true;
                        });
  app->registerCallback(static_cast<int>(Barcode::CallbackType::ON_PLAY_STOP),
                        []() {
                          daisyseed.PrintLine("Playing stopped");
                          hw.led2.Set(0, 0, 0);
                          do_update_leds = true;
                        });
  daisyseed.PrintLine("Barcode loaded");

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

  loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
  while (1) {
    if (metroPrintTimer.Process()) {
#ifdef INCLUDE_AUDIO_PROFILING
      // print it to the serial connection (as percentages)
      daisyseed.PrintLine("CPU Load " FLT_FMT3 "%% " FLT_FMT3 "%% " FLT_FMT3
                          "%% ",
                          FLT_VAR3(loadMeter.GetMinCpuLoad() * 100.0f),
                          FLT_VAR3(loadMeter.GetAvgCpuLoad() * 100.0f),
                          FLT_VAR3(loadMeter.GetMaxCpuLoad() * 100.0f));
#endif
    }

    if (do_update_leds) {
      hw.UpdateLeds();
      do_update_leds = false;
    }
    System::Delay(1);
  }
}
