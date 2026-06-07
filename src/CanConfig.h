#pragma once
#include <stdint.h>
#include <driver/gpio.h>

/**
 * ============================================================================
 *  CanBusConfig
 * ============================================================================
 *  Structure décrivant la configuration d’un bus CAN unique.
 *
 *  Cette structure est fournie par l’application (pas par la bibliothèque).
 *  Elle permet de décrire :
 *    - un bus CAN interne (TWAI de l’ESP32)
 *    - ou un bus CAN externe (MCP2515)
 *
 *  La bibliothèque CanUniversal lit simplement ces valeurs pour initialiser
 *  correctement chaque bus, sans imposer de choix matériel.
 * ============================================================================
 */
struct CanBusConfig {

    /** Active ou désactive ce bus CAN */
    bool enabled;

    /** Vitesse du bus CAN (en bit/s) — ex : 125000, 250000, 500000, 1000000 */
    uint32_t speed;

    /** Broches TX/RX pour le CAN interne ESP32 (TWAI) */
    gpio_num_t tx_pin;
    gpio_num_t rx_pin;

    /** Broches pour le MCP2515 (bus SPI) */
    gpio_num_t cs_pin;     // Chip Select
    gpio_num_t int_pin;    // Interruption
    gpio_num_t sck_pin;    // SPI Clock
    gpio_num_t mosi_pin;   // SPI MOSI
    gpio_num_t miso_pin;   // SPI MISO

    /** Quartz du MCP2515 (en Hz) — ex : 8 MHz ou 16 MHz */
    uint32_t quartz;

    /** Tolérance ACAN2515 (en %) — ex : 50 */
    uint8_t tolerance;

    // =========================================================================
    // 🔥 Ajouts Discovery 2026 — support du mode loopback (tests sans matériel)
    // =========================================================================
    bool loopback = false;   // Active le mode loopback (ESP32 interne)
    bool no_ack   = false;   // Désactive l’attente d’ACK (ESP32 interne)
};


/**
 * ============================================================================
 *  CanConfigProvider
 * ============================================================================
 *  Interface abstraite permettant à l’application de fournir :
 *    - le nombre total de bus CAN
 *    - la configuration de chaque bus
 *
 *  La bibliothèque CanUniversal ne dépend donc plus d’un fichier de config
 *  statique : elle interroge simplement ce provider.
 *
 *  Avantages :
 *    - configuration propre dans l’application
 *    - bibliothèque totalement générique
 *    - support multi‑bus illimité
 *    - facile à adapter selon les projets (Master, Booster, Gateway…)
 * ============================================================================
 */
struct CanConfigProvider {

    /** Retourne le nombre total de bus CAN disponibles */
    virtual uint8_t busCount() const = 0;

    /**
     * Retourne la configuration d’un bus donné.
     * @param index : numéro du bus (0, 1, 2…)
     */
    virtual const CanBusConfig& bus(uint8_t index) const = 0;
};
