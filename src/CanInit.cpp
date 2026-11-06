#include "CanInit.h"
#include "CanConfig.h"
#include "CanBus.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>
#include <SPI.h>

/**
 * ============================================================================
 * 🚀 CanInit — Initialisation multi‑bus Chemins de Fer du Chablais (CFC)
 * ============================================================================
 * Rôle :
 *   - Initialiser chaque bus CAN défini dans le provider
 *   - Attacher un driver valide OU un bus invalide (mais sûr)
 *   - Garantir qu'aucun accès CAN ne provoque un crash
 *
 * Comportement :
 *   - Si un bus échoue → il est marqué invalide mais reste accessible
 *   - Si un bus réussit → driver opérationnel
 *
 * Résultat :
 *   → Architecture multi‑bus préservée
 *   → Aucun Guru Meditation même si MCP2515 absent
 * ============================================================================
 */

// Maximum de bus gérés par la bibliothèque
static const uint8_t MAX_BUSES = 8;

// -----------------------------------------------------------------------------
// 🟧 Drivers MCP2515 (un par bus externe)
// -----------------------------------------------------------------------------
static ACAN2515 *drivers[MAX_BUSES] = {nullptr};

// -----------------------------------------------------------------------------
// ISR statiques — les interruptions ESP32 n'acceptent pas de capture (lambda),
// il faut donc une fonction libre par bus qui appelle driver->isr().
// Sans cet appel, ACAN2515 ne reçoit JAMAIS de trame.
// -----------------------------------------------------------------------------
static void IRAM_ATTR isr0()
{
    if (drivers[0])
        drivers[0]->isr();
}
static void IRAM_ATTR isr1()
{
    if (drivers[1])
        drivers[1]->isr();
}
static void IRAM_ATTR isr2()
{
    if (drivers[2])
        drivers[2]->isr();
}
static void IRAM_ATTR isr3()
{
    if (drivers[3])
        drivers[3]->isr();
}
static void IRAM_ATTR isr4()
{
    if (drivers[4])
        drivers[4]->isr();
}
static void IRAM_ATTR isr5()
{
    if (drivers[5])
        drivers[5]->isr();
}
static void IRAM_ATTR isr6()
{
    if (drivers[6])
        drivers[6]->isr();
}
static void IRAM_ATTR isr7()
{
    if (drivers[7])
        drivers[7]->isr();
}

static void (*const isrs[MAX_BUSES])() = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7};

// SPI initialisé une seule fois (partagé entre plusieurs MCP2515)
static bool spiBegun = false;

// -----------------------------------------------------------------------------
// 🚀 CanInit::begin — initialise tous les bus décrits par le provider
// -----------------------------------------------------------------------------
// Tolérant aux erreurs : renvoie toujours true, mais chaque bus est marqué
// valide ou invalide. Vérifiez CanBus::bus(i).isValid() après l'appel.
// -----------------------------------------------------------------------------
bool CanInit::begin(const CanConfigProvider &provider)
{

    uint8_t count = provider.busCount();
    if (count > MAX_BUSES)
    {
        Serial.printf("[CAN] Attention : %u bus demandés, %u maximum supportés\n",
                      count, MAX_BUSES);
        count = MAX_BUSES;
    }

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

            // Loopback logiciel via mRequestedCANMode
            if (cfg.loopback)
            {
                settings.mRequestedCANMode = ACAN_ESP32_Settings::LoopBackMode;
            }
            else
            {
                settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode;
            }

            uint32_t err = ACAN_ESP32::can.begin(settings);

            if (err != 0)
            {
                Serial.printf("[CAN%u] Erreur ACAN_ESP32 : 0x%X → bus désactivé\n", i, err);
                CanBus::attachInvalid(i);
                continue;
            }

            Serial.printf("[CAN%u] OK (ESP32 interne)\n", i);
            CanBus::attach(i, &ACAN_ESP32::can);

            // 🟪 Loopback logiciel : trames relues par receive()
            if (cfg.loopback)
                CanBus::bus(i).enableSoftwareLoopback();

            continue;
        }

        // ---------------------------------------------------------------------
        // 🟧 CAN externe MCP2515
        // ---------------------------------------------------------------------

        // Initialiser le SPI une seule fois pour tous les MCP2515
        if (!spiBegun)
        {
            SPI.begin(cfg.sck_pin, cfg.miso_pin, cfg.mosi_pin, cfg.cs_pin);
            spiBegun = true;
        }

        drivers[i] = new ACAN2515(cfg.cs_pin, SPI, cfg.int_pin);

        ACAN2515Settings settings(cfg.quartz, cfg.speed, cfg.tolerance);

        // ISR dédiée à ce bus : sans elle, aucune réception MCP2515
        uint32_t err = drivers[i]->begin(settings, isrs[i]);

        if (err != 0)
        {
            Serial.printf("[CAN%u] Erreur MCP2515 : 0x%X → bus désactivé\n", i, err);
            delete drivers[i];
            drivers[i] = nullptr;
            CanBus::attachInvalid(i);
            continue;
        }

        Serial.printf("[CAN%u] OK (MCP2515 externe)\n", i);
        CanBus::attach(i, drivers[i]);

        // 🟪 Loopback logiciel également disponible sur MCP2515
        if (cfg.loopback)
            CanBus::bus(i).enableSoftwareLoopback();
    }

    return true;
}