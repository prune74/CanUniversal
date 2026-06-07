#include "CanBus.h"
#include "CanMsg.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>

/**
 * ============================================================================
 * 🚌 CanBus — Discovery 2026
 * ============================================================================
 * Gestion sécurisée de plusieurs bus CAN simultanés.
 * Chaque bus peut être valide ou invalide, mais l’accès reste toujours sûr.
 * ============================================================================
 */

// -----------------------------------------------------------------------------
// 🟦 Tableau statique des bus
// -----------------------------------------------------------------------------
CanBus* CanBus::buses[8] = { nullptr };


// -----------------------------------------------------------------------------
// 🟩 Constructeur — driverPtr != nullptr → bus valide
// -----------------------------------------------------------------------------
CanBus::CanBus(void *driverPtr)
    : driver(driverPtr),
      valid(driverPtr != nullptr)   // 🔥 Correction essentielle
{
}


// -----------------------------------------------------------------------------
// 🟩 Attacher un driver valide
// -----------------------------------------------------------------------------
void CanBus::attach(uint8_t index, void *driverPtr) {
    buses[index] = new CanBus(driverPtr);
}


// -----------------------------------------------------------------------------
// 🟥 Attacher un bus invalide (driver absent ou init échouée)
// -----------------------------------------------------------------------------
void CanBus::attachInvalid(uint8_t index) {
    buses[index] = new CanBus(nullptr);   // valid = false automatiquement
}


// -----------------------------------------------------------------------------
// 🟦 Accès sécurisé à un bus CAN
// -----------------------------------------------------------------------------
CanBus& CanBus::bus(uint8_t index) {

    // Bus factice utilisé si le bus réel est invalide
    static CanBus invalidBus(nullptr);  // valid = false

    if (index >= 8 || buses[index] == nullptr || !buses[index]->valid) {
        return invalidBus;
    }

    return *buses[index];
}


// -----------------------------------------------------------------------------
// 🟩 Envoi d’un message CAN
// -----------------------------------------------------------------------------
bool CanBus::send(const CanMsg &msg) {

    if (!valid || driver == nullptr)
        return false;

    CANMessage frame = msg.toFrame();

    // Driver ESP32 interne
    if (driver == &ACAN_ESP32::can) {
        return ACAN_ESP32::can.tryToSend(frame);
    }

    // Driver MCP2515 externe
    ACAN2515 *d = static_cast<ACAN2515*>(driver);
    return d->tryToSend(frame);
}


// -----------------------------------------------------------------------------
// 🟩 Réception d’un message CAN
// -----------------------------------------------------------------------------
bool CanBus::receive(CanMsg &msg) {

    if (!valid || driver == nullptr)
        return false;

    CANMessage frame;

    // Driver ESP32 interne
    if (driver == &ACAN_ESP32::can) {
        if (ACAN_ESP32::can.receive(frame)) {
            msg = CanMsg(frame);
            return true;
        }
        return false;
    }

    // Driver MCP2515 externe
    ACAN2515 *d = static_cast<ACAN2515*>(driver);
    if (d->receive(frame)) {
        msg = CanMsg(frame);
        return true;
    }

    return false;
}
