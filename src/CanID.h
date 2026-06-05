#pragma once
#include <stdint.h>

/**
 * ============================================================================
 *  Classe CanID
 * ============================================================================
 *  Représente la structure d'un identifiant CAN 29 bits utilisé dans :
 *    - Locoduino
 *    - LaBox
 *    - Discovery 2026
 *
 *  Format 29 bits Locoduino :
 *
 *      [ 3 bits ] [ 8 bits ] [ 1 bit ] [    16 bits    ]
 *        prio       cmde       resp        nodeId
 *
 *  Ce fichier fournit :
 *    - les SHIFT (position des champs dans l'ID)
 *    - les MASK (masques d'extraction)
 *    - un encodeur 29 bits (make29)
 *    - des décodeurs (prio, cmde, resp, nodeId)
 *    - la détection 11 bits / 29 bits
 *
 *  Objectif :
 *    Fournir une API simple, générique et indépendante du hardware.
 *    Aucun rôle n'est imposé : l'application décide de la signification
 *    des champs prio/cmde/resp/nodeId.
 * ============================================================================
 */
class CanID
{
public:
    // -------------------------------------------------------------------------
    // TAILLE DES CHAMPS (en bits)
    // -------------------------------------------------------------------------
    static constexpr uint8_t PRIO_BITS = 3;    // priorité sur 3 bits
    static constexpr uint8_t CMDE_BITS = 8;    // commande sur 8 bits
    static constexpr uint8_t RESP_BITS = 1;    // flag réponse sur 1 bit
    static constexpr uint8_t NODEID_BITS = 16; // nodeId sur 16 bits

    // -------------------------------------------------------------------------
    // MASQUES BRUTS (non shiftés)
    // -------------------------------------------------------------------------
    static constexpr uint32_t PRIO_MASK_RAW = (1u << PRIO_BITS) - 1u;     // 0x7
    static constexpr uint32_t CMDE_MASK_RAW = (1u << CMDE_BITS) - 1u;     // 0xFF
    static constexpr uint32_t RESP_MASK_RAW = (1u << RESP_BITS) - 1u;     // 0x1
    static constexpr uint32_t NODEID_MASK_RAW = (1u << NODEID_BITS) - 1u; // 0xFFFF

    // -------------------------------------------------------------------------
    // SHIFT = position des champs dans l'identifiant 29 bits
    // -------------------------------------------------------------------------
    static constexpr uint8_t SHIFT_PRIO = 25;
    static constexpr uint8_t SHIFT_CMDE = 17;
    static constexpr uint8_t SHIFT_RESP = 16;
    static constexpr uint8_t SHIFT_NODEID = 0;

    // -------------------------------------------------------------------------
    // MASQUES COMPLETS (shiftés à leur position)
    // -------------------------------------------------------------------------
    static constexpr uint32_t MASK_PRIO = PRIO_MASK_RAW << SHIFT_PRIO;
    static constexpr uint32_t MASK_CMDE = CMDE_MASK_RAW << SHIFT_CMDE;
    static constexpr uint32_t MASK_RESP = RESP_MASK_RAW << SHIFT_RESP;
    static constexpr uint32_t MASK_NODEID = NODEID_MASK_RAW << SHIFT_NODEID;

    // -------------------------------------------------------------------------
    // ID 11 bits (format standard)
    // -------------------------------------------------------------------------
    static constexpr uint32_t MAX_11BIT_ID = 0x7FF;

    // Détection du format
    static bool is11(uint32_t id) { return id <= MAX_11BIT_ID; }
    static bool is29(uint32_t id) { return id > MAX_11BIT_ID; }

    // -------------------------------------------------------------------------
    // Encodeur 29 bits
    // Construit un identifiant Locoduino à partir des champs individuels.
    //
    // Exemple :
    //   uint32_t id = CanID::make29(1, 0x20, 0, 0x1234);
    // -------------------------------------------------------------------------
    static uint32_t make29(uint8_t prio, uint8_t cmde, uint8_t resp, uint16_t nodeId)
    {
        return ((uint32_t)prio << SHIFT_PRIO) | ((uint32_t)cmde << SHIFT_CMDE) | ((uint32_t)resp << SHIFT_RESP) | ((uint32_t)nodeId << SHIFT_NODEID);
    }

    // -------------------------------------------------------------------------
    // Décodeurs 29 bits
    // Extraient les champs individuels d'un identifiant 29 bits.
    // -------------------------------------------------------------------------
    static uint8_t prio(uint32_t id) { return (id & MASK_PRIO) >> SHIFT_PRIO; }
    static uint8_t cmde(uint32_t id) { return (id & MASK_CMDE) >> SHIFT_CMDE; }
    static uint8_t resp(uint32_t id) { return (id & MASK_RESP) >> SHIFT_RESP; }
    static uint16_t nodeId(uint32_t id) { return (id & MASK_NODEID) >> SHIFT_NODEID; }
};
