#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace cfg {
  // ==== Pins ====
  constexpr int kPinFanPwm  = 25;  
  constexpr int kPinDsRoom  = 4;   
  constexpr int kPinDsFan   = 15;  
  constexpr int kPinPotAdc  = 35;  
  constexpr uint32_t kPwmFreqHz  = 30'000;
  constexpr uint8_t  kPwmBits    = 11;
  constexpr int      kPwmChannel = 0;
  constexpr int      kPwmTimer   = 0;

  constexpr int kAdcSamples = 16;

  constexpr float kAlphaMin = 0.6f;  
  constexpr float kAlphaMax = 1.2f;   

  constexpr float kComfortSetC  = 22.0f; 
  constexpr float kComfortBandC = 2.0f;  

  constexpr float kTempSoftC = 70.0f; 
  constexpr float kTempHardC = 85.0f; 

  constexpr float kFanVmaxKmh   = 18.6f; 
  constexpr float kVrefKmh      = 45.0f;
  constexpr float kDutyBaseCap  = 0.95f; 
  constexpr float kSpeedBlowKmh = 38.0f; 

  inline constexpr const char* kFtmsServiceUuid = "00001826-0000-1000-8000-00805f9b34fb";
  inline constexpr const char* kIndoorBikeChar  = "00002ad2-0000-1000-8000-00805f9b34fb";
  inline constexpr const char* kNameHint        = "Tacx"; // optional device name hint
  constexpr uint32_t kSpeedTimeoutMs = 5'000;
}
