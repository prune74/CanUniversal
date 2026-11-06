#pragma once
#include <stdint.h>
#include "CanMsg.h"

/**
 * 🚌 CanBus — Abstraction multi-bus Chemins de Fer du Chablais (CFC)
 * Mode loopback : en plus du self-test du driver, un loopback logiciel
 * place chaque trame envoyée dans une file interne relue par receive().
 */
class CanBus
{
public:
    static CanBus &bus(uint8_t index);
    static void attach(uint8_t index, void *driverPtr);
    static void attachInvalid(uint8_t index);

    bool isValid() const { return valid; }

    // 🟪 Activer le loopback logiciel (appelé par CanInit si cfg.loopback)
    void enableSoftwareLoopback() { lbEnabled = true; }

    bool send(const CanMsg &msg);
    bool receive(CanMsg &msg);

private:
    explicit CanBus(void *driverPtr);

    void *driver = nullptr;
    bool valid = true;

    // 🟪 Loopback logiciel : file SPSC (un producteur, un consommateur)
    static const uint8_t LB_SIZE = 32;
    CanMsg lbQueue[LB_SIZE];
    volatile uint8_t lbIn = 0;  // index d'écriture (send)
    volatile uint8_t lbOut = 0; // index de lecture (receive)
    bool lbEnabled = false;

    void lbPush(const CanMsg &msg);
    bool lbPop(CanMsg &msg);

    static CanBus *buses[8];
};