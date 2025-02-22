

// #define INCLUDE_FVERB3

#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisy_seed.h"
#include "daisysp.h"
//
#ifdef INCLUDE_FVERB3
#include "../../lib/fverb3.h"
#endif
//
#include "../../lib/App.h"
#include "../../lib/Config.h"
#include "../../lib/barcode/Barcode.h"
#include "../../lib/reverb2/Reverb2.h"
#include "../../lib/softcut/Utilities.h"
// add namespaces
using namespace softcut;
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

class Follower {
 public:
  Follower(float sampleRate)
      : a_(std::exp(-1.0 / (0.001 * sampleRate))),  // Attack time = 1ms
        b_(std::exp(-1.0 / (0.020 * sampleRate))),  // Decay time = 20ms
        y_(0) {}

  float process(float x) {
    const auto abs_x = std::abs(x);
    if (abs_x > y_) {
      y_ = a_ * y_ + (1 - a_) * abs_x;
    } else {
      y_ = b_ * y_ + (1 - b_) * abs_x;
    }
    return y_;
  }

 private:
  float a_, b_, y_;
};
Follower follower(CONFIG_AUDIO_SAMPLE_RATE);
App *app;
Barcode *barcode;
Reverb2 reverb;
I2CHandle i2c;
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
float DSY_SDRAM_BSS reverb_buffer[DSY_REVERB2_MAX_SIZE];

bool button1Pressed = false;
bool button2Pressed = false;
bool updateDigitalOrAnalog = false;
bool buttonOn[8] = {false};
GPIO leds[8];
GPIO buttons[3];
GPIO button;

static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out, size_t size) {
#ifdef INCLUDE_AUDIO_PROFILING
  // measure - start
  // DWT->CYCCNT = 0;
  // audiocallback_sample_num = size / 2;
  loadMeter.OnBlockStart();
#endif

  if (updateDigitalOrAnalog) {
    // process the potentiometers
    float val = daisyseed.adc.GetFloat(0);
    val = val - 0.088f / (0.969 - 0.088);
    if (val < 0.02) {
      val = 0.0;
    } else if (val > 0.98) {
      val = 1.0;
    }
    knob1Slew.setTarget(val);
    val = daisyseed.adc.GetFloat(1);
    if (val < 0.02) {
      val = 0.0;
    } else if (val > 0.98) {
      val = 1.0;
    }
    knob2Slew.setTarget(val);
    app->setMainWet(knob1Slew.update());
    reverbWet = knob2Slew.update();
    reverb.SetWet(reverbWet);
  }
  updateDigitalOrAnalog = !updateDigitalOrAnalog;

  // clear out
  for (size_t i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
    out[0][i] = 0;
    out[1][i] = 0;
  }

  app->Process(in, out, CONFIG_AUDIO_BLOCK_SIZE);

  reverb.Process(out, out, CONFIG_AUDIO_BLOCK_SIZE);

  // add tanf to the output
  for (size_t i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
    float follow = follower.process(out[0][0]);
    out[0][i] = tanhf(out[0][i] + (follow / 2));
    out[1][i] = tanhf(out[1][i] + (follow / 2));
  }

  // DC block
  for (size_t i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
    out[0][i] = tanhf(dcblock[0].Process(out[0][i]));
    out[1][i] = tanhf(dcblock[1].Process(out[1][i]));
  }

#ifdef INCLUDE_FVERB3
  fverb3.compute(out, CONFIG_AUDIO_BLOCK_SIZE);
#endif

#ifdef INCLUDE_AUDIO_PROFILING
  loadMeter.OnBlockEnd();
#endif
}

