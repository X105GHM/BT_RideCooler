#include "BLEBikeFTMS.hpp"
#include <NimBLEDevice.h>
#include <Arduino.h>

namespace
{
    NimBLEAdvertisedDevice *gTarget = nullptr;
    NimBLERemoteCharacteristic *gChar = nullptr;
    BLEBikeFTMS *gSelf = nullptr;

    // Kompatible Scan-Callbacks (kein override -> weniger Versionsstress)
    class ScanCb : public NimBLEScanCallbacks
    {
    public:
        void onResult(NimBLEAdvertisedDevice *d)
        {
            if (!gSelf)
                return;
            const bool hasSvc = d->isAdvertisingService(NimBLEUUID(gSelf->serviceUuid()));
            const bool nameOk = (!gSelf->nameHint().empty() && d->getName().find(gSelf->nameHint()) != std::string::npos);
            if (hasSvc || nameOk)
            {
                gTarget = d;
                NimBLEDevice::getScan()->stop();
            }
        }
        // Manche NimBLE-Versionen haben zusätzlich onScanEnd(uint32_t) – nicht nötig hier.
    };

    class ClientCb : public NimBLEClientCallbacks
    {
    public:
        void onConnect(NimBLEClient *) { Serial.println("BLE connected"); }
        void onDisconnect(NimBLEClient *)
        {
            Serial.println("BLE disconnected");
            gChar = nullptr;
        }
    };

    // Notification: Indoor Bike Data (0x2AD2)
    void notifyCB(NimBLERemoteCharacteristic *, uint8_t *data, size_t len, bool)
    {
        if (!gSelf)
            return;
        if (len >= 4)
        {
            const uint16_t flags = data[0] | (uint16_t(data[1]) << 8);
            const bool speedPresent = (flags & 0x0001) != 0;
            int idx = 2;
            if (speedPresent && len >= idx + 2)
            {
                const uint16_t centiKmh = data[idx] | (uint16_t(data[idx + 1]) << 8);
                const float kmh = centiKmh / 100.0f;
                gSelf->emitSpeed(kmh);
            }
        }
    }
}

void BLEBikeFTMS::begin(const char *serviceUuid, const char *charUuid, const char *nameHint)
{
    serviceUuid_ = serviceUuid ? serviceUuid : "";
    charUuid_ = charUuid ? charUuid : "";
    nameHint_ = nameHint ? nameHint : "";

    gSelf = this;

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P7);

    client_ = NimBLEDevice::createClient();
    client_->setClientCallbacks(new ClientCb());

    auto *scan = NimBLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setInterval(45);
    scan->setWindow(15);
    scan->setScanCallbacks(new ScanCb());
    scan->start(5, false);
}

void BLEBikeFTMS::loop()
{
    if (!client_->isConnected())
    {
        if (gTarget)
        {
            Serial.printf("Verbinde: %s\n", gTarget->getAddress().toString().c_str());
            if (client_->connect(gTarget))
            {
                if (auto *s = client_->getService(NimBLEUUID(serviceUuid_)))
                {
                    gChar = s->getCharacteristic(NimBLEUUID(charUuid_));
                    if (gChar && gChar->canNotify())
                    {
                        gChar->subscribe(true, notifyCB);
                    }
                    else
                    {
                        Serial.println("Characteristic not notifiable");
                    }
                }
                else
                {
                    Serial.println("Service not found");
                }
            }
            else
            {
                Serial.println("Connect fehlgeschlagen, scanne weiter …");
                NimBLEDevice::getScan()->start(5, false);
                gTarget = nullptr;
            }
        }
        else if (!NimBLEDevice::getScan()->isScanning())
        {
            NimBLEDevice::getScan()->start(5, false);
        }
    }
}

void BLEBikeFTMS::onSpeed(SpeedCb cb)
{
    cb_ = std::move(cb);
}
