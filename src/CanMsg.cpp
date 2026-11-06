#include "CanMsg.h"

/**
 * ============================================================================
 *  Constructeurs
 * ============================================================================
 */

// Message vide
CanMsg::CanMsg()
    : id(0), dlc(0) {}

// ID 11 bits + payload
CanMsg::CanMsg(uint16_t id11, std::initializer_list<uint8_t> payload)
    : id(id11), dlc(payload.size()) {

    // Garde-fou : une trame CAN porte au maximum 8 octets
    if (dlc > 8) dlc = 8;

    int i = 0;
    for (uint8_t b : payload) {
        if (i >= 8) break;
        data[i++] = b;
    }
}

// ID 29 bits + payload
CanMsg::CanMsg(uint32_t id29, std::initializer_list<uint8_t> payload)
    : id(id29), dlc(payload.size()) {

    // Garde-fou : une trame CAN porte au maximum 8 octets
    if (dlc > 8) dlc = 8;

    int i = 0;
    for (uint8_t b : payload) {
        if (i >= 8) break;
        data[i++] = b;
    }
}


/**
 * ============================================================================
 *  Conversion ACAN → CanMsg
 * ============================================================================
 *  Permet de recevoir une trame ACAN (CANMessage) et de la convertir
 *  en message universel CanMsg.
 */
CanMsg::CanMsg(const CANMessage &frame)
    : id(frame.id), dlc(frame.len) {

    if (dlc > 8) dlc = 8;   // par sécurité

    for (uint8_t i = 0; i < dlc; i++) {
        data[i] = frame.data[i];
    }
}


/**
 * ============================================================================
 *  Conversion CanMsg → ACAN
 * ============================================================================
 *  Permet d'envoyer un message universel CanMsg via ACAN.
 */
CANMessage CanMsg::toFrame() const {
    CANMessage frame;

    frame.id  = id;
    frame.ext = CanID::is29(id);  // true si ID 29 bits
    frame.len = dlc;

    for (uint8_t i = 0; i < dlc; i++) {
        frame.data[i] = data[i];
    }

    return frame;
}