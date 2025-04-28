/**
 * @file main.cpp
 * @author Louis L
 * @brief Fichier principal du compteur de vitesse.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>

// Autres fichiers du programme.
#include "display.hpp"
#include "GPSModule.hpp"
#include "battery.hpp"

// Instanciation des objets.
Display display(0x3C, 10000);
GPSModule gps(Serial1, 9600, display);
Battery battery(A0, 1000, 160, display);

// Initialisation du programme.
void setup()
{
    // Initialisation des objets.
    display.setDevices(&battery, &gps);
    display.begin();
    gps.begin();
    battery.begin();
    pinMode(6, INPUT_PULLUP);
    Serial.begin(115200);

    // Démarrage
    battery.startupCheck();
    display.displayStartupMessage();
}

// Boucle d'exécution principale.
void loop()
{
    // Exécution des tâches de chaque composant.
    gps.loop();
    battery.loopCheck();
    display.loop();

    // Gestion du bouton.
    if (digitalRead(6) == LOW)
    {
        display.displayNextMenu();

        while (digitalRead(6) == LOW)
            delay(1);

        delay(10);
    }
}