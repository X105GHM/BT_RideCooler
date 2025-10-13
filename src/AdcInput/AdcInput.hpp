#pragma once
#include <cstdint>

class AdcInput
{
public:
    explicit AdcInput(int pin, int samples = 16);
    void begin();
    uint16_t readRaw(); // 0..4095
    float read01();     // 0..1
private:
    int pin_;
    int samples_;
};
