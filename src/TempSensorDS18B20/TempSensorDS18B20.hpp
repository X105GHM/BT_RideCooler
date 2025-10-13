#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>

class DS18B20Sensor
{
public:
    DS18B20Sensor(uint8_t pin, const DeviceAddress& addr);
    void begin();
    float readTemperature(); 

private:
    OneWire          ow;
    DallasTemperature dt;
    DeviceAddress    address;
};
