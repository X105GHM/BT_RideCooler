#include "TempSensorDS18B20.hpp"
#include <cstring>
#include <cmath>
#include <Arduino.h>

DS18B20Bus::DS18B20Bus(uint8_t pin) : pin_(pin), ow_(pin), dt_(&ow_)
{
    mtx_ = xSemaphoreCreateMutex();
}

void DS18B20Bus::begin()
{
    if (started_) 
    {
        return;
    }

    dt_.begin();
    dt_.setWaitForConversion(true);
    started_ = true;

    int count = dt_.getDeviceCount();
    Serial.printf("DS18B20Bus(pin=%u): found %d device(s)\n", pin_, count);
}

bool DS18B20Bus::readTemperature(const DeviceAddress &addr, float &outC)
{
    if (!started_) 
    {
        begin();
    }

    if (mtx_) 
    {
        xSemaphoreTake(mtx_, portMAX_DELAY);
    }

    bool okReq = dt_.requestTemperaturesByAddress(addr);
    if (!okReq) 
    {
        if (mtx_) xSemaphoreGive(mtx_);
        outC = NAN;
        return false;
    }

    const float t = dt_.getTempC(addr);

    if (mtx_) 
    {
        xSemaphoreGive(mtx_);
    }

    if (t == DEVICE_DISCONNECTED_C) 
    { // -127°C
        outC = NAN;
        return false;
    }

    outC = t;
    return true;
}

static DS18B20Bus &getBusForPin(uint8_t pin)
{
    static DS18B20Bus bus(pin);
    return bus;
}

DS18B20Sensor::DS18B20Sensor(uint8_t pin, const DeviceAddress &addr) : bus_(getBusForPin(pin))
{
    std::memcpy(address_, addr, sizeof(DeviceAddress));
}

void DS18B20Sensor::begin()
{
    bus_.begin();
}

static void printAddress(const DeviceAddress &addr)
{
    for (int i = 0; i < 8; ++i) 
    {
        if (addr[i] < 16) Serial.print("0");
        Serial.print(addr[i], HEX);
        if (i < 7) Serial.print(":");
    }
}

float DS18B20Sensor::readTemperature()
{
    float t = NAN;
    if (!bus_.readTemperature(address_, t))
    {
        Serial.print("DS18B20 read failed for addr=");
        printAddress(address_);
        Serial.println();
        return NAN;
    }
    return t;
}
