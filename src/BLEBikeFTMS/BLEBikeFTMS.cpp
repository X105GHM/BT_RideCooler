#include "BLEBikeFTMS.hpp"
#include <NimBLEDevice.h>
#include <Arduino.h>

namespace
{
    const NimBLEAdvertisedDevice *gTarget = nullptr;
    NimBLERemoteCharacteristic *gChar = nullptr;
    BLEBikeFTMS *gSelf = nullptr;
    bool gFtmsStarted = false;

    class ScanCb : public NimBLEScanCallbacks
    {
    public:
        void onResult(const NimBLEAdvertisedDevice *d) override
        {
            if (!gSelf)
                return;

            auto addr = d->getAddress().toString();
            std::string name = d->getName();

            Serial.printf("Found device: %s, name='%s'\n", addr.c_str(), name.empty() ? "<none>" : name.c_str());

            // FTMS im Advertising
            bool hasSvc = d->isAdvertisingService(NimBLEUUID(gSelf->serviceUuid()));
            if (hasSvc)
            {
                Serial.println("  -> advertises FTMS service!");
            }

            bool nameOk = (!gSelf->nameHint().empty() &&
                           !name.empty() &&
                           name.find(gSelf->nameHint()) != std::string::npos);

            if (nameOk /* || hasSvc */)
            {
                Serial.println("  -> Sieht nach unserem Fahrrad aus, Scan wird gestoppt.");
                gTarget = d;
                NimBLEDevice::getScan()->stop();
            }
        }
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
        if (!gSelf || len < 4)
            return;

        const uint16_t flags = data[0] | (uint16_t(data[1]) << 8);
        int idx = 2;

        // Debug:
        Serial.printf("IndoorBikeData notify, len=%d\n", (int)len);
        Serial.printf("  flags=0x%04X\n", flags);

        // Wichtig: Bit 0 == 0 -> Speed-Feld
        float speedKmh = NAN;
        if ((flags & 0x0001) == 0 && len >= idx + 2)
        {
            const uint16_t rawSpeed = data[idx] | (uint16_t(data[idx + 1]) << 8);
            speedKmh = rawSpeed / 100.0f; // Einheit: 0.01 km/h
            idx += 2;
            Serial.printf("  speed = %.1f km/h\n", speedKmh);
        }

        // Cadence
        float cadenceRpm = NAN;
        if ((flags & 0x0004) != 0 && len >= idx + 2)
        {
            const uint16_t rawCad = data[idx] | (uint16_t(data[idx + 1]) << 8);
            cadenceRpm = rawCad / 2.0f; // Einheit: 0.5 rpm
            idx += 2;
            Serial.printf("  cadence = %.1f rpm\n", cadenceRpm);
        }

        // Power
        float powerW = NAN;
        if ((flags & 0x0040) != 0 && len >= idx + 2)
        {
            const int16_t rawPow = int16_t(data[idx] | (uint16_t(data[idx + 1]) << 8));
            powerW = static_cast<float>(rawPow); // Einheit: Watt
            idx += 2;
            Serial.printf("  power = %.0f W\n", powerW);
        }

        if (!std::isnan(speedKmh))
        {
            gSelf->emitSpeed(speedKmh);
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
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

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
                Serial.println("BLE connected");
                gFtmsStarted = false; // nach jedem Connect neu

                // FTMS-Service (0x1826) holen
                auto *s = client_->getService(NimBLEUUID(serviceUuid_));
                if (!s)
                {
                    Serial.println("Service not found");
                    return;
                }

                // Indoor Bike Data (0x2AD2) holen
                gChar = s->getCharacteristic(NimBLEUUID(charUuid_));
                if (!gChar)
                {
                    Serial.println("Indoor Bike Data-Charakteristik nicht gefunden");
                    return;
                }
                if (!gChar->canNotify())
                {
                    Serial.println("Indoor Bike Data ist nicht benachrichtigungsfähig");
                    return;
                }

                if (!gChar->subscribe(true, notifyCB))
                {
                    Serial.println("Abonnieren der Indoor Bike Data fehlgeschlagen");
                }
                else
                {
                    Serial.println("Abonnieren der Indoor Bike Data erfolgreich");
                }

                // Control Point holen und Start/Resume schicken 
                NimBLERemoteCharacteristic *ctrl =
                    s->getCharacteristic(NimBLEUUID("00002AD9-0000-1000-8000-00805f9b34fb"));
                if (ctrl && ctrl->canWrite())
                {
                    Serial.println("FTMS-Steuerpunkt gefunden, fordere Kontrolle an und starte");

                    uint8_t reqCtrl[1] = {0x00}; // Request Control
                    ctrl->writeValue(reqCtrl, sizeof(reqCtrl), false);

                    uint8_t start[1] = {0x07}; // Start/Resume
                    ctrl->writeValue(start, sizeof(start), false);

                    gFtmsStarted = true;
                }
                else
                {
                    Serial.println("FTMS Control Point nicht gefunden oder nicht schreibbar");
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
