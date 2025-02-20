#ifndef PCA9552_HPP
#define PCA9552_HPP

#include <daisy_seed.h>
#include <daisysp.h>

#include <array>
#include <cstdint>

class PCA9552 {
 public:
  enum Mode { HIGH = 0, LOW = 1, PWM0 = 2, PWM1 = 3 };

  enum ErrorCode {
    OK = 0x00,
    ERROR = 0xFF,
    ERR_WRITE = 0xFE,
    ERR_CHAN = 0xFD,
    ERR_MODE = 0xFC,
    ERR_REG = 0xFB,
    ERR_I2C = 0xFA
  };

  PCA9552() = delete;

  PCA9552(uint8_t deviceAddress, daisy::I2CHandle* i2cHandle,
          const std::array<uint8_t, 16>& rowMap,
          const std::array<uint8_t, 16>& colMap)
      : address(deviceAddress),
        i2c(i2cHandle),
        error(OK),
        row(rowMap),
        col(colMap),
        leds{},
        leds_last{} {
    initialize();
  }

  ErrorCode writeRegister(uint8_t reg, uint8_t value) {
    uint8_t txdata[2] = {reg, value};
    if (i2c->TransmitBlocking(address, txdata, 2, 100) ==
        daisy::I2CHandle::Result::OK) {
      return OK;
    }
    return ERROR;
  }

  uint8_t readRegister(uint8_t reg) {
    uint8_t rxdata = 0;
    if (i2c->TransmitBlocking(address, &reg, 1, 100) !=
        daisy::I2CHandle::Result::OK) {
      return ERROR;
    }
    if (i2c->ReceiveBlocking(address, &rxdata, 1, 100) !=
        daisy::I2CHandle::Result::OK) {
      return ERROR;
    }
    return rxdata;
  }

  void setPrescaler(uint8_t gen, uint8_t psc) {
    writeRegister(gen == 0 ? PCA9552_PSC0 : PCA9552_PSC1, psc);
  }

  uint8_t getPrescaler(uint8_t gen) {
    return readRegister(gen == 0 ? PCA9552_PSC0 : PCA9552_PSC1);
  }

  void setPWM(uint8_t gen, uint8_t pwm) {
    writeRegister(gen == 0 ? PCA9552_PWM0 : PCA9552_PWM1, pwm);
  }

  uint8_t getPWM(uint8_t gen) {
    return readRegister(gen == 0 ? PCA9552_PWM0 : PCA9552_PWM1);
  }

  ErrorCode setOutputMode(uint8_t pin, Mode mode) {
    if (mode > PWM1) return ERR_MODE;

    uint8_t reg = PCA9552_LS0;
    while (pin > 3) {
      reg++;
      pin -= 4;
    }

    uint8_t ledSelect = readRegister(reg);
    ledSelect &= ~(0x03 << (pin * 2));
    ledSelect |= (mode << (pin * 2));

    return writeRegister(reg, ledSelect);
  }

  void pinMode(uint8_t pin, uint8_t mode) {
    if (mode != 1) setOutputMode(pin, HIGH);
  }

  void digitalWrite(uint8_t pin, uint8_t val) {
    setOutputMode(pin, val == 0 ? LOW : HIGH);
  }

  void clear() {
    for (auto& row : leds) {
      for (auto& led : row) {
        led = 0;
      }
    }
  }

  void ledSet(uint8_t led, uint8_t state) { leds[row[led]][col[led]] = state; }

  void render() {
    for (size_t i = 0; i < 4; ++i) {
      bool changed = false;
      for (size_t j = 0; j < 4; ++j) {
        if (leds[i][j] != leds_last[i][j]) {
          changed = true;
          leds_last[i][j] = leds[i][j];
        }
      }
      if (changed) {
        writeRegister(PCA9552_LS0 + i, ledByte(leds[i]));
      }
    }
  }

 private:
  static constexpr uint8_t PCA9552_INPUT0 = 0x00;
  static constexpr uint8_t PCA9552_INPUT1 = 0x01;
  static constexpr uint8_t PCA9552_PSC0 = 0x02;
  static constexpr uint8_t PCA9552_PWM0 = 0x03;
  static constexpr uint8_t PCA9552_PSC1 = 0x04;
  static constexpr uint8_t PCA9552_PWM1 = 0x05;
  static constexpr uint8_t PCA9552_LS0 = 0x06;
  static constexpr uint8_t PCA9552_LS1 = 0x07;
  static constexpr uint8_t PCA9552_LS2 = 0x08;
  static constexpr uint8_t PCA9552_LS3 = 0x09;

  uint8_t address;
  daisy::I2CHandle* i2c;
  ErrorCode error;
  std::array<std::array<uint8_t, 4>, 4> leds;
  std::array<std::array<uint8_t, 4>, 4> leds_last;
  std::array<uint8_t, 16> row;
  std::array<uint8_t, 16> col;

  void initialize() {
    setPrescaler(0, 0);   // 44 Hz
    setPWM(0, 225);       // Dim PWM
    setPrescaler(1, 12);  // 1 Hz
    setPWM(1, 128);       // 50% duty cycle

    for (uint8_t i = 0; i < 4; i++) {
      writeRegister(PCA9552_LS0 + i, 0x55);
    }
  }

  static uint8_t ledByte(const std::array<uint8_t, 4>& ls) {
    uint8_t byte = 0x00;
    for (size_t i = 0; i < 4; i++) {
      uint8_t pos = (6 - 2 * i);
      switch (ls[i]) {
        case 0:
          byte |= (0b01 << pos);
          break;  // LED off
        case 1:
          byte |= (0b10 << pos);
          break;  // LED dim
        case 3:
          byte |= (0b11 << pos);
          break;  // LED bright blink
      }
    }
    return byte;
  }
};

#endif  // PCA9552_HPP
