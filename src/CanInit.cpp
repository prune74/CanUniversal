// ============================================================================
//  CanInit — initialisation tolérante aux erreurs
//  CanUniversal v3.0.0
//
//  Pour chaque bus de la configuration :
//    - pas de broche CS  → moteur TWAI (CAN interne de l'ESP32)
//    - broche CS définie → moteur MCP2515 (puce externe en SPI)
//  Le drapeau "loopback" active le loopback MATÉRIEL, géré par chaque
//  moteur (TWAI_MODE_NO_ACK côté ESP32, mode loopback côté MCP2515).
//  Le loopback logiciel reste disponible à la main via
//  CanBus::bus(i).enableSoftwareLoopback().
// ============================================================================

#include "CanInit.h"
#include "CanConfig.h"
#include "CanBus.h"
#include "CanBackend.h"
#include "TwaiBackend.h"
#include "Mcp2515Backend.h"

static const uint8_t MAX_BUSES = 8;

bool CanInit::begin(const CanConfigProvider &provider)
{
    uint8_t count = provider.busCount();
    if (count > MAX_BUSES) count = MAX_BUSES;

    for (uint8_t i = 0; i < count; i++)
    {
        const CanBusConfig &cfg = provider.bus(i);

        if (!cfg.enabled) {
            CanBus::attachInvalid(i);       // bus éteint dans la config
            continue;
        }

        // ---- Choisir le moteur selon la config ----
        CanBackend *moteur;
        if (cfg.cs_pin == GPIO_NUM_NC)
            moteur = new TwaiBackend(cfg.tx_pin, cfg.rx_pin);
        else
            moteur = new Mcp2515Backend(cfg);

        // ---- Essayer de le démarrer (tolérant aux erreurs) ----
        if (moteur->begin(cfg)) {
            CanBus::attach(i, moteur);       // moteur branché → bus valide
        } else {
            delete moteur;                   // moteur raté...
            CanBus::attachInvalid(i);        // ...bus invalide, mais sûr
        }
    }
    return true;   // toujours true : tolérant, comme toujours
}