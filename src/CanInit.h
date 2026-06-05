#pragma once

#include "CanBus.h"
#include "CanConfig.h"

/**
 * ============================================================================
 *  CanInit
 * ============================================================================
 *  Classe responsable de l’initialisation des bus CAN.
 *
 *  La bibliothèque ne connaît :
 *    - ni les pins
 *    - ni les vitesses
 *    - ni le nombre de bus
 *    - ni le matériel utilisé (ESP32 interne ou MCP2515)
 *
 *  → Toutes ces informations sont fournies par l’application via
 *    un CanConfigProvider.
 *
 *  Objectifs :
 *    - garder un setup() propre
 *    - rendre la bibliothèque totalement générique
 *    - permettre un nombre illimité de bus CAN
 *    - ne rien imposer à l’application (noms, rôles, vitesses…)
 * ============================================================================
 */
class CanInit {
public:

    /**
     * Initialise tous les bus CAN décrits par le provider.
     *
     * @param provider : objet fourni par l’application contenant
     *                   la configuration complète de tous les bus.
     *
     * @return true si l’initialisation s’est déroulée correctement.
     */
    static bool begin(const CanConfigProvider& provider);
};
