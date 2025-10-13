#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "Config/Config.hpp"
#include "FanPwm/FanPwm.hpp"
#include "BLEBikeFTMS/BLEBikeFTMS.hpp"
#include "AdcInput/AdcInput.hpp"
#include "TempSensorDS18B20/TempSensorDS18B20.hpp"
#include "Comfort/Comfort_Control.hpp"

namespace
{
  FanPwm fan;
  BLEBikeFTMS bike;
  AdcInput pot{cfg::kPinPotAdc, cfg::kAdcSamples};

  // TODO: echte Adressen deiner Sensoren eintragen (8 Byte pro Sensor)
  DeviceAddress ROOM_ADDR = {0x28, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  DeviceAddress FAN_ADDR = {0x28, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01};


  DS18B20Sensor roomSensor(cfg::kPinDsRoom, ROOM_ADDR);
  DS18B20Sensor fanSensor(cfg::kPinDsFan, FAN_ADDR);

  SemaphoreHandle_t mtx;
  float speedKmh = 0.0f;
  uint32_t lastSpeedMs = 0;

  float clamp(float v, float lo, float hi) { return std::min(hi, std::max(lo, v)); }

  void taskBLE(void *)
  {
    bike.onSpeed([](float kmh)
                 {
      xSemaphoreTake(mtx, portMAX_DELAY);
      speedKmh   = kmh;
      lastSpeedMs = millis();
      xSemaphoreGive(mtx); });
    bike.begin(cfg::kFtmsServiceUuid, cfg::kIndoorBikeChar, cfg::kNameHint);
    for (;;)
    {
      bike.loop();
      vTaskDelay(pdMS_TO_TICKS(20));
    }
  }

  void taskControl(void *)
  {
    ComfortControl comfort{};
    for (;;)
    {
      // aktuelle Bike-Speed (mit Timeout auf 0)
      float v = 0.0f;
      uint32_t t = 0;
      xSemaphoreTake(mtx, portMAX_DELAY);
      v = speedKmh;
      t = lastSpeedMs;
      xSemaphoreGive(mtx);
      if (millis() - t > cfg::kSpeedTimeoutMs)
        v = 0.0f;

      // α vom Poti
      const float alpha = cfg::kAlphaMin + (cfg::kAlphaMax - cfg::kAlphaMin) * pot.read01();

      // Temperaturen (blockierend, simpel)
      const float tRoom = roomSensor.readTemperature();
      const float tFan = fanSensor.readTemperature();

      // PWM aus Komfort + Mapping
      float duty = comfort.computeDuty(v, alpha, tRoom, tFan);

      // Elektronik-Schutz
      if (!std::isnan(tFan))
      {
        if (tFan >= cfg::kTempHardC)
        {
          duty = 0.0f;
        }
        else if (tFan >= cfg::kTempSoftC)
        {
          const float k = (cfg::kTempHardC - tFan) / (cfg::kTempHardC - cfg::kTempSoftC); // 1..0
          duty *= clamp(k, 0.0f, 1.0f);
        }
      }

      fan.setDuty(duty);
      vTaskDelay(pdMS_TO_TICKS(20)); // 50 Hz Regel-Loop
    }
  }

  void taskTelemetry(void *)
  {
    for (;;)
    {
      float v = 0;
      uint32_t t = 0;
      xSemaphoreTake(mtx, portMAX_DELAY);
      v = speedKmh;
      t = lastSpeedMs;
      xSemaphoreGive(mtx);

      const float alpha = cfg::kAlphaMin + (cfg::kAlphaMax - cfg::kAlphaMin) * pot.read01();

      const float tRoom = roomSensor.readTemperature();
      const float tFan = fanSensor.readTemperature();

      Serial.printf("v=%.1f km/h (age %ums) alpha=%.2f  Troom=%.1f°C  Tfan=%.1f°C\n", v, millis() - t, alpha, tRoom, tFan);

      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(300);

  pot.begin();
  roomSensor.begin();
  fanSensor.begin();

  fan.begin(cfg::kPinFanPwm, cfg::kPwmChannel, cfg::kPwmTimer, cfg::kPwmFreqHz, cfg::kPwmBits);
  fan.setDuty(0.0f);

  mtx = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(taskBLE, "BLE", 8192, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskControl, "CTRL", 6144, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskTelemetry, "TEL", 4096, nullptr, 1, nullptr, 0);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
