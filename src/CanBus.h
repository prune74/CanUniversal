// ============================================================================
//  CanBus — API unifiée multi-bus (jamais NULL)
//  CanUniversal v3.0.0
//
//  Chaque bus possède son moteur (TWAI interne ou MCP2515 externe).
//  CanBus::bus(index) renvoie TOUJOURS une référence valide : un bus
//  absent ou en erreur est simplement marqué invalide (isValid() == false)
//  et ses send()/receive() renvoient false — jamais de crash.
// ============================================================================

#pragma once
#include <stdint.h>
#include "CanMsg.h"
#include "CanBackend.h"

class CanBus {

public:

    /// Récupère le bus n° index (toujours une référence valide).
    static CanBus& bus(uint8_t index);

    /// Rattache un moteur démarré avec succès → bus valide.
    static void attach(uint8_t index, CanBackend *moteur);

    /// Marque le bus comme invalide (moteur absent ou raté) — mais sûr.
    static void attachInvalid(uint8_t index);

    /// Le bus est-il utilisable ?
    bool isValid() const { return valid; }

    /// Loopback logiciel optionnel (la trame émise est aussi rangée
    /// dans une file, visible côté receive()). Le loopback MATÉRIEL,
    /// lui, s'active via le drapeau "loopback" de la configuration.
    void enableSoftwareLoopback() { lbEnabled = true; }

    /// Envoi non bloquant (false = bus invalide ou case pleine).
    bool send(const CanMsg &msg);

    /// Réception non bloquante (false = rien à lire).
    bool receive(CanMsg &msg);

private:

    explicit CanBus(CanBackend *moteur);

    CanBackend *moteur;
    bool valid;

    // --- Loopback logiciel (file circulaire sans verrou) ---
    static const uint8_t LB_SIZE = 32;
    CanMsg lbQueue[LB_SIZE];
    volatile uint8_t lbIn = 0;
    volatile uint8_t lbOut = 0;
    bool lbEnabled = false;

    void lbPush(const CanMsg &msg);
    bool lbPop(CanMsg &msg);

    static CanBus* buses[8];
};