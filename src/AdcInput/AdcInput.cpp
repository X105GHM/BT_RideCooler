#include "AdcInput.hpp"
#include <Arduino.h>

AdcInput::AdcInput(int pin, int samples) : pin_{pin}, samples_{samples} {}

void AdcInput::begin()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

uint16_t AdcInput::readRaw()
{
    uint32_t sum = 0;
    for (int i = 0; i < samples_; ++i)
        sum += analogRead(pin_);
    return static_cast<uint16_t>(sum / samples_);
}

float AdcInput::read01()
{
    return readRaw() / 4095.0f;
}
