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
Display display(0x3C, 5000);
GPSModule gps(Serial1, 9600, display);
Battery battery(A0, 2500, 74, display);

// Initialisation du programme.
void setup()
{
    // Initialisation des objets.
    display.setDevices(&battery, &gps);
    display.begin();
    gps.begin();
    battery.begin();
    pinMode(6, INPUT_PULLUP);

    // Démarrage
    battery.startupCheck();
    display.displayStartupMessage();
    display.displayAutonomy(battery.getPercentage(), battery.getRemainingAutonomyHours(), battery.getRemainingAutonomyMinutes());
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

        delay(10); // TODO PEUT-être faire un meilleur système avec une lib ?
    }
}

// TODO Vérifier toutes les présentations de code et les commentaires dans le code source. Vérifier que tout est bien commenté et que tout est bien écrit. Vérifier que tout est bien formaté. Vérifier que tout est bien indenté. Vérifier que tout est bien espacé. Vérifier que tout est bien aligné. Vérifier que tout est bien organisé. Vérifier que tout est bien structuré. Vérifier que tout est bien lisible. Vérifier que tout est bien compréhensible. Vérifier que tout est bien clair. Vérifier que tout est bien concis. Vérifier que tout est bien précis.
// TODO Bien unifier l'organisation du code, avec les commentaires, les noms...