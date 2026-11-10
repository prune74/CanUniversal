// ============================================================================
//  TwaiBackend — moteur du CAN interne de l'ESP32 (TWAI)
//  CanUniversal v3.0.0 — sans dépendance externe
//
//  Le circuit TWAI fait tout le protocole CAN en matériel : on lui donne
//  vitesse + broches + viseur, puis on dépose / ramasse les trames.
//  Aucune interruption : polling pur, le signal DCC n'est jamais dérangé.
//  send()/receive() ne bloquent JAMAIS (timeout 0) et renvoient toujours
//  un résultat honnête : true = fait, false = pas possible maintenant.
// ============================================================================

#include "TwaiBackend.h"
#include "Arduino.h"

TwaiBackend::TwaiBackend(gpio_num_t txPin, gpio_num_t rxPin)
    : txPin(txPin), rxPin(rxPin) {}

bool TwaiBackend::begin(const CanBusConfig &cfg)
{
    savedSpeed = cfg.speed;
    loopbackMode = cfg.loopback;

    // 1) Où sont les fils ? + mode.
    //    loopback = auto-test matériel : le circuit se parle à lui-même,
    //    sans avoir besoin d'un autre participant (TWAI_MODE_NO_ACK).
    g = (twai_general_config_t)TWAI_GENERAL_CONFIG_DEFAULT(
        txPin, rxPin, TWAI_MODE_NORMAL);
    if (loopbackMode)
        g.mode = TWAI_MODE_NO_ACK;
    g.rx_queue_len = 20;
    g.tx_queue_len = 5;

    // 2) La vitesse. Les valeurs classiques sont pré-calculées par l'ESP32.
    //    250000 = la vitesse du monde Märklin (laBox, CS2/CS3, Gleisbox).
    const twai_timing_config_t t1M  = TWAI_TIMING_CONFIG_1MBITS();
    const twai_timing_config_t t800 = TWAI_TIMING_CONFIG_800KBITS();
    const twai_timing_config_t t500 = TWAI_TIMING_CONFIG_500KBITS();
    const twai_timing_config_t t250 = TWAI_TIMING_CONFIG_250KBITS();
    const twai_timing_config_t t125 = TWAI_TIMING_CONFIG_125KBITS();

    const twai_timing_config_t *t = nullptr;
    switch (cfg.speed) {
        case 1000000: t = &t1M;  break;
        case  800000: t = &t800; break;
        case  500000: t = &t500; break;
        case  250000: t = &t250; break;   // Märklin
        case  125000: t = &t125; break;
        default: return false;            // vitesse inconnue → refus propre
    }

    // 3) Le viseur (filtre matériel). Sans viseur : tout passe.
    //    Avec : le circuit ne réveille le programme QUE pour les trames
    //    voulues (id matché, bits du masque à 0 ignorés).
    twai_filter_config_t f;
    if (cfg.filter_mask == 0) {
        f = (twai_filter_config_t)TWAI_FILTER_CONFIG_ACCEPT_ALL();
    } else {
        f.acceptance_code = cfg.filter_id;
        f.acceptance_mask = ~cfg.filter_mask;
        f.single_filter = true;
    }

    // 4) Démarrage du circuit.
    if (twai_driver_install(&g, t, &f) != ESP_OK) return false;
    if (twai_start() != ESP_OK) {
        twai_driver_uninstall();
        return false;
    }
    return true;
}

bool TwaiBackend::send(const CanMsg &msg)
{
    if (!loopbackMode && isOff())
        recover();                 // bus malade → soigner avant d'émettre

    twai_message_t m = {};
    m.identifier = msg.id;
    m.extd = msg.is29();           // 11 ou 29 bits, comme CanID
    m.data_length_code = msg.dlc;
    for (uint8_t i = 0; i < msg.dlc; i++) m.data[i] = msg.data[i];

    // timeout 0 = "rends-moi la main immédiatement si c'est impossible" :
    // jamais bloquant, résultat honnête.
    return twai_transmit(&m, 0) == ESP_OK;
}

bool TwaiBackend::receive(CanMsg &msg)
{
    twai_message_t m = {};
    // timeout 0 = "s'il n'y a rien, dis-le tout de suite" : jamais bloquant.
    if (twai_receive(&m, 0) != ESP_OK) return false;

    msg.id = m.identifier;
    msg.dlc = m.data_length_code;
    for (uint8_t i = 0; i < m.data_length_code; i++) msg.data[i] = m.data[i];
    return true;
}

bool TwaiBackend::isOff()
{
    twai_status_info_t s = {};
    twai_get_status_info(&s);
    return s.state == TWAI_STATE_BUS_OFF;   // fils débranchés, court-circuit...
}

void TwaiBackend::recover()
{
    // Recette officielle ESP32 : arrêt, puis écoute du silence du bus
    // (128 séquences de 11 bits), puis le bus repartira au prochain send().
    twai_stop();
    twai_initiate_recovery();
}