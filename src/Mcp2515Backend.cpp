// ============================================================================
//  Mcp2515Backend — moteur du CAN externe (MCP2515 via SPI)
//  CanUniversal v3.0.0
// ============================================================================

#include "Mcp2515Backend.h"
#include "driver/gpio.h"

// --- Commandes SPI comprises par la puce (chapitre "SPI" du manuel) ---

static const uint8_t CMD_RESET    = 0xC0;  // "redémarre tout"
static const uint8_t CMD_WRITE    = 0x02;  // "je vais écrire un réglage"
static const uint8_t CMD_READ     = 0x03;  // "je vais lire un réglage"
static const uint8_t CMD_MODIFY    = 0x05;  // "change seulement certains bits"
static const uint8_t CMD_LOAD_TX0  = 0x40;  // "voici une trame (case d'envoi 0)"
static const uint8_t CMD_SEND_TX0  = 0x81;  // "envoie la case 0 !"
static const uint8_t CMD_READ_RX0  = 0x90;  // "lis-moi la case de réception 0"

// --- Adresses des réglages DANS la puce ---

static const uint8_t REG_RXF0SIDH  = 0x00;  // viseur n°0
static const uint8_t REG_RXM0SIDH  = 0x20;  // masque n°0 (bits ignorés)
static const uint8_t REG_CNF1      = 0x2A;  // vitesse, morceau 1/3
static const uint8_t REG_CNF2      = 0x2B;  // vitesse, morceau 2/3
static const uint8_t REG_CNF3      = 0x2C;  // vitesse, morceau 3/3
static const uint8_t REG_CANINTF   = 0x2C;  // drapeaux (en mode service)
static const uint8_t REG_EFLG      = 0x2D;  // registre des erreurs
static const uint8_t REG_CANCTRL   = 0x0F;  // le "volant" de la puce
static const uint8_t REG_CANSTAT   = 0x1E;  // ce que la puce fait
static const uint8_t REG_TXB0CTRL  = 0x30;  // case d'envoi n°0
static const uint8_t REG_RXB0CTRL  = 0x60;  // case de réception n°0
static const uint8_t REG_RXB1CTRL  = 0x70;  // case de réception n°1

// --- Bits utiles ---

static const uint8_t F_RX0IF     = 0x01;  // "quelque chose à lire (case 0)"
static const uint8_t F_TXREQ     = 0x08;  // "la case d'envoi est occupée"
static const uint8_t F_TXBO      = 0x80;  // "bus mort" (bus-off)
static const uint8_t MODE_CONFIG = 0x80;  // mode "réglages" (REQOP=100)
static const uint8_t MODE_SLEEP  = 0x20;  // mode "sommeil"  (REQOP=001)

Mcp2515Backend::Mcp2515Backend(const CanBusConfig &cfg)
    : saved(cfg)
{
}

// --- Petites commandes SPI --------------------------------------------------

uint8_t Mcp2515Backend::readReg(uint8_t reg)
{
    select();
    SPI.transfer(CMD_READ);
    SPI.transfer(reg);
    uint8_t v = SPI.transfer(0);   // envoie des zéros pour recevoir
    deselect();
    return v;
}

void Mcp2515Backend::writeReg(uint8_t reg, uint8_t value)
{
    select();
    SPI.transfer(CMD_WRITE);
    SPI.transfer(reg);
    SPI.transfer(value);
    deselect();
}

void Mcp2515Backend::setBits(uint8_t reg, uint8_t mask)
{
    select();
    SPI.transfer(CMD_MODIFY);
    SPI.transfer(reg);
    SPI.transfer(mask);       // bits visés
    SPI.transfer(mask);       // ... mis à 1
    deselect();
}

void Mcp2515Backend::clearBits(uint8_t reg, uint8_t mask)
{
    select();
    SPI.transfer(CMD_MODIFY);
    SPI.transfer(reg);
    SPI.transfer(mask);       // bits visés
    SPI.transfer(0);          // ... mis à 0
    deselect();
}

