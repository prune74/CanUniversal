#pragma once
#include <stdint.h>
#include <driver/gpio.h>

/**
 * ============================================================================
 *  CanBusConfig
 * ============================================================================
 *  Structure décrivant la configuration d'un bus CAN unique.
 *
 *  Cette structure est fournie par l'application (pas par la bibliothèque).
 *  Elle permet de décrire :
 *    - un bus CAN interne (TWAI de l'ESP32)
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

    bool loopback;         // Loopback TWAI (ESP32 interne uniquement)

    // ------------------------------------------------------------------------
    // Filtre matériel optionnel ("viseur")
    //   filter_mask == 0        -> tout accepter (défaut)
    //   filter_mask != 0        -> n'écouter que les trames dont l'ID matche
    //                               filter_id, en ignorant les bits où le
    //                               masque vaut 0.
    //   filter_extended = true  -> trames 29 bits (Märklin)
    //   filter_extended = false -> trames 11 bits (booster)
    // ------------------------------------------------------------------------
    uint32_t filter_id;
    uint32_t filter_mask;
    bool     filter_extended;

    // -------------------------------------------------------------------------
    // Constructeur par défaut sécurisé
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
          loopback(false),
          filter_id(0),
          filter_mask(0),
          filter_extended(true)
    {}

    // -------------------------------------------------------------------------
    // Constructeur complet — permet l'usage de { ... } dans MasterConfig
    // viseur optionnel : filter_mask = 0 par défaut = tout accepter.
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
        bool loopback,
        uint32_t filter_id = 0,
        uint32_t filter_mask = 0,
        bool filter_extended = true
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
          loopback(loopback),
          filter_id(filter_id),
          filter_mask(filter_mask),
          filter_extended(filter_extended)
    {}
};


/**
 * ============================================================================
 *  CanConfigProvider
 * ============================================================================
 *  Interface abstraite permettant à l'application de fournir :
 *    - le nombre total de bus CAN
 *    - la configuration de chaque bus
 *
 *  La bibliothèque CanUniversal ne dépend d'aucun fichier de config statique :
 *  elle interroge simplement ce provider.
 * ============================================================================
 */
struct CanConfigProvider {

    /** Retourne le nombre total de bus CAN disponibles */
    virtual uint8_t busCount() const = 0;

    /**
     * Retourne la configuration d'un bus donné.
     * @param index : numéro du bus (0, 1, 2...)
     */
    virtual const CanBusConfig& bus(uint8_t index) const = 0;
};