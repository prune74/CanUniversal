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

    bool enabled;          // Active ou désactive le bus
    uint32_t speed;        // Vitesse en bit/s

    // Broches TWAI interne ESP32
    gpio_num_t tx_pin;
    gpio_num_t rx_pin;

    // Broches MCP2515 (SPI)
    gpio_num_t cs_pin;
    gpio_num_t int_pin;
    gpio_num_t sck_pin;
    gpio_num_t mosi_pin;
    gpio_num_t miso_pin;

    uint32_t quartz;       // Quartz MCP2515 (Hz)
    uint8_t  tolerance;    // Tolérance ACAN2515 (%)

    bool loopback;         // Loopback TWAI (ESP32 interne uniquement)

    // -------------------------------------------------------------------------
    // 🔹 Constructeur par défaut sécurisé
    // -------------------------------------------------------------------------
    CanBusConfig()
        : enabled(false),
          speed(0),
          tx_pin(GPIO_NUM_NC),
          rx_pin(GPIO_NUM_NC),
          cs_pin(GPIO_NUM_NC),
          int_pin(GPIO_NUM_NC),
          sck_pin(GPIO_NUM_NC),
          mosi_pin(GPIO_NUM_NC),
          miso_pin(GPIO_NUM_NC),
          quartz(0),
          tolerance(0),
          loopback(false)
    {}

    // -------------------------------------------------------------------------
    // 🔹 Constructeur complet — permet l’usage de { … } dans MasterConfig
    // -------------------------------------------------------------------------
    CanBusConfig(
        bool enabled,
        uint32_t speed,
        gpio_num_t tx_pin,
        gpio_num_t rx_pin,
        gpio_num_t cs_pin,
        gpio_num_t int_pin,
        gpio_num_t sck_pin,
        gpio_num_t mosi_pin,
        gpio_num_t miso_pin,
        uint32_t quartz,
        uint8_t tolerance,
        bool loopback
    )
        : enabled(enabled),
          speed(speed),
          tx_pin(tx_pin),
          rx_pin(rx_pin),
          cs_pin(cs_pin),
          int_pin(int_pin),
          sck_pin(sck_pin),
          mosi_pin(mosi_pin),
          miso_pin(miso_pin),
          quartz(quartz),
          tolerance(tolerance),
          loopback(loopback)
    {}
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
