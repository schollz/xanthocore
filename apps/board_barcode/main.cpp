
#define AUDIO_SAMPLE_RATE 48000  // 32000 or 48000
#define AUDIO_BLOCK_SIZE 128

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
#include "../../lib/PCA9552.h"
#include "../../lib/barcode/Barcode.h"
#include "../../lib/reverb2/Reverb2.h"
#include "../../lib/softcut/Utilities.h"
// add namespaces
using namespace softcut;
using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

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
static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out, size_t size) {
#ifdef INCLUDE_AUDIO_PROFILING
  // measure - start
  // DWT->CYCCNT = 0;
  // audiocallback_sample_num = size / 2;
  loadMeter.OnBlockStart();
#endif

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

  reverb.Process(out, out, AUDIO_BLOCK_SIZE);
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
  daisyseed.StartLog(true);

  for (size_t i = 0; i < 2; i++) {
    dcblock[i].Init(hw.AudioSampleRate());
  }
#ifdef INCLUDE_FVERB3
  fverb3.init(AUDIO_SAMPLE_RATE);
  fverb3.set_input_diffusion_2(80);
  fverb3.set_tail_density(80);
  fverb3.set_decay(75);
#endif

  reverb.Init(hw.AudioSampleRate(), reverb_buffer);

  // clear tape_linear_buffer
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));

  knob1Slew = LinearRamp(AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE / 2, 0.2f);
  knob2Slew = LinearRamp(AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE / 2, 0.2f);

  // setup PCA9552
  // create a row map  0,0,0,0,1,1,1,1,2,2,2,3,3,3,3
  // create a col map  0,1,2,3,0,1,2,3,0,1,2,0,1,2,3
  const std::array<uint8_t, 16> row = {0, 0, 0, 0, 1, 1, 1, 1,
                                       2, 2, 2, 3, 3, 3, 3};
  const std::array<uint8_t, 16> col = {0, 1, 2, 3, 0, 1, 2, 3,
                                       0, 1, 2, 3, 0, 1, 2, 3};

  // turn off all GPIO
  GPIO d0_;
  d0_.Init(D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d1_;
  d1_.Init(D1, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d2_;
  d2_.Init(D2, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d3_;
  d3_.Init(D3, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d4_;
  d4_.Init(D4, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d5_;
  d5_.Init(D5, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d6_;
  d6_.Init(D6, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d7_;
  d7_.Init(D7, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d8_;
  d8_.Init(D8, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d9_;
  d9_.Init(D9, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d10_;
  d10_.Init(D10, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d11_;
  d11_.Init(D11, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d12_;
  d12_.Init(D12, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d13_;
  d13_.Init(D13, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d14_;
  d14_.Init(D14, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d15_;
  d15_.Init(D15, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d16_;
  d16_.Init(D16, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d17_;
  d17_.Init(D17, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d18_;
  d18_.Init(D18, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d19_;
  d19_.Init(D19, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d20_;
  d20_.Init(D20, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d21_;
  d21_.Init(D21, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d22_;
  d22_.Init(D22, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d23_;
  d23_.Init(D23, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d24_;
  d24_.Init(D24, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  GPIO d25_;
  d25_.Init(D25, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  // setup D11 and D12 for I2c

  // initialize i2c to communicate
  i2c = I2CHandle();
  I2CHandle::Config i2c_conf;
  i2c_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
  i2c_conf.speed = I2CHandle::Config::Speed::I2C_100KHZ;
  i2c_conf.mode = I2CHandle::Config::Mode::I2C_MASTER;
  // https://github.com/electro-smith/libDaisy/blob/master/src/daisy_seed.h#L209-L210
  i2c_conf.pin_config.scl = Pin(PORTB, 8);
  i2c_conf.pin_config.sda = Pin(PORTB, 9);
  i2c.Init(i2c_conf);
  // do an I2c scan
  System::Delay(1000);
  daisyseed.PrintLine("Scanning I2C bus...");
  for (uint8_t i = 0; i < 128; i++) {
    // send bytes to address
    uint8_t dummy = 0;
    if (i2c.TransmitBlocking(i, &dummy, 1, 100) == I2CHandle::Result::OK) {
      daisyseed.PrintLine("Found device at 0x%02X", i);
    }
    if (i2c.ReceiveBlocking(i, &dummy, 1, 100) == I2CHandle::Result::OK) {
      daisyseed.PrintLine("Found device at 0x%02X", i);
    }
  }
  daisyseed.PrintLine("Done scanning I2C bus");

  System::Delay(1000);
  daisyseed.PrintLine("Scanning I2C bus...");
  for (uint8_t i = 0; i < 128; i++) {
    // send bytes to address
    uint8_t dummy = 0;
    if (i2c.TransmitBlocking(i, &dummy, 1, 100) == I2CHandle::Result::OK) {
      daisyseed.PrintLine("Found device at 0x%02X", i);
    }
    if (i2c.ReceiveBlocking(i, &dummy, 1, 100) == I2CHandle::Result::OK) {
      daisyseed.PrintLine("Found device at 0x%02X", i);
    }
  }
  daisyseed.PrintLine("Done scanning I2C bus");

  PCA9552 pca9552 = PCA9552(0x60, &i2c, row, col);

  // light upt each led one at a time
  while (true) {
    for (size_t i = 0; i < 16; i++) {
      daisyseed.PrintLine("LED %d", i);
      pca9552.ledSet(i, 1);
      pca9552.render();
      System::Delay(500);
      pca9552.ledSet(i, 0);
      pca9552.render();
      System::Delay(500);
    }
  }
  // PCA9552(uint8_t deviceAddress, daisy::I2CHandle* i2cHandle,
  //   const std::array<uint8_t, 16>& rowMap,
  //   const std::array<uint8_t, 16>& colMap)
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
                          do_update_leds = true;
                        });
  app->registerCallback(
      static_cast<int>(Barcode::CallbackType::ON_RECORD_START), []() {
        daisyseed.PrintLine("Recording");
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
      do_update_leds = false;
    }
    System::Delay(10);
  }
}
