// ============================================================================
//  CanMsg — trame CAN (11 ou 29 bits, 0 à 8 octets de données)
//  CanUniversal v3.0.0
//
//  La trame ne connaît pas le matériel : c'est le même objet pour le CAN
//  interne de l'ESP32 (TWAI) et pour un MCP2515 externe.
//  Le format 11 ou 29 bits se déduit de l'ID (voir CanID).
// ============================================================================

#pragma once
#include <stdint.h>
#include <initializer_list>
#include "CanID.h"

class CanMsg {

public:

    uint32_t id;       // Identifiant CAN (11 ou 29 bits)
    uint8_t  dlc;      // Nombre d'octets utiles (0..8)
    uint8_t  data[8];  // Charge utile

    CanMsg();

    /// Trame 11 bits — ex : CanMsg((uint16_t)0x123, { 0xAB, 0xCD });
    CanMsg(uint16_t id11, std::initializer_list<uint8_t> payload);

    /// Trame 29 bits — ex : CanMsg((uint32_t)0x00086010, { 0, 0x10, 0, 0 });
    /// ⚠ cast obligatoire quand l'ID est écrit en dur, sinon constructeur ambigu.
    CanMsg(uint32_t id29, std::initializer_list<uint8_t> payload);

    // --- Format (déduit de l'ID, comme CanID) ---

    bool is11() const { return CanID::is11(id); }
    bool is29() const { return CanID::is29(id); }

    // --- Découpage 29 bits (Locoduino) ---

    uint8_t  prio()   const { return CanID::prio(id); }
    uint8_t  cmde()   const { return CanID::cmde(id); }
    uint8_t  resp()   const { return CanID::resp(id); }
    uint16_t nodeId() const { return CanID::nodeId(id); }
};