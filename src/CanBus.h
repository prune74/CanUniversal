#pragma once
#include <stdint.h>

class CanMsg;

/**
 * ============================================================================
 * 🚌 CanBus — Abstraction multi‑bus Discovery 2026
 * ============================================================================
 * Cette classe encapsule un driver CAN (ESP32 interne ou MCP2515 externe)
 * derrière une API unifiée :
 *
 *      • send()    → envoi d’un message CAN
 *      • receive() → réception d’un message CAN
 *
 * Objectifs :
 *   - gérer plusieurs bus CAN simultanés (CAN0, CAN1…)
 *   - masquer totalement ACAN_ESP32 et ACAN2515
 *   - permettre une architecture tolérante aux erreurs
 *   - éviter tout crash même si un bus est absent ou invalide
 *
 * Un bus peut être :
 *   - valide   → driver initialisé et opérationnel
 *   - invalide → driver absent, erreur d’init, ou matériel non câblé
 *
 * Dans tous les cas, l’accès via CanBus::bus(index) reste sûr.
 * ============================================================================
 */
class CanBus {
public:

    // -------------------------------------------------------------------------
    // 🟦 Accès sécurisé à un bus CAN
    // -------------------------------------------------------------------------
    // Retourne un bus valide ou un bus "factice" si le bus réel est invalide.
    static CanBus& bus(uint8_t index);

    // -------------------------------------------------------------------------
    // 🟩 Attacher un driver valide (appelé par CanInit)
    // -------------------------------------------------------------------------
    static void attach(uint8_t index, void *driverPtr);

    // -------------------------------------------------------------------------
    // 🟥 Attacher un bus invalide (driver absent ou init échouée)
    // -------------------------------------------------------------------------
    static void attachInvalid(uint8_t index);

    // -------------------------------------------------------------------------
    // 🟨 Vérifier si le bus est valide
    // -------------------------------------------------------------------------
    bool isValid() const { return valid; }

    // -------------------------------------------------------------------------
    // 🟦 API unifiée d’envoi / réception
    // -------------------------------------------------------------------------
    bool send(const CanMsg &msg);
    bool receive(CanMsg &msg);

private:

    // Constructeur interne
    explicit CanBus(void *driverPtr);

    // Pointeur vers driver ACAN (ESP32 ou MCP2515)
    void *driver = nullptr;

    // État du bus
    bool valid = true;

    // Tableau statique des bus
    static CanBus* buses[8];
};
