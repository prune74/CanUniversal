#include "CanBus.h"
#include "CanMsg.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>

static const uint8_t MAX_BUSES = 8;

CanBus* CanBus::buses[8] = { nullptr };

// 🟩 Constructeur — driverPtr != nullptr → bus valide
CanBus::CanBus(void *driverPtr)
    : driver(driverPtr),
      valid(driverPtr != nullptr)
{
}

// 🟪 Loopback logiciel — empiler une trame (côté émission)
// File SPSC : seul send() écrit lbIn, seul receive() écrit lbOut → sans verrou.
void CanBus::lbPush(const CanMsg &msg) {
    const uint8_t next = (uint8_t)((lbIn + 1) % LB_SIZE);
    if (next == lbOut) return;   // file pleine : trame ignorée
    lbQueue[lbIn] = msg;
    lbIn = next;
}

// 🟪 Loopback logiciel — dépiler une trame (côté réception)
bool CanBus::lbPop(CanMsg &msg) {
    if (lbIn == lbOut) return false;   // file vide
    msg = lbQueue[lbOut];
    lbOut = (uint8_t)((lbOut + 1) % LB_SIZE);
    return true;
}

void CanBus::attach(uint8_t index, void *driverPtr) {
    if (index >= MAX_BUSES) return;
    delete buses[index];
    buses[index] = new CanBus(driverPtr);
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

// 🟩 Envoi d'un message CAN
bool CanBus::send(const CanMsg &msg) {

    if (!valid || driver == nullptr)
        return false;

    CANMessage frame = msg.toFrame();
    bool ok;

    if (driver == &ACAN_ESP32::can) {
        ok = ACAN_ESP32::can.tryToSend(frame);
    } else {
        ACAN2515 *d = static_cast<ACAN2515*>(driver);
        ok = d->tryToSend(frame);
    }

    // 🟪 Loopback logiciel : la trame émise devient visible côté réception
    if (lbEnabled)
        lbPush(msg);

    return ok;
}

// 🟩 Réception d'un message CAN
bool CanBus::receive(CanMsg &msg) {

    if (!valid || driver == nullptr)
        return false;

    CANMessage frame;

    if (driver == &ACAN_ESP32::can) {
        if (ACAN_ESP32::can.receive(frame)) {
            msg = CanMsg(frame);
            return true;
        }
        // 🟪 rien côté driver → consulter la file loopback
        if (lbEnabled)
            return lbPop(msg);
        return false;
    }

    ACAN2515 *d = static_cast<ACAN2515*>(driver);
    if (d->receive(frame)) {
        msg = CanMsg(frame);
        return true;
    }

    // 🟪 rien côté driver → consulter la file loopback
    if (lbEnabled)
        return lbPop(msg);

    return false;
}