#include "CanInit.h"
#include "CanConfig.h"
#include "CanBus.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>
#include <SPI.h>

/**
 * ============================================================================
 *  CanInit::begin(provider)
 * ============================================================================
 *  Initialise tous les bus CAN décrits par l’application via CanConfigProvider.
 *
 *  La bibliothèque ne connaît :
 *    - ni les pins
 *    - ni les vitesses
 *    - ni le nombre de bus
 *    - ni le matériel utilisé (ESP32 interne ou MCP2515)
 *
 *  → Toutes ces informations sont fournies par l’application.
 *
 *  La bibliothèque se contente de :
 *    1. Lire la configuration de chaque bus
 *    2. Déterminer s’il s’agit d’un bus interne (TWAI) ou externe (MCP2515)
 *    3. Instancier le bon driver ACAN
 *    4. Initialiser le bus
 *    5. Enregistrer le driver dans CanBus pour l’envoi/réception
 *
 *  Cette approche rend la bibliothèque :
 *    - totalement générique
 *    - indépendante du matériel
 *    - compatible multi‑bus
 *    - simple à intégrer dans n’importe quel projet
 * ============================================================================
 */

bool CanInit::begin(const CanConfigProvider& provider) {

    const uint8_t count = provider.busCount();

    Serial.printf("[CAN] Initialisation de %u bus...\n", count);

    for (uint8_t i = 0; i < count; i++) {

        // Récupération de la configuration du bus i
        const CanBusConfig& cfg = provider.bus(i);

        // ---------------------------------------------------------------------
        // Bus désactivé
        // ---------------------------------------------------------------------
        if (!cfg.enabled) {
            Serial.printf("[CAN%u] Désactivé\n", i);
            continue;
        }

        // ---------------------------------------------------------------------
        // CAN interne ESP32 (TWAI)
        // ---------------------------------------------------------------------
        // Convention : si cs_pin == GPIO_NUM_NC → ce n’est PAS un MCP2515
        //              donc c’est un bus CAN interne ESP32
        // ---------------------------------------------------------------------
        if (cfg.cs_pin == GPIO_NUM_NC) {

            ACAN_ESP32_Settings settings(cfg.speed);
            settings.mTxPin = cfg.tx_pin;
