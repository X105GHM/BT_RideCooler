#pragma once
#include <cstdint>
#include <algorithm>

class FanPwm
{
public:
    void begin(int pin, int channel, int timer, uint32_t freqHz, uint8_t bits);
    void setDuty(float duty01); // 0..1
private:
    int pin_ = -1;
    int channel_ = 0;
    int timer_ = 0;
    uint8_t bits_ = 8;
};
