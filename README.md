# 🌐 CanUniversal

## Bibliothèque CAN universelle pour ESP32 — sans aucune dépendance externe

**CanUniversal v3.0.0** est une bibliothèque C++ modulaire pour l'**ESP32** (framework Arduino, PlatformIO).

Elle fournit une abstraction unifiée pour le bus CAN avec **deux moteurs natifs**, écrits en interne :

- le **CAN interne de l'ESP32** (contrôleur TWAI) — 2 broches suffisent
- un ou plusieurs **MCP2515 externes** (puce CAN reliée en SPI)

…et une **API unique** pour envoyer / recevoir des trames, quel que soit le moteur.

- **Zéro dépendance** : plus aucune bibliothèque externe (ACAN_ESP32 / ACAN2515 ont été retirés en v3.0.0)
- **Zéro interruption** : tout fonctionne en *polling* non bloquant — cohabitation parfaite avec le signal **DCC** (RMT)
- **Multi-bus** : jusqu'à 8 bus CAN, mélange libre TWAI + MCP2515
- **Trames 11 bits et 29 bits** (format Locoduino / Märklin), avec helpers de décodage
- **Filtre matériel optionnel** (« viseur ») par bus
- **Mode loopback matériel** par bus, pour tester sans câblage
- **Tolérante aux erreurs** : un bus absent ou mal câblé est marqué *invalide*, sans jamais faire crasher l'application

Utilisée dans l'architecture **Chemins de Fer du Chablais (CFC)** — DCC + CAN sur les mêmes cartes.

---

# ✨ Fonctionnalités

## 🔧 Initialisation automatique (CanInit)
- Détection du moteur par bus : **TWAI interne** si `cs_pin == GPIO_NUM_NC`, **MCP2515** sinon
- Support **multi-bus** : CAN0, CAN1, CAN2… (8 max)
- Tolérante aux erreurs : `begin()` ne retourne jamais false, chaque bus est vérifié via `isValid()`

## 📡 Gestion du bus (CanBus)
- `send()` / `receive()` non bloquants, résultat honnête (true/false)
- `CanBus::bus(index)` renvoie **toujours** une référence valide (jamais NULL)
- Loopback logiciel optionnel (`enableSoftwareLoopback()`) en complément du loopback matériel

## 📦 Messages typés (CanMsg)
- `id` + `dlc` + `data[8]` : simple et efficace
- `is11()` / `is29()` : le format se déduit de l'ID (comme `CanID`)
- Helpers : `prio()`, `cmde()`, `resp()`, `nodeId()` (découpage 29 bits Locoduino)

## 🎯 Filtre matériel (« viseur »)
Par bus, optionnel :

```cpp
// n'écouter que les trames 29 bits dont le champ prio vaut 0 :
CanBusConfig(/* ... */, 0x00000000, 0x07000000, true)
//                  └ filter_id   └ filter_mask └ filter_extended
```

- `filter_mask == 0` (défaut) : tout accepter
- sinon : n'écouter que les trames dont l'ID matche `filter_id`, en ignorant les bits où le masque vaut 0
- le filtrage est fait **par le matériel** : le processeur n'est pas dérangé pour rien

---

# 📦 Installation

### Via PlatformIO (recommandé)

Dans votre `platformio.ini` :

```ini
lib_deps =
    https://github.com/prune74/CanUniversal.git
```

*(Une fois la version publiée sur le registre : `prune74/CanUniversal@^3.0.0`)*

⚠️ Après une mise à jour du dépôt, forcez le re-téléchargement :
`pio pkg update -e <votre_env>` (ou supprimez `.pio/libdeps/...`).

### Via l'IDE Arduino

Ajoutez ce dépôt via le Gestionnaire de bibliothèque, ou copiez-le dans votre dossier `libraries/`.

---

# 📁 Structure du projet

```
CanUniversal/
 ├── src/
 │    ├── CanBackend.h         ← interface commune des moteurs (la « prise »)
 │    ├── TwaiBackend.h/.cpp   ← moteur CAN interne ESP32 (TWAI)
 │    ├── Mcp2515Backend.h/.cpp← moteur CAN externe (MCP2515 via SPI)
 │    ├── CanBus.h/.cpp        ← API unifiée multi-bus
 │    ├── CanInit.h/.cpp       ← initialisation tolérante aux erreurs
 │    ├── CanConfig.h          ← configuration + CanConfigProvider
 │    ├── CanMsg.h/.cpp        ← trame CAN (11 ou 29 bits)
 │    ├── CanID.h              ← découpage 29 bits Locoduino/Märklin
 │    └── CanUniversal.h       ← en-tête principal
 ├── Examples/
 │    └── BasicExample/BasicExample.ino
 ├── library.json              ← registre PlatformIO
 ├── library.properties        ← Arduino Library Manager
 ├── keywords.txt              ← coloration syntaxique
 ├── README.md
 └── LICENSE.txt               ← MIT
```

