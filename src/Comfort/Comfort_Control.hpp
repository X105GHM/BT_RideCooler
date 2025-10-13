#pragma once
#include <cmath>
#include <algorithm>

class ComfortControl
{
public:
    float computeDuty(float speedKmh, float alpha, float tRoomC, float tFanC) const;
};