bool Mcp2515Backend::waitConfigMode()
{
    // Demander le "mode réglages" et attendre la confirmation de la puce.
    setBits(REG_CANCTRL, MODE_CONFIG);
    for (int i = 0; i < 50; i++) {
        if ((readReg(REG_CANSTAT) >> 5) == 0x04) return true;
        delay(1);
    }
    return false;   // puce muette : fils branchés ? alimentation ?
}

// --- Démarrage --------------------------------------------------------------

bool Mcp2515Backend::begin(const CanBusConfig &cfg)
{
    saved = cfg;
    cs = (int)cfg.cs_pin;

    // Ligne SPI sur ses 4 fils (les MCP2515 partagent SCK/MOSI/MISO,
    // chacun a son CS).
    SPI.begin((int)cfg.sck_pin, (int)cfg.miso_pin, (int)cfg.mosi_pin, cs);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    pinMode((int)cfg.int_pin, INPUT);   // inutilisée en polling, protégée

    // Réveiller la puce proprement : petit reset général.
    select(); SPI.transfer(CMD_RESET); deselect();
    delay(2);

    // Entrer en "mode réglages" pour tout pouvoir configurer.
    if (!waitConfigMode()) return false;

    // La vitesse, selon le quartz soudé près de la puce.
    // Table pour le quartz le plus courant (8 MHz) — valeurs du manuel.
    uint8_t cnf1, cnf2, cnf3;
    if (cfg.quartz == 8000000) {
        switch (cfg.speed) {
            case 500000: cnf1 = 0x40; cnf2 = 0x92; cnf3 = 0x02; break;
            case 250000: cnf1 = 0x40; cnf2 = 0xB1; cnf3 = 0x05; break; // Märklin
            case 125000: cnf1 = 0x41; cnf2 = 0xB1; cnf3 = 0x05; break;
            default: return false;      // vitesse inconnue sur ce quartz
        }
    } else {
        return false;   // autre quartz : ajouter les lignes ici
    }
    writeReg(REG_CNF1, cnf1);
    writeReg(REG_CNF2, cnf2);
    writeReg(REG_CNF3, cnf3);

    // Le viseur (filtre matériel) : masque ouvert si filter_mask == 0,
    // sinon identifiant + masque traduits en 4 octets "puce".
    if (cfg.filter_mask == 0) {
        writeReg(REG_RXM0SIDH,     0);
        writeReg(REG_RXM0SIDH + 1, 0);
        writeReg(REG_RXM0SIDH + 2, 0);
        writeReg(REG_RXM0SIDH + 3, 0);
    } else {
        uint32_t id = cfg.filter_id, mk = cfg.filter_mask;
        uint8_t ie = cfg.filter_extended ? 0x08 : 0x00;
        writeReg(REG_RXF0SIDH,     (id >> 21) & 0xFF);
        writeReg(REG_RXF0SIDH + 1, ((id >> 16) & 0xE0) | ie | ((id >> 13) & 0x03));
        writeReg(REG_RXF0SIDH + 2, (id >> 8) & 0xFF);
        writeReg(REG_RXF0SIDH + 3, id & 0xFF);
        writeReg(REG_RXM0SIDH,     (mk >> 21) & 0xFF);
        writeReg(REG_RXM0SIDH + 1, ((mk >> 16) & 0xE0) | ie | ((mk >> 13) & 0x03));
        writeReg(REG_RXM0SIDH + 2, (mk >> 8) & 0xFF);
        writeReg(REG_RXM0SIDH + 3, mk & 0xFF);
    }

    // Réception : case 0 filtrée ; si elle déborde, la trame roule
    // dans la case 1 (BUKT = débordement autorisé).
    writeReg(REG_RXB0CTRL, 0x04);
    writeReg(REG_RXB1CTRL, 0x60);

    // Aucune interruption : on lira les drapeaux nous-mêmes (polling).
    writeReg(0x2B, 0x00);            // CANINTE : rien d'autorisé
    writeReg(REG_CANINTF, 0x00);     // drapeaux effacés

    // Retour au mode "en service".
    clearBits(REG_CANCTRL, MODE_CONFIG);
    begun = true;
    return true;
}

