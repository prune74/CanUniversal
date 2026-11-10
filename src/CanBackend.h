// ============================================================================
//  CanBackend — interface commune de tous les moteurs CAN
//  CanUniversal v3.0.0
//
//  « La prise électrique » : n'importe quel moteur CAN qui implémente cette
//  interface peut être branché sur un CanBus. CanBus ne sait jamais ce
//  qu'il y a derrière (TWAI interne ESP32, MCP2515 externe...).
//
//  Contrat non bloquant :
//    - send()    : dépose la trame si possible TOUT DE SUITE, sinon false,
//                  sans jamais attendre.
//    - receive() : renvoie une trame s'il y en a une, false sinon,
//                  sans jamais attendre.
// ============================================================================

#pragma once
#include <stdint.h>
#include "CanMsg.h"
#include "CanConfig.h"

class CanBackend {

public:

    virtual ~CanBackend() {}

    /// Allume le moteur (vitesse, broches, viseur). false = échec propre
    /// (vitesse inconnue, broches occupées, puce muette...).
    virtual bool begin(const CanBusConfig &cfg) = 0;

    /// Envoie une trame SANS jamais attendre ni bloquer.
    virtual bool send(const CanMsg &msg) = 0;

    /// Prend une trame reçue, s'il y en a une. Ne bloque jamais.
    virtual bool receive(CanMsg &msg) = 0;

    /// Bus mort (fils débranchés, court-circuit) ?
    virtual bool isOff() = 0;

    /// Recette de secours après un bus mort.
    virtual void recover() = 0;
};