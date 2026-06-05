#pragma once
#include "CanMsg.h"
#include <stdint.h>

/**
 * ============================================================================
 *  Classe CanBus
 * ============================================================================
 *  Abstraction d’un bus CAN matériel.
 *
 *  Cette classe encapsule un driver ACAN (ESP32 interne ou MCP2515 externe)
 *  derrière une API unifiée :
 *
 *      - send()    : envoi d’un message CAN
 *      - receive() : réception d’un message CAN
 *
 *  Le code applicatif ne manipule JAMAIS les drivers ACAN directement.
 *  Il utilise uniquement :
 *
 *      CanBus::bus(index)
 *
 *  Ce qui permet :
 *    - plusieurs bus CAN simultanés (CAN0, CAN1, CAN2…)
 *    - une architecture totalement générique
 *    - aucune dépendance matérielle dans le code applicatif
 * ============================================================================
 */
class CanBus {
public:

    /**
     * -------------------------------------------------------------------------
     *  Constructeur
     * -------------------------------------------------------------------------
     *  @param driverPtr : pointeur vers un driver ACAN
     *
     *  Ce pointeur peut être :
     *    - &ACAN_ESP32::can  → driver CAN interne ESP32
     *    - new ACAN2515(...) → instance d’un MCP2515 externe
     *
     *  Le pointeur est stocké tel quel, sans cast.
     *  Le cast est effectué uniquement dans send() / receive().
     * -------------------------------------------------------------------------
     */
    explicit CanBus(void *driverPtr);

    /**
     * Envoi d’un message CAN via le driver associé.
     * @param msg : message générique CanMsg
     * @return true si l’envoi a réussi
     */
    bool send(const CanMsg &msg);

    /**
     * Réception d’un message CAN via le driver associé.
     * @param msg : message générique CanMsg (rempli si réception OK)
     * @return true si un message a été reçu
     */
    bool receive(CanMsg &msg);

    // ---------------------------------------------------------------------
    // Accès global aux bus CAN
    // ---------------------------------------------------------------------

    /**
     * Enregistre un driver ACAN pour un bus donné.
     *
     * Appelé automatiquement par :
     *      CanInit::begin(provider)
     *
     * Exemple :
     *      CanBus::attach(0, &ACAN_ESP32::can);
     *      CanBus::attach(1, new ACAN2515(...));
     */
    static void attach(uint8_t index, void *driverPtr);

    /**
     * Retourne une référence vers le bus CAN demandé.
     *
     * Exemple d’utilisation :
     *      CanBus::bus(0).send(msg);
     *      CanBus::bus(1).receive(msg);
     *
     * @param index : numéro du bus (0, 1, 2…)
     */
    static CanBus& bus(uint8_t index);

private:

    /**
     * Pointeur vers le driver ACAN associé à ce bus.
     * Peut pointer vers :
     *    - ACAN_ESP32::can
     *    - une instance ACAN2515
     */
    void *driver;

    /**
     * Tableau statique contenant les pointeurs vers les bus CAN.
     *
     * Ce tableau est rempli par CanBus::attach() lors de l’initialisation.
     * Exemple :
     *      buses[0] = new CanBus(&ACAN_ESP32::can);
     *      buses[1] = new CanBus(new ACAN2515(...));
     *
     * Limite actuelle : 8 bus maximum (suffisant pour 99% des projets).
     */
    static CanBus* buses[8];
};
