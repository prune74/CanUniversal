// ============================================================================
//  TwaiBackend — moteur du CAN interne de l'ESP32 (TWAI)
//  CanUniversal v3.0.0 — sans dépendance externe
// ============================================================================

#pragma once
#include "CanBackend.h"
#include "CanConfig.h"
#include "driver/twai.h"

class TwaiBackend : public CanBackend
{
public:
    TwaiBackend(gpio_num_t txPin, gpio_num_t rxPin);

    bool begin(const CanBusConfig &cfg) override;
    bool send(const CanMsg &msg) override;
    bool receive(CanMsg &msg) override;
    bool isOff() override;
    void recover() override;

private:
    gpio_num_t txPin, rxPin;
    twai_general_config_t g;     // réglages gardés pour un éventuel redémarrage
    uint32_t savedSpeed = 0;
    bool loopbackMode = false;   // auto-test matériel activé ?
};