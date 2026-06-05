#pragma once

#include "CanBus.h"
#include "CanConfig.h"

/**
 * ============================================================================
 * 🚀 CanInit — Initialisation centralisée des bus CAN (Discovery 2026)
 * ============================================================================
 *
 * Rôle :
 *   Cette classe est responsable de l’initialisation de *tous* les bus CAN
 *   déclarés par l’application via un CanConfigProvider.
 *
 *   La bibliothèque ne connaît :
 *     • ni les pins
 *     • ni les vitesses
 *     • ni le nombre de bus
 *     • ni le matériel utilisé (ESP32 interne ou MCP2515)
 *
 *   👉 Toutes ces informations sont fournies par l’application.
 *
 * Objectifs :
 *   • garder un setup() propre et lisible
 *   • rendre la bibliothèque totalement générique
 *   • permettre un nombre illimité de bus CAN
 *   • supporter plusieurs matériels simultanément
 *   • garantir une initialisation *tolérante aux erreurs*
 *
 * Fonctionnement :
 *   • Pour chaque bus décrit par le provider :
 *       - tentative d’initialisation
 *       - si OK → driver attaché
 *       - si erreur → bus attaché comme “invalide” (mais jamais NULL)
 *
 *   👉 Cela garantit qu’aucun accès CAN ne provoque un crash,
 *      même si un MCP2515 est absent ou mal câblé.
 *
 * ============================================================================
 */
class CanInit {
public:

    /**
     * ------------------------------------------------------------------------
     * 🟦 begin()
     * ------------------------------------------------------------------------
     * Initialise tous les bus CAN décrits par le provider.
     *
     * @param provider
     *        Objet fourni par l’application contenant la configuration
     *        complète de tous les bus (pins, vitesses, quartz, rôles…).
     *
     * @return true
     *         Toujours true : l’initialisation est *tolérante*.
     *         Les erreurs sont gérées bus par bus.
     *
     * Détails :
     *   • Chaque bus est initialisé indépendamment.
     *   • Un bus en erreur est marqué comme invalide mais reste accessible.
     *   • Aucun pointeur NULL n’est jamais exposé à l’application.
     *
     * ------------------------------------------------------------------------
     */
    static bool begin(const CanConfigProvider& provider);
};
