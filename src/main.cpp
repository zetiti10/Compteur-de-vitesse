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
Battery battery(A0, 250, 74, display);

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
    //display.displayAutonomy(battery.getPercentage(), battery.getRemainingAutonomyHours(), battery.getRemainingAutonomyMinutes());
}

// Boucle d'exécution principale.
void loop()
{
    gps.loop();
    battery.loopCheck();
    display.loop();

    if (digitalRead(6) == LOW)
    {
        display.displayNextMenu();
        
        while(digitalRead(6) == LOW)
            delay(1);

        delay(10); // TODO Peut-être faire un meilleur système ?
    }
}

// TODO Bien unifier et vérifier l'organisation du code, avec les commentaires (doxygen), les noms...