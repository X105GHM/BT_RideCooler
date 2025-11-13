#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class DS18B20Bus
{
public:
    explicit DS18B20Bus(uint8_t pin);
    void begin();
    bool readTemperature(const DeviceAddress& addr, float& outC);

private:
    uint8_t         pin_;
    OneWire         ow_;
    DallasTemperature dt_;
    bool            started_ = false;
    SemaphoreHandle_t mtx_ = nullptr;
};

class DS18B20Sensor
{
public:
    DS18B20Sensor(uint8_t pin, const DeviceAddress& addr);
    void begin();
    float readTemperature();

private:
    DS18B20Bus&  bus_;
    DeviceAddress address_;
};