int main(void) {
  // hw.Init(false): 74.7% avg, 90.3% max
  // hw.Init(true): 79.9% avg, 96.6% max
  hw.Init(true);
  daisyseed.StartLog(true);

  for (size_t i = 0; i < 2; i++) {
    dcblock[i].Init(hw.AudioSampleRate());
  }
#ifdef INCLUDE_FVERB3
  fverb3.init(CONFIG_AUDIO_SAMPLE_RATE);
  fverb3.set_input_diffusion_2(80);
  fverb3.set_tail_density(80);
  fverb3.set_decay(75);
#endif

  reverb.Init(hw.AudioSampleRate(), reverb_buffer);

  // clear tape_linear_buffer
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));

  knob1Slew =
      LinearRamp(CONFIG_AUDIO_SAMPLE_RATE / CONFIG_AUDIO_BLOCK_SIZE / 2, 0.2f);
  knob2Slew =
      LinearRamp(CONFIG_AUDIO_SAMPLE_RATE / CONFIG_AUDIO_BLOCK_SIZE / 2, 0.2f);

  // // turn off all GPIO
  // GPIO d0_;
  // d0_.Init(D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d1_;
  // d1_.Init(D1, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d2_;
  // d2_.Init(D2, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d3_;
  // d3_.Init(D3, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d4_;
  // d4_.Init(D4, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d5_;
  // d5_.Init(D5, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d6_;
  // d6_.Init(D6, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d7_;
  // d7_.Init(D7, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d8_;
  // d8_.Init(D8, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d9_;
  // d9_.Init(D9, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d10_;
  // d10_.Init(D10, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d11_;
  // d11_.Init(D11, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d12_;
  // d12_.Init(D12, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d13_;
  // d13_.Init(D13, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d14_;
  // d14_.Init(D14, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d15_;
  // d15_.Init(D15, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d16_;
  // d16_.Init(D16, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d17_;
  // d17_.Init(D17, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d18_;
  // d18_.Init(D18, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d19_;
  // d19_.Init(D19, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d20_;
  // d20_.Init(D20, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d21_;
  // d21_.Init(D21, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d22_;
  // d22_.Init(D22, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d23_;
  // d23_.Init(D23, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d24_;
  // d24_.Init(D24, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  // GPIO d25_;
  // d25_.Init(D25, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  // // setup D11 and D12 for I2c

  buttons[0].Init(D22, GPIO::Mode::OUTPUT);
  buttons[1].Init(D23, GPIO::Mode::OUTPUT);
  buttons[2].Init(D24, GPIO::Mode::OUTPUT);
  buttons[0].Write(0);
  buttons[1].Write(0);
  buttons[2].Write(1);
  button.Init(D25, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  leds[0].Init(D30, GPIO::Mode::OUTPUT);
  leds[1].Init(D0, GPIO::Mode::OUTPUT);
  leds[2].Init(D7, GPIO::Mode::OUTPUT);
  leds[3].Init(D8, GPIO::Mode::OUTPUT);
  leds[4].Init(D10, GPIO::Mode::OUTPUT);
  leds[5].Init(D13, GPIO::Mode::OUTPUT);
  leds[6].Init(D14, GPIO::Mode::OUTPUT);
  leds[7].Init(D9, GPIO::Mode::OUTPUT);
  // // create array of 3 buttons
  // bool buttonOn[8] = {false};
  // uint8_t x = 0;
  // while (true) {
  //   buttons[0].Write(x & 1);
  //   buttons[1].Write(x & 2);
  //   buttons[2].Write(x & 4);
  //   if (buttonOn[x] != button.Read()) {
  //     daisyseed.PrintLine("Button %d = %d", x, buttonOn[x]);
  //     buttonOn[x] = !buttonOn[x];
  //     leds[x].Write(!buttonOn[x]);
  //   }
  //   System::Delay(1);
  //   x++;
  //   if (x >= 8) {
  //     x = 0;
  //   }
  // }

  // Create an ADC Channel Config object
  AdcChannelConfig adc_config[3];
  adc_config[0].InitSingle(A0);
  adc_config[1].InitSingle(A1);
  adc_config[2].InitSingle(A2);
  daisyseed.adc.Init(adc_config, 3);
  daisyseed.adc.Start();

  // while (true) {
  //   daisyseed.PrintLine("%2.2f %2.2f %2.2f", daisyseed.adc.GetFloat(0),
  //                       daisyseed.adc.GetFloat(1),
  //                       daisyseed.adc.GetFloat(2));
  //   System::Delay(10);
  // }

  // PCA9552 pca9552 = PCA9552(0x60, &i2c, row, col);
  // // light upt each led one at a time
  // while (true) {
  //   for (size_t i = 0; i < 16; i++) {
  //     daisyseed.PrintLine("LED %d", i);
  //     pca9552.ledSet(i, 1);
  //     pca9552.render();
  //     System::Delay(500);
  //     pca9552.ledSet(i, 0);
  //     pca9552.render();
  //     System::Delay(500);
  //   }
  // }
  // PCA9552(uint8_t deviceAddress, daisy::I2CHandle* i2cHandle,
  //   const std::array<uint8_t, 16>& rowMap,
  //   const std::array<uint8_t, 16>& colMap)

  metroPrintTimer.Init(2.0f, 1000);

  // print starting
  daisyseed.PrintLine("Loading barcode...");
  app = new Barcode();
  barcode = static_cast<Barcode *>(app);
  app->Init(tape_linear_buffer, MAX_SIZE, CONFIG_AUDIO_SAMPLE_RATE,
            CONFIG_AUDIO_BLOCK_SIZE);
  app->registerCallback(
      static_cast<int>(Barcode::CallbackType::ON_RECORD_START),
      []() { daisyseed.PrintLine("Recording started"); });
  app->registerCallback(static_cast<int>(Barcode::CallbackType::ON_RECORD_STOP),
                        []() {
                          daisyseed.PrintLine("Recording stopped");
                          do_update_leds = true;
                          // turn off all leds
                          for (size_t i = 0; i < 8; i++) {
                            leds[i].Write(0);
                          }
                        });
  app->registerCallback(
      static_cast<int>(Barcode::CallbackType::ON_RECORD_START), []() {
        daisyseed.PrintLine("Recording");
        // turn on all leds
        for (size_t i = 0; i < 8; i++) {
          leds[i].Write(1);
        }
        do_update_leds = true;
      });
  app->registerCallback(static_cast<int>(Barcode::CallbackType::ON_PLAY_START),
                        []() {
                          daisyseed.PrintLine("Playing started");
                          do_update_leds = true;
                        });
  app->registerCallback(static_cast<int>(Barcode::CallbackType::ON_PLAY_STOP),
                        []() {
                          daisyseed.PrintLine("Playing stopped");
                          do_update_leds = true;
                        });
  daisyseed.PrintLine("Barcode loaded");

  daisyseed.PrintLine("Starting audio...");
  SaiHandle::Config sai_config;
  if (CONFIG_AUDIO_SAMPLE_RATE == 32000) {
    sai_config.sr = SaiHandle::Config::SampleRate::SAI_32KHZ;
    hw.SetAudioSampleRate(sai_config.sr);
  } else if (CONFIG_AUDIO_SAMPLE_RATE == 48000) {
    sai_config.sr = SaiHandle::Config::SampleRate::SAI_48KHZ;
    hw.SetAudioSampleRate(sai_config.sr);
  } else {
    // create an infinite loop
    while (1) {
      System::Delay(1000);
      daisyseed.PrintLine("Invalid sample rate");
    }
  }

  hw.SetAudioBlockSize(CONFIG_AUDIO_BLOCK_SIZE);
  hw.StartAudio(AudioCallback);
  daisyseed.PrintLine("Audio started");

  loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
  size_t x = 0;
  bool firstButtonSet = true;
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
      daisyseed.PrintLine("Position: %f %2.3f",
                          barcode->getVoices().getSavedPosition(0),
                          daisyseed.adc.GetFloat(0));
    }

    // check buttons
    buttons[0].Write(x & 1);
    buttons[1].Write(x & 2);
    buttons[2].Write(x & 4);
    // System::Delay(5);
    if (buttonOn[x] != button.Read()) {
      // daisyseed.PrintLine("Button %d = %d", x, buttonOn[x]);
      buttonOn[x] = !buttonOn[x];
      leds[x].Write(!buttonOn[x]);
      if (!firstButtonSet) {
        if (!buttonOn[x]) {
          daisyseed.PrintLine("Button %d = %d", x, buttonOn[x]);
          barcode->ToggleRecording();
        }
      }
    }
    x++;
    if (x >= 8) {
      x = 0;
      firstButtonSet = false;
    }
    System::Delay(1);
  }
}
