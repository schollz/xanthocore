
#define AUDIO_SAMPLE_RATE 48000  // 32000 or 48000
#define AUDIO_BLOCK_SIZE 128

// #define INCLUDE_FVERB3

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

// add namespaces
using namespace softcut;
using namespace daisy;
using namespace daisysp;

App *app;
Barcode *barcode;
#ifdef INCLUDE_FVERB3
FVerb3 fverb3;
#endif
#define INCLUDE_AUDIO_PROFILING 1
#ifdef INCLUDE_FVERB3
#define MAX_SIZE (2 << 22)
#else
#define MAX_SIZE (2 << 22)
#endif

DaisyPod hw;
DaisySeed daisyseed;
CpuLoadMeter loadMeter;
Metro metroPrintTimer;
LinearRamp knob1Slew;
LinearRamp knob2Slew;
DcBlock dcblock[2];

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
    float val = hw.knob1.Process();
    if (val < 0.02) {
      val = 0.0;
    } else if (val > 0.98) {
      val = 1.0;
    }
    knob1Slew.setTarget(val);
    val = hw.knob2.Process();
    if (val < 0.02) {
      val = 0.0;
    } else if (val > 0.98) {
      val = 1.0;
    }
    knob2Slew.setTarget(val);
    app->setMainWet(knob1Slew.update());
    reverbWet = hw.knob2.Process();
#ifdef INCLUDE_FVERB3
    fverb3.set_wet(reverbWet * 100);
    fverb3.set_dry((1.0 - reverbWet) * 100);
#endif
  }
  updateDigitalOrAnalog = !updateDigitalOrAnalog;

  // clear out
  for (size_t i = 0; i < AUDIO_BLOCK_SIZE; i++) {
    out[0][i] = 0;
    out[1][i] = 0;
  }

  app->Process(in, out, AUDIO_BLOCK_SIZE);

  // DC block
  for (size_t i = 0; i < AUDIO_BLOCK_SIZE; i++) {
    out[0][i] = dcblock[0].Process(out[0][i]);
    out[1][i] = dcblock[1].Process(out[1][i]);
  }
  // // add tanf to the output
  // for (size_t i = 0; i < AUDIO_BLOCK_SIZE; i++) {
  //   out[0][i] = tanf(out[0][i]) / 2;
  //   out[1][i] = tanf(out[1][i]) / 2;
  // }

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

  for (size_t i = 0; i < 2; i++) {
    dcblock[i].Init(hw.AudioSampleRate());
  }
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
  SaiHandle::Config sai_config;
  if (AUDIO_SAMPLE_RATE == 32000) {
    sai_config.sr = SaiHandle::Config::SampleRate::SAI_32KHZ;
    hw.SetAudioSampleRate(sai_config.sr);
  } else if (AUDIO_SAMPLE_RATE == 48000) {
    sai_config.sr = SaiHandle::Config::SampleRate::SAI_48KHZ;
    hw.SetAudioSampleRate(sai_config.sr);
  } else {
    // create an infinite loop
    while (1) {
      System::Delay(1000);
      daisyseed.PrintLine("Invalid sample rate");
    }
  }
  hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
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
      // print position of voice 0
      daisyseed.PrintLine("Position: %f",
                          barcode->getVoices().getSavedPosition(0));
    }

    if (do_update_leds) {
      hw.UpdateLeds();
      do_update_leds = false;
    }
    System::Delay(10);
  }
}