// --- Envoyer -----------------------------------------------------------------

bool Mcp2515Backend::send(const CanMsg &msg)
{
    // Case d'envoi libre ? Sinon on rend la main TOUT DE SUITE.
    if (readReg(REG_TXB0CTRL) & F_TXREQ) return false;

    // Traduire l'adresse en 4 octets "puce".
    uint32_t id = msg.id;
    uint8_t sidh, sidl, eid8, eid0;
    if (msg.is29()) {                     // enveloppe longue 29 bits
        uint32_t sid = (id >> 18) & 0x7FF;
        uint32_t eid = id & 0x3FFFF;
        sidh = sid >> 3;
        sidl = ((sid & 0x07) << 5) | 0x08 | ((eid >> 16) & 0x03);
        eid8 = eid >> 8;
        eid0 = eid & 0xFF;
    } else {                              // enveloppe courte 11 bits
        sidh = id >> 3;
        sidl = (id & 0x07) << 5;
        eid8 = 0; eid0 = 0;
    }

    // Déposer la trame dans la case 0, d'un seul coup SPI.
    select();
    SPI.transfer(CMD_LOAD_TX0);
    SPI.transfer(sidh); SPI.transfer(sidl);
    SPI.transfer(eid8); SPI.transfer(eid0);
    SPI.transfer(msg.dlc);
    for (uint8_t i = 0; i < 8; i++)
        SPI.transfer(i < msg.dlc ? msg.data[i] : 0);
    deselect();

    // Puis : "envoie !"
    select(); SPI.transfer(CMD_SEND_TX0); deselect();
    return true;
}

// --- Recevoir ------------------------------------------------------------------

bool Mcp2515Backend::receive(CanMsg &msg)
{
    // Quelqu'un a-t-il déposé une trame dans la case 0 ?
    if (!(readReg(REG_CANINTF) & F_RX0IF)) return false;

    select();
    SPI.transfer(CMD_READ_RX0);           // lecture en un seul coup
    uint8_t sidh = SPI.transfer(0);
    uint8_t sidl = SPI.transfer(0);
    uint8_t eid8 = SPI.transfer(0);
    uint8_t eid0 = SPI.transfer(0);
    uint8_t dlc  = SPI.transfer(0);
    msg.data[0] = SPI.transfer(0); msg.data[1] = SPI.transfer(0);
    msg.data[2] = SPI.transfer(0); msg.data[3] = SPI.transfer(0);
    msg.data[4] = SPI.transfer(0); msg.data[5] = SPI.transfer(0);
    msg.data[6] = SPI.transfer(0); msg.data[7] = SPI.transfer(0);
    deselect();

    // Recoller l'adresse.
    if (sidl & 0x08) {                    // bit IDE : enveloppe longue
        uint32_t sid = ((uint32_t)sidh << 3) | (sidl >> 5);
        msg.id = (sid << 18) | (((uint32_t)(sidl & 0x03)) << 16)
                 | (((uint32_t)eid8) << 8) | eid0;
    } else {
        msg.id = ((uint32_t)sidh << 3) | (sidl >> 5);
    }
    msg.dlc = dlc & 0x0F;
    clearBits(REG_CANINTF, F_RX0IF);      // éteindre le drapeau
    return true;
}

// --- Santé du bus -------------------------------------------------------------

bool Mcp2515Backend::isOff()
{
    // Drapeau "bus mort" du registre d'erreurs (fils coupés...).
    return (readReg(REG_EFLG) & F_TXBO) != 0;
}

void Mcp2515Backend::recover()
{
    // Recette de secours : reset, puis remise des réglages sauvegardés.
    if (!begun) return;
    select(); SPI.transfer(CMD_RESET); deselect();
    delay(2);
    begin(saved);
}