#include "TempSensorDS18B20.hpp"
#include <cstring>   
#include <cmath>     

DS18B20Sensor::DS18B20Sensor(uint8_t pin, const DeviceAddress& addr)
    : ow(pin), dt(&ow)
{
    std::memcpy(address, addr, sizeof(DeviceAddress));
}

void DS18B20Sensor::begin()
{
    dt.begin();
    dt.setWaitForConversion(true);
}

float DS18B20Sensor::readTemperature()
{
    if (!dt.requestTemperaturesByAddress(address)) {
        return NAN;
    }
    const float t = dt.getTempC(address);
    return (t == DEVICE_DISCONNECTED_C) ? NAN : t;
}
