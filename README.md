# 🌐 CanUniversal
## Bibliothèque CAN universelle pour ESP32 — Architecture Discovery 2026
**CanUniversal** est une bibliothèque C++ modulaire conçue pour les projets embarqués basés sur **ESP32**.

Elle fournit une abstraction unifiée permettant d’utiliser :
- le CAN interne ESP32 (TWAI) via ACAN_ESP32
- un ou plusieurs MCP2515 externes via ACAN2515
- une API unique pour envoyer/recevoir des trames
- une configuration matérielle fournie par l’application (Provider)
- des messages typés Discovery 2026 (CanMsg, CanID)

Cette bibliothèque est utilisée dans :
- Discovery Master Board
- DCC2CAN
- Booster Discovery 2026
- Gateway CAN/DCC

et plus largement dans l’architecture Discovery 2026.

# ✨ Fonctionnalités principales

## 🔧 Initialisation automatique
- Détection du type de bus :
  - **ESP32 interne** si `cs_pin == GPIO_NUM_NC`
  - **MCP2515 externe** sinon
- Support **multi‑bus** : CAN0, CAN1, CAN2…

---

## 📡 Gestion du bus (CanBus)
- Envoi / réception de trames  
- Bufferisation interne  
- Gestion des erreurs  
- API unifiée pour tous les drivers (ACAN_ESP32 / ACAN2515)

---

## 📦 Messages typés (CanMsg)
- Structure simple et efficace  
- Helpers pour manipuler les données  
- Compatible avec l’architecture **Discovery 2026**

---

## 🧩 Configuration modulaire (CanConfigProvider)
Chaque module fournit sa propre configuration matérielle :

- **MasterConfig**  
- **Dcc2CanConfig**  
- **BoosterConfig**  
- **GatewayConfig**

➡️ **Aucun code matériel dans la bibliothèque**, tout est externalisé dans les modules.

---

## 🧠 Architecture propre
- Code clair, documenté, maintenable  
- Séparation stricte entre :
  - **la bibliothèque générique** (CanUniversal)  
  - **les modules Discovery spécifiques** (Master, Booster, DCC2CAN, Gateway…)

## 📦 Installation

### Via PlatformIO (recommandé)

Dans votre `platformio.ini` :

```ini
lib_deps =
    https://github.com/prune74/CanUniversal.git
```
## 📁 Structure du projet

```Code
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
 ├── examples/
 │    └── BasicExample/
 │         └── BasicExample.ino
 ├── library.json
 ├── README.md
 └── LICENSE
```
## 🚀 Exemple minimal (avec Provider)

```cpp
#include <CanMsg.h>
#include <CanInit.h>
#include <CanBus.h>
#include "MyCanConfig.h"   // votre provider

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!CanInit::begin(MY_CAN_CONFIG)) {
        Serial.println("Erreur d'initialisation CAN !");
        while (true);
    }

    Serial.println("CAN initialisé !");
}

void loop() {
    // Envoi
    CanMsg msg;
    msg.id = 0x123;
    msg.len = 2;
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;
    CanBus::send(msg);

    // Réception
    if (CanBus::available()) {
        CanMsg rx = CanBus::receive();
        Serial.printf("Reçu ID=0x%X, len=%d\n", rx.id, rx.len);
    }

    delay(100);
}

```
# 🧩 Modules internes

## CanInit
- Initialise automatiquement chaque bus CAN  
- Sélectionne le driver approprié (**ESP32 interne** ou **MCP2515 externe**)  
- Attache les drivers à CanBus  

---

## CanBus
- API unifiée pour envoyer et recevoir des trames  
- Compatible **multi‑bus**  
- Gestion des erreurs et de la bufferisation interne  

---

## CanMsg
- Structure de trame CAN simple et efficace  
- Helpers pour manipuler les données (lecture/écriture, cast, etc.)  

---

## CanID
- Gestion centralisée des IDs Discovery 2026  
- Encodage et décodage des champs d’identification  

---

## CanConfigProvider
- Interface permettant à l’application de fournir sa configuration matérielle  
- Permet une architecture totalement modulaire (Master, DCC2CAN, Booster, Gateway…)  


## 🛠 Compatibilité

| Plateforme | Support |
| --- | --- |
| ESP32 (CAN interne) | ✔️ ACAN_ESP32 |
| MCP2515 (CAN externe) | ✔️ ACAN2515 |
| Arduino Framework | ✔️ |
| PlatformIO | ✔️ |

## 📜 Licence
Ce projet est distribué sous licence MIT.
Vous êtes libre de l’utiliser dans vos projets personnels ou professionnels.

## 🤝 Contribuer
Les contributions sont les bienvenues :
- Pull Requests
- Issues
- Améliorations de documentation
- Exemples supplémentaires

## 🧭 Auteur
Bruno (prune74)  
Projet Discovery 2026 — Architecture CAN & DCC
