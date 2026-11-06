# 🌐 CanUniversal
## Bibliothèque CAN universelle pour ESP32 — Architecture Chemins de Fer du Chablais (CFC)
**CanUniversal** est une bibliothèque C++ modulaire conçue pour les projets embarqués basés sur **ESP32**.

Elle fournit une abstraction unifiée permettant d'utiliser :
- le CAN interne ESP32 (TWAI) via ACAN_ESP32
- un ou plusieurs MCP2515 externes via ACAN2515
- une API unique pour envoyer/recevoir des trames
- une configuration matérielle fournie par l'application (Provider)
- des messages typés Chemins de Fer du Chablais (CFC) (CanMsg, CanID)

Cette bibliothèque est utilisée dans :
- l'architecture Chemins de Fer du Chablais (CFC).

# ✨ Fonctionnalités principales

## 🔧 Initialisation automatique
- Détection du type de bus :
  - **ESP32 interne** si `cs_pin == GPIO_NUM_NC`
  - **MCP2515 externe** sinon
- Support **multi‑bus** : CAN0, CAN1, CAN2…
- Tolérante aux erreurs : un bus absent ou mal câblé est marqué **invalide**, sans jamais faire crasher l'application

---

## 📡 Gestion du bus (CanBus)
- Envoi / réception de trames
- API unifiée pour tous les drivers (ACAN_ESP32 / ACAN2515)
- Accès sécurisé : `CanBus::bus(index)` renvoie toujours une référence valide

---

## 📦 Messages typés (CanMsg)
- Structure simple et efficace
- Helpers pour manipuler les données
- Compatible avec l'architecture **Chemins de Fer du Chablais (CFC)**

---

## 🧩 Configuration modulaire (CanConfigProvider)
Chaque module fournit sa propre configuration matérielle.

➡️ **Aucun code matériel dans la bibliothèque**, tout est externalisé dans les modules.

---

## 🧠 Architecture propre
- Code clair, documenté, maintenable
- Séparation stricte entre :
  - **la bibliothèque générique** (CanUniversal)
  - **les modules spécifiques**

## 📦 Installation

### Via PlatformIO (recommandé)

Dans votre `platformio.ini` :

```ini
lib_deps =
    prune74/CanUniversal
```

Les dépendances ACAN (`ACAN_ESP32`, `ACAN2515`) sont déclarées dans `library.json` et installées automatiquement.

### Via l'IDE Arduino

Ajoutez ce dépôt via le Gestionnaire de bibliothèque ou copiez-le dans votre dossier `libraries/`.

## 📁 Structure du projet

```
CanUniversal/
 ├── src/
 │    ├── CanBus.cpp
 │    ├── CanBus.h
 │    ├── CanConfig.h
 │    ├── CanID.h
 │    ├── CanInit.cpp
 │    ├── CanInit.h
 │    ├── CanMsg.cpp
 │    ├── CanMsg.h
 │    └── CanUniversal.h      ← en-tête principal
 ├── examples/
 │    └── BasicExample/
 │         └── BasicExample.ino
 ├── library.json              ← registre PlatformIO
 ├── library.properties        ← Arduino Library Manager
 ├── keywords.txt              ← coloration syntaxique
 ├── README.md
 └── LICENSE
```

## 🚀 Exemple minimal (avec Provider)

```cpp
#include <CanUniversal.h>

struct MyCanConfig : public CanConfigProvider {
    CanBusConfig buses[1] = {
        // enabled, speed, tx, rx, cs, int, sck, mosi, miso, quartz, tolérance, loopback
        CanBusConfig(true, 500000,
                     GPIO_NUM_4, GPIO_NUM_5,      // TWAI interne
                     GPIO_NUM_NC, GPIO_NUM_NC,
                     GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
                     0, 0,
                     false)
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

    if (!CanBus::bus(0).isValid()) {
        Serial.println("CAN0 invalide — vérifiez le câblage !");
    }
}

void loop() {
    // Envoi
    CanMsg tx((uint16_t)0x123, { 0xAB, 0xCD });
    CanBus::bus(0).send(tx);

    // Réception
    CanMsg rx;
    if (CanBus::bus(0).receive(rx)) {
        Serial.printf("Reçu ID=0x%X DLC=%u\n", rx.id, rx.dlc);
    }

    delay(100);
}
```

Voir [`examples/BasicExample/BasicExample.ino`](examples/BasicExample/BasicExample.ino) pour une version complète (mode loopback, configuration MCP2515 en commentaire).

# 🧩 Modules internes

## CanInit
- Initialise automatiquement chaque bus CAN
- Sélectionne le driver approprié (**ESP32 interne** ou **MCP2515 externe**)
- Installe l'ISR de réception MCP2515 (indispensable : sans elle, aucune trame n'est reçue)
- Attache les drivers à CanBus

---

## CanBus
- API unifiée pour envoyer et recevoir des trames
- Compatible **multi‑bus** (jusqu'à 8)
- Bus invalide = bus factice sûr, jamais de crash

---

## CanMsg
- Structure de trame CAN simple et efficace (11 ou 29 bits, 0–8 octets)
- Helpers pour manipuler les données

---

## CanID
- Gestion centralisée des IDs Chemins de Fer du Chablais (CFC)
- Encodage et décodage des champs d'identification (format 29 bits)

---

## CanConfigProvider
- Interface permettant à l'application de fournir sa configuration matérielle
- Permet une architecture totalement modulaire

## 🛠 Compatibilité

| Plateforme | Support |
| --- | --- |
| ESP32 (CAN interne) | ✔️ ACAN_ESP32 |
| MCP2515 (CAN externe) | ✔️ ACAN2515 |
| Multi‑bus | ✔️ (8 bus max) |
| Arduino Framework | ✔️ |
| PlatformIO | ✔️ |

## 📜 Licence
Ce projet est distribué sous licence MIT (voir [LICENSE](LICENSE)).
Vous êtes libre de l'utiliser dans vos projets personnels ou professionnels.

## 🤝 Contribuer
Les contributions sont les bienvenues :
- Pull Requests
- Issues
- Améliorations de documentation
- Exemples supplémentaires

## 🧭 Auteur
Bruno Rivera (prune74)
Projet Chemins de Fer du Chablais (CFC) — Architecture CAN & DCC