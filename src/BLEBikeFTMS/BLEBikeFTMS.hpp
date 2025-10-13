#pragma once
#include <functional>
#include <NimBLEClient.h>
#include <string>

class BLEBikeFTMS
{
public:
    using SpeedCb = std::function<void(float)>; // km/h

    void begin(const char* serviceUuid, const char* charUuid, const char* nameHint);
    void loop();
    void onSpeed(SpeedCb cb);
    const std::string& serviceUuid() const { return serviceUuid_; }
    const std::string& charUuid()   const { return charUuid_;   }
    const std::string& nameHint()   const { return nameHint_;   }
    void emitSpeed(float kmh) { if (cb_) cb_(kmh); }

private:
    std::string serviceUuid_;
    std::string charUuid_;
    std::string nameHint_;
    NimBLEClient* client_ = nullptr;
    SpeedCb cb_;
};
