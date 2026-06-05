# CanUniversal  
**Bibliothèque CAN universelle pour ESP32 — Discovery 2026**

CanUniversal est une bibliothèque C++ légère et modulaire destinée aux projets embarqués basés sur ESP32.  
Elle fournit une abstraction unifiée pour :

- **ACAN_ESP32** (CAN interne ESP32)
- **ACAN2515** (CAN externe via MCP2515)
- **Gestion des IDs CAN**
- **Encapsulation / décapsulation de messages**
- **Initialisation automatique du bus**
- **Messages typés pour Discovery 2026**

Cette bibliothèque est utilisée dans le projet **Discovery Master Board** et dans l’architecture **Discovery 2026**.

---

## ✨ Fonctionnalités

- Initialisation automatique du CAN interne ou externe  
- Gestion centralisée des IDs (CanID)  
- Messages CAN typés (CanMsg)  
- Gestion du bus (CanBus)  
- Configuration simple (CanConfig)  
- Compatible ACAN_ESP32 et ACAN2515  
- Architecture modulaire, facile à intégrer  
- Code propre, lisible, documenté  

---

## 📦 Installation

### Via PlatformIO (recommandé)

Dans votre `platformio.ini` :

```ini
lib_deps =
    https://github.com/prune74/CanUniversal.git
```
📁 Structure du projet

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
🚀 Exemple minimal

```cpp
#include <CanMsg.h>
#include <CanInit.h>
#include <CanBus.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialisation du bus CAN (interne ou MCP2515 selon config)
    if (!CanInit::begin()) {
        Serial.println("Erreur d'initialisation CAN !");
        while (true);
    }

    Serial.println("CAN initialisé !");
}

void loop() {
    // Exemple d’envoi
    CanMsg msg;
    msg.id = 0x123;
    msg.len = 2;
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;

    CanBus::send(msg);

    // Exemple de réception
    if (CanBus::available()) {
        CanMsg rx = CanBus::receive();
        Serial.printf("Reçu ID=0x%X, len=%d\n", rx.id, rx.len);
    }

    delay(100);
}
```
