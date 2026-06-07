#include "CanInit.h"
#include "CanConfig.h"
#include "CanBus.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>
#include <SPI.h>

/**
 * ============================================================================
 * 🚀 CanInit — Initialisation multi‑bus Discovery 2026
 * ============================================================================
 * Rôle :
 *   - Initialiser chaque bus CAN défini dans le provider
 *   - Attacher un driver valide OU un bus invalide (mais sûr)
 *   - Garantir qu’aucun accès CAN ne provoque un crash
 *
 * Comportement :
 *   - Si un bus échoue → il est marqué invalide mais reste accessible
 *   - Si un bus réussit → driver opérationnel
 *
 * Résultat :
 *   → Architecture 2‑bus préservée
 *   → Aucun Guru Meditation même si MCP2515 absent
 * ============================================================================
 */

bool CanInit::begin(const CanConfigProvider &provider)
{

    const uint8_t count = provider.busCount();
    Serial.printf("[CAN] Initialisation de %u bus...\n", count);

    for (uint8_t i = 0; i < count; i++)
    {

        const CanBusConfig &cfg = provider.bus(i);

        // ---------------------------------------------------------------------
        // 🟥 Bus désactivé dans la config
        // ---------------------------------------------------------------------
        if (!cfg.enabled)
        {
            Serial.printf("[CAN%u] Désactivé\n", i);
            CanBus::attachInvalid(i);
            continue;
        }

        // ---------------------------------------------------------------------
        // 🟦 CAN interne ESP32 (TWAI)
        // ---------------------------------------------------------------------
        if (cfg.cs_pin == GPIO_NUM_NC)
        {

            ACAN_ESP32_Settings settings(cfg.speed);
            settings.mTxPin = cfg.tx_pin;
            settings.mRxPin = cfg.rx_pin;
            
            // 🔥 Mode test sans matériel
            if (cfg.loopback)
                settings.mLoopBackMode = true;
            if (cfg.no_ack)
                settings.mNoAckMode = true;

            uint32_t err = ACAN_ESP32::can.begin(settings);

            if (err != 0)
            {
                Serial.printf("[CAN%u] Erreur ACAN_ESP32 : 0x%X → bus désactivé\n", i, err);
                CanBus::attachInvalid(i);
                continue;
            }

            Serial.printf("[CAN%u] OK (ESP32 interne)\n", i);
            CanBus::attach(i, &ACAN_ESP32::can);
            continue;
        }

        // ---------------------------------------------------------------------
        // 🟧 CAN externe MCP2515
        // ---------------------------------------------------------------------
        static ACAN2515 *drivers[8] = {nullptr};

        drivers[i] = new ACAN2515(cfg.cs_pin, SPI, cfg.int_pin);
        Serial.println(">>> DEBUG: SPI.begin() exécuté");

        SPI.begin(cfg.sck_pin, cfg.miso_pin, cfg.mosi_pin, cfg.cs_pin);

        ACAN2515Settings settings(cfg.quartz, cfg.speed, cfg.tolerance);

        uint32_t err = drivers[i]->begin(settings, []() {});

        if (err != 0)
        {
            Serial.printf("[CAN%u] Erreur MCP2515 : 0x%X → bus désactivé\n", i, err);
            CanBus::attachInvalid(i);
            continue;
        }

        Serial.printf("[CAN%u] OK (MCP2515 externe)\n", i);
        CanBus::attach(i, drivers[i]);
    }

    return true;
}
