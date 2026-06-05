#include "CanBus.h"
#include <ACAN_ESP32.h>
#include <ACAN2515.h>

/**
 * ============================================================================
 *  Implémentation de CanBus
 * ============================================================================
 *  Cette classe encapsule un driver ACAN (ESP32 interne ou MCP2515 externe)
 *  derrière une API unifiée :
 *
 *      - send()    : envoi d'un message CAN
 *      - receive() : réception d'un message CAN
 *
 *  Le code applicatif ne manipule JAMAIS la structure ACAN (CANMessage).
 *  Il utilise uniquement CanMsg, une abstraction indépendante du hardware.
 *
 *  Objectifs :
 *    - permettre plusieurs bus CAN simultanés (CAN[0], CAN[1], CAN[2]…)
 *    - ne rien imposer à l’application (rôle, vitesse, pins…)
 *    - masquer totalement les drivers ACAN_ESP32 et ACAN2515
 * ============================================================================
 */


/**
 * ---------------------------------------------------------------------------
 *  Constructeur
 * ---------------------------------------------------------------------------
 *  @param driverPtr : pointeur générique vers un driver ACAN
 *
 *  Ce pointeur peut être :
 *    - &ACAN_ESP32::can  → driver CAN interne ESP32
 *    - new ACAN2515(...) → instance d’un MCP2515 externe
 *
 *  On stocke ce pointeur tel quel, sans cast.
 *  Le cast n’est effectué qu’au moment de l’envoi/réception.
 * ---------------------------------------------------------------------------
 */
CanBus::CanBus(void *driverPtr)
    : driver(driverPtr)
{
    // Rien à faire ici : le driver sera utilisé tel quel.
}


/**
 * ---------------------------------------------------------------------------
 *  Envoi d'un message CAN
 * ---------------------------------------------------------------------------
 *  @param msg : message générique CanMsg
 *  @return true si l’envoi a réussi
 *
 *  Étapes :
 *    1. Conversion CanMsg → CANMessage (format ACAN)
 *    2. Détection du type de driver (ESP32 ou MCP2515)
 *    3. Appel de tryToSend() sur le bon driver
 * ---------------------------------------------------------------------------
 */
bool CanBus::send(const CanMsg &msg) {

    // Conversion générique CanMsg → CANMessage (ACAN)
    CANMessage frame = msg.toFrame();

    // -------------------------------------------------------------------------
    // Driver ESP32 interne (TWAI)
    // -------------------------------------------------------------------------
    if (driver == &ACAN_ESP32::can) {
        return ACAN_ESP32::can.tryToSend(frame);
    }

    // -------------------------------------------------------------------------
    // Driver MCP2515 externe
    // -------------------------------------------------------------------------
    ACAN2515 *d = static_cast<ACAN2515*>(driver);
    return d->tryToSend(frame);
}


/**
 * ---------------------------------------------------------------------------
 *  Réception d'un message CAN
 * ---------------------------------------------------------------------------
 *  @param msg : message générique CanMsg (rempli si réception OK)
 *  @return true si un message a été reçu
 *
 *  Étapes :
 *    1. Lecture d’un CANMessage depuis le driver
 *    2. Conversion CANMessage → CanMsg
 *    3. Retour du résultat
 * ---------------------------------------------------------------------------
 */
bool CanBus::receive(CanMsg &msg) {

    CANMessage frame;

    // -------------------------------------------------------------------------
    // Driver ESP32 interne
    // -------------------------------------------------------------------------
    if (driver == &ACAN_ESP32::can) {

        if (ACAN_ESP32::can.receive(frame)) {
            msg = CanMsg(frame);   // Conversion ACAN → générique
            return true;
        }

        return false;
    }

    // -------------------------------------------------------------------------
    // Driver MCP2515 externe
    // -------------------------------------------------------------------------
    ACAN2515 *d = static_cast<ACAN2515*>(driver);

    if (d->receive(frame)) {
        msg = CanMsg(frame);       // Conversion ACAN → générique
        return true;
    }

    return false;
}
