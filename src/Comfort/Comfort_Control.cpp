#include "Comfort_Control.hpp"
#include "Config/Config.hpp"

static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

float ComfortControl::computeDuty(float speedKmh, float alpha, float tRoomC, float /*tFanC*/) const
{
    // Rule 1: If speed >= 38 km/h → full blast
    if (speedKmh >= cfg::kSpeedBlowKmh)
        return 1.0f;

    // Base mapping: avoid early full-scale.
    // duty_base = min(0.95, (alpha * v_bike) / V_ref), with V_ref = 45 km/h
    constexpr float VrefKmh = 45.0f;
    float duty = std::min(0.95f, (alpha * speedKmh) / VrefKmh);

    // Temperature comfort scaling: keep within comfort band
    float scale = 1.0f;
    if (!std::isnan(tRoomC))
    {
        const float lo = cfg::kComfortSetC - cfg::kComfortBandC;
        const float hi = cfg::kComfortSetC + cfg::kComfortBandC;
        if (tRoomC < lo)
        {
            // map [lo-4, lo] → scale [0.3,1.0]
            const float x = clampf((tRoomC - (lo - 4.0f)) / 4.0f, 0.0f, 1.0f);
            scale = 0.3f + 0.7f * x;
        }
        else if (tRoomC > hi)
        {
            // map [hi, hi+4] → scale [1.0,1.4]
            const float x = clampf((tRoomC - hi) / 4.0f, 0.0f, 1.0f);
            scale = 1.0f + 0.4f * x;
        }
    }
    duty *= scale;

    return clampf(duty, 0.0f, 1.0f);
}
