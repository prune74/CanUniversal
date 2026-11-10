// ============================================================================
//  Mcp2515Backend — moteur du CAN externe (MCP2515 via SPI)
//  CanUniversal v3.0.0
//
//  La puce fait le travail CAN toute seule ; l'ESP32 lui parle par 4 fils
//  SPI. Tout est en POLLING : le fil d'interruption (INT) n'est même pas
//  utilisé → rien ne vient déranger le signal DCC.
//  Indispensable pour les cartes à double bus : l'ESP32 n'a qu'UN seul
//  contrôleur TWAI interne, le second bus passe par un MCP2515.
// ============================================================================

#pragma once
#include <Arduino.h>
#include "CanBackend.h"
#include "CanConfig.h"
#include <SPI.h>

class Mcp2515Backend : public CanBackend
{
public:

    Mcp2515Backend(const CanBusConfig &cfg);

    bool begin(const CanBusConfig &cfg) override;
    bool send(const CanMsg &msg) override;
    bool receive(CanMsg &msg) override;
    bool isOff() override;
    void recover() override;

private:

    // Commandes SPI vers la puce
    void select()   { digitalWrite(cs, LOW);  }   // décrocher
    void deselect() { digitalWrite(cs, HIGH); }  // raccrocher
    uint8_t readReg(uint8_t reg);
    void    writeReg(uint8_t reg, uint8_t value);
    void    setBits(uint8_t reg, uint8_t mask);    // certains bits → 1
    void    clearBits(uint8_t reg, uint8_t mask);  // certains bits → 0
    bool    waitConfigMode();                      // attendre "mode réglages"

    int cs;
    gpio_num_t sckPin, mosiPin, misoPin;
    uint32_t quartz;
    CanBusConfig saved;    // pour redémarrer après un dépannage
    bool begun = false;
};