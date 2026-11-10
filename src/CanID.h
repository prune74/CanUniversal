// ============================================================================
//  CanID — construction et décodage des identifiants CAN
//  CanUniversal v3.0.0
//
//  Format 11 bits (standard) et 29 bits (étendu, convention Locoduino) :
//      [ prio : 3 bits ][ cmde : 8 bits ][ resp : 1 bit ][ nodeId : 16 bits ]
// ============================================================================

#pragma once
#include <stdint.h>

class CanID {

public:

    static constexpr uint16_t MAX_11BIT_ID = 0x7FF;   // 2047

    /// Construit un identifiant 29 bits à partir de ses champs.
    static uint32_t make29(uint8_t prio, uint8_t cmde,
                           uint8_t resp, uint16_t nodeId) {
        return ((uint32_t)(prio & 0x07) << 26)
             | ((uint32_t)(cmde & 0xFF) << 17)
             | ((uint32_t)(resp & 0x01) << 16)
             |  (nodeId & 0xFFFF);
    }

    /// L'identifiant tient-il sur 11 bits ? (bits hauts tous à zéro)
    static bool is11(uint32_t id) { return (id & 0xFFFFF800) == 0; }

    /// L'identifiant est-il 29 bits (étendu) ?
    static bool is29(uint32_t id) { return !is11(id); }

    // --- Découpage 29 bits (Locoduino) ---

    static uint8_t  prio(uint32_t id)    { return (id >> 26) & 0x07; }
    static uint8_t  cmde(uint32_t id)    { return (id >> 17) & 0xFF; }
    static uint8_t  resp(uint32_t id)    { return (id >> 16) & 0x01; }
    static uint16_t nodeId(uint32_t id)  { return id & 0xFFFF; }
};