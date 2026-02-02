# Space2Corps Actuator Control System

## Description
Ce projet est un système de contrôle d'actionneurs pour CubeSat développé pour la mission Space2Corps. Il utilise un ESP32-C6 pour contrôler des servos moteurs et gérer les statuts de la mission via une machine à états finis.

## Caractéristiques
- Contrôle précis des servos moteurs via PWM
- Gestion des statuts de mission avec transitions valides
- Communication WiFi en mode AP
- Détection de fin de course
- Architecture multi-tâches avec FreeRTOS

## Matériel
- ESP32-C6 DevKitC-1
- Servo moteur (alimentation 4.8V-6V)
- Capteur de fin de course
- Driver L298N (optionnel pour moteurs pas-à-pas)

## Configuration

### PlatformIO
```ini
[env:esp32-c6-devkitc-1]
platform = espressif32
board = esp32-c6-devkitc-1
framework = espidf
monitor_speed = 115200
```

## Installation

1. Cloner le dépôt :
```bash
git clone https://github.com/votre-utilisateur/Space2Corps-Actuator.git
```

2. Configurer PlatformIO :
```bash
pio run -t menuconfig
```

3. Construire et téléverser :
```bash
pio run -t upload
```

## Utilisation

### Commandes disponibles
Le système accepte les commandes suivantes via le socket UDP :
- `STATUS`: Retourne le statut actuel
- `OPEN_HINGE`: Ouvre la charnière
- `CLOSE_HINGE`: Ferme la charnière
- `STANDBY`: Passe en mode veille

## Structure du projet

```
.
├── include/               # Fichiers d'en-tête
│   ├── actuator.h         # Contrôle des actionneurs
│   ├── main.h             # Définitions principales
│   ├── status.h           # Gestion des statuts
│   └── wifi.h             # Communication WiFi
├── src/                   # Code source
│   ├── actuator.c         # Implémentation des actionneurs
│   ├── main.c             # Code principal
│   ├── status.c           # Implémentation des statuts
│   └── wifi.c             # Implémentation WiFi
├── platformio.ini         # Configuration PlatformIO
└── README.md              # Ce fichier
```

## Développement

### Dépendances
- ESP-IDF v5.5.0
- FreeRTOS
- Driver LEDC pour PWM

### Tâches
- `wifi_task`: Gère la communication WiFi
- `control_task`: Contrôle les actionneurs et les statuts

### Notes supplémentaires pour le projet :

1. **Configuration recommandée** :
   - Alimentation servo : 6V externe
   - Alimentation ESP32-C6 : 5V via USB
   - Configuration des broches dans `actuator.h`

2. **Points d'amélioration possibles** :
   - Ajouter une calibration automatique du servo
   - Implémenter un protocole de communication plus robuste
   - Ajouter des tests unitaires

3. **Documentation complémentaire** :
   - Diagramme d'état dans le dossier `doc/`
   - Schéma de câblage dans `doc/wiring.png`