---

# 🚀 Exemple minimal (avec Provider)

```cpp
#include <CanUniversal.h>

struct MyCanConfig : public CanConfigProvider {
    CanBusConfig buses[1] = {
        //         speed  tx        rx        cs    int   sck   mosi  miso  quartz loopback
        CanBusConfig(true, 500000,
                     GPIO_NUM_4, GPIO_NUM_5,   // TWAI interne
                     GPIO_NUM_NC, GPIO_NUM_NC,
                     GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
                     0,                          // quartz (inutile en TWAI)
                     true)                       // loopback : test sans câblage
    };

    uint8_t busCount() const override { return 1; }
    const CanBusConfig& bus(uint8_t index) const override { return buses[index]; }
};

MyCanConfig canConfig;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // begin() est tolérante : un bus en erreur reste accessible (invalide)
    CanInit::begin(canConfig);

    if (!CanBus::bus(0).isValid())
        Serial.println("CAN0 invalide — vérifiez le câblage !");
}

void loop() {
    // Envoi — le cast indique le format (11 bits ici)
    CanMsg tx((uint16_t)0x123, { 0xAB, 0xCD });
    CanBus::bus(0).send(tx);

    // Réception
    CanMsg rx;
    if (CanBus::bus(0).receive(rx))
        Serial.printf("Reçu ID=0x%X DLC=%u\n", rx.id, rx.dlc);

    delay(100);
}
```

Voir [`Examples/BasicExample/BasicExample.ino`](Examples/BasicExample/BasicExample.ino) pour une version commentée, et le dépôt [Test-CanUniversal-3.0.0](https://github.com/prune74/Test-CanUniversal-3.0.0) pour un projet PlatformIO complet de test (loopback TWAI validé sur matériel).

💡 **Trames 29 bits** : `CanMsg tx((uint32_t)0x00086010, { ... });` — le cast `(uint32_t)` est obligatoire quand l'ID est écrit en dur (sinon le compilateur ne sait pas choisir entre les constructeurs 11 et 29 bits).

💡 **Bus réel** : mettez `loopback = false`, branchez un transceiver CAN (ex. SN65HVD230 côté ESP32, TJA1050 côté MCP2515) + 2 fils + une résistance de 120 Ω à chaque extrémité. Vitesse Märklin (laBox, CS2/CS3) : **250 000 bit/s**.

---

# 🧩 Architecture interne

```
                    CanInit::begin(provider)
                              │
             ┌────────────────┴────────────────┐
             │  pour chaque bus de la config   │
             │  cs_pin défini ?                │
             │   NON → TwaiBackend             │
             │   OUI → Mcp2515Backend          │
             └────────────────┬────────────────┘
                              │
                      CanBus::bus(index)
                              │
                 CanBackend (interface commune)
                 ├── TwaiBackend    → contrôleur TWAI (polling)
                 └── Mcp2515Backend→ puce SPI (polling, sans ISR)
```

Aucune interruption nulle part : l'application vient lire les trames quand **elle** le décide (`receive()` ne bloque jamais). Idéal à côté d'un signal DCC généré par RMT.

## CanID — format 29 bits Locoduino

```
[ prio : 3 bits ][ cmde : 8 bits ][ resp : 1 bit ][ nodeId : 16 bits ]
```

`CanID::make29(prio, cmde, resp, nodeId)` construit l'ID ; `prio()`, `cmde()`, `resp()`, `nodeId()` le décodent. Compatible avec le trafic **Märklin** (250 kbit/s, trames étendues).

---

# 🛠 Compatibilité

| Plateforme / moteur | Support |
| --- | --- |
| ESP32 — CAN interne (TWAI) | ✅ natif |
| MCP2515 externe (SPI) | ✅ natif (quartz 8 MHz : 125/250/500 kbit/s) |
| Multi-bus | ✅ (8 max, moteurs mélangés) |
| Trames 11 bits | ✅ |
| Trames 29 bits (Locoduino/Märklin) | ✅ |
| Filtre matériel | ✅ par bus |
| Framework Arduino / PlatformIO | ✅ espressif32 (arduino-esp32 2.x) |

---

# 📜 Licence

Ce projet est distribué sous licence MIT (voir [LICENSE.txt](LICENSE.txt)).
Vous êtes libre de l'utiliser dans vos projets personnels ou professionnels.

# 🤝 Contribuer

Les contributions sont bienvenues : Pull Requests, Issues, exemples supplémentaires.

# 🧭 Auteur

**Bruno Rivera** (prune74)
Projet Chemins de Fer du Chablais (CFC) — Architecture CAN & DCC