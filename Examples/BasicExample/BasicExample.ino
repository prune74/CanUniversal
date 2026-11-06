/**
 * ============================================================================
 *  🌐 CanUniversal — BasicExample
 * ============================================================================
 *  Exemple minimal : un seul bus CAN sur le contrôleur interne de l'ESP32
 *  (TWAI), en mode loopback — fonctionne même sans transceiver branché :
 *  tout ce qui est envoyé est rebouclé et reçu.
 *
 *  Broches par défaut : TX = GPIO 4, RX = GPIO 5.
 *
 *  Pour un MCP2515 externe, remplacez la configuration par :
 *      CanBusConfig(true, 250000, GPIO_NUM_NC, GPIO_NUM_NC,
 *                   GPIO_NUM_13, GPIO_NUM_14,   // CS, INT
 *                   GPIO_NUM_18, GPIO_NUM_23, GPIO_NUM_19,  // SCK, MOSI, MISO
 *                   8000000, 50, false)
 * ============================================================================
 */

#include <CanUniversal.h>

// -----------------------------------------------------------------------------
// Provider : la configuration matérielle est fournie par l'application,
// jamais par la bibliothèque.
// -----------------------------------------------------------------------------
struct MyCanConfig : public CanConfigProvider {

    CanBusConfig buses[1] = {
        // enabled, speed, tx, rx, cs, int, sck, mosi, miso, quartz, tolérance, loopback
        CanBusConfig(true, 500000,
                     GPIO_NUM_4, GPIO_NUM_5,      // TWAI interne
                     GPIO_NUM_NC, GPIO_NUM_NC,
                     GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
                     0, 0,
                     true)                        // loopback pour l'exemple
    };

    uint8_t busCount() const override { return 1; }

    const CanBusConfig& bus(uint8_t index) const override { return buses[index]; }
};

MyCanConfig canConfig;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n--- CanUniversal BasicExample ---");

    // Initialisation tolérante aux erreurs : toujours vérifier la validité
    // de chaque bus après l'appel.
    CanInit::begin(canConfig);

    if (!CanBus::bus(0).isValid()) {
        Serial.println("[ERREUR] CAN0 invalide — vérifiez le câblage.");
    } else {
        Serial.println("[OK] CAN0 prêt (loopback).");
    }
}

void loop() {
    // Envoi d'une trame 11 bits
    CanMsg tx((uint16_t)0x123, { 0xAB, 0xCD });
    if (CanBus::bus(0).send(tx)) {
        Serial.print("[TX] ID=0x123 DLC=2 DATA=");
        for (uint8_t i = 0; i < tx.dlc; i++) {
            Serial.printf("%02X ", tx.data[i]);
        }
        Serial.println();
    }

    // Réception (rebouclée en mode loopback)
    CanMsg rx;
    if (CanBus::bus(0).receive(rx)) {
        Serial.printf("[RX] ID=0x%X DLC=%u DATA=%02X %02X\n",
                      rx.id, rx.dlc, rx.data[0], rx.data[1]);
    }

    delay(1000);
}