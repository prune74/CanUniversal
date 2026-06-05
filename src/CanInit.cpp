#include "CanInit.h"
#include "CanConfig.h"
#include "CanBus.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>
#include <SPI.h>

bool CanInit::begin(const CanConfigProvider& provider) {

    const uint8_t count = provider.busCount();
    Serial.printf("[CAN] Initialisation de %u bus...\n", count);

    for (uint8_t i = 0; i < count; i++) {

        const CanBusConfig& cfg = provider.bus(i);

        if (!cfg.enabled) {
            Serial.printf("[CAN%u] Désactivé\n", i);
            continue;
        }

        // ---------------------------------------------------------------------
        // CAN interne ESP32 (TWAI)
        // ---------------------------------------------------------------------
        if (cfg.cs_pin == GPIO_NUM_NC) {

            ACAN_ESP32_Settings settings(cfg.speed);
            settings.mTxPin = cfg.tx_pin;
            settings.mRxPin = cfg.rx_pin;

            uint32_t err = ACAN_ESP32::can.begin(settings);

            if (err != 0) {
                Serial.printf("[CAN%u] Erreur ACAN_ESP32 : 0x%X\n", i, err);
            } else {
                Serial.printf("[CAN%u] OK (ESP32 interne)\n", i);
            }

            CanBus::attach(i, &ACAN_ESP32::can);
        }

        // ---------------------------------------------------------------------
        // CAN externe MCP2515
        // ---------------------------------------------------------------------
        else {

            static ACAN2515* drivers[8] = { nullptr };

            drivers[i] = new ACAN2515(cfg.cs_pin, SPI, cfg.int_pin);

            ACAN2515Settings settings(cfg.quartz, cfg.speed, cfg.tolerance);

            uint32_t err = drivers[i]->begin(settings, [](){});

            if (err != 0) {
                Serial.printf("[CAN%u] Erreur MCP2515 : 0x%X\n", i, err);
            } else {
                Serial.printf("[CAN%u] OK (MCP2515 externe)\n", i);
            }

            CanBus::attach(i, drivers[i]);
        }
    }

    return true;
}
