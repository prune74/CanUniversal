#include "CanBus.h"
#include "CanMsg.h"
#include "CanBackend.h"

static const uint8_t MAX_BUSES = 8;

CanBus* CanBus::buses[8] = { nullptr };

// Constructeur — moteur != nullptr -> bus valide
CanBus::CanBus(CanBackend *moteur)
    : moteur(moteur),
      valid(moteur != nullptr)
{
}

// Loopback logiciel — empiler une trame (côté émission)
// File SPSC : seul send() écrit lbIn, seul receive() écrit lbOut -> sans verrou.
void CanBus::lbPush(const CanMsg &msg) {
    const uint8_t next = (uint8_t)((lbIn + 1) % LB_SIZE);
    if (next == lbOut) return;   // file pleine : trame ignorée
    lbQueue[lbIn] = msg;
    lbIn = next;
}

// Loopback logiciel — dépiler une trame (côté réception)
bool CanBus::lbPop(CanMsg &msg) {
    if (lbIn == lbOut) return false;   // file vide
    msg = lbQueue[lbOut];
    lbOut = (uint8_t)((lbOut + 1) % LB_SIZE);
    return true;
}

void CanBus::attach(uint8_t index, CanBackend *moteur) {
    if (index >= MAX_BUSES) return;
    delete buses[index];
    buses[index] = new CanBus(moteur);
}

void CanBus::attachInvalid(uint8_t index) {
    if (index >= MAX_BUSES) return;
    delete buses[index];
    buses[index] = new CanBus(nullptr);
}

CanBus& CanBus::bus(uint8_t index) {
    static CanBus invalidBus(nullptr);
    if (index >= MAX_BUSES || buses[index] == nullptr || !buses[index]->valid) {
        return invalidBus;
    }
    return *buses[index];
}

// Envoi d'un message CAN
bool CanBus::send(const CanMsg &msg) {

    if (!valid || moteur == nullptr)
        return false;

    bool ok = moteur->send(msg);

    // Loopback logiciel : la trame émise devient visible côté réception
    if (lbEnabled)
        lbPush(msg);

    return ok;
}

// Réception d'un message CAN
bool CanBus::receive(CanMsg &msg) {

    if (!valid || moteur == nullptr)
        return false;

    if (moteur->receive(msg))
        return true;

    // rien côté moteur -> consulter la file loopback
    if (lbEnabled)
        return lbPop(msg);

    return false;
}