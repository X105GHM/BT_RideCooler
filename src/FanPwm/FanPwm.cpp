#include "FanPwm.hpp"
#include <Arduino.h>

void FanPwm::begin(int pin, int channel, int timer, uint32_t freqHz, uint8_t bits)
{
    pin_ = pin;
    channel_ = channel;
    timer_ = timer;
    bits_ = bits;
    ledcSetup(channel_, freqHz, bits_);
    ledcAttachPin(pin_, channel_);
    setDuty(0.0f);
}

void FanPwm::setDuty(float duty01)
{
    duty01 = std::clamp(duty01, 0.0f, 1.0f);
    const uint32_t maxVal = (1u << bits_) - 1u;
    const uint32_t val = static_cast<uint32_t>(duty01 * maxVal + 0.5f);
    ledcWrite(channel_, val);
}
