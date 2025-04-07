/**
 * @file main.cpp
 * @author Louis L
 * @brief Fichier principal du compteur de vitesse.
 * @version 2.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>

// Autres fichiers du programme.
#include "main.hpp"
#include "bitmaps.hpp"

// Instanciation des objets.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
TinyGPSPlus gps;

// Variables globales.
unsigned long lastDisplayUpdate = 0;
unsigned long lastAnimation = 0;
int animationStep = 0;
bool newData = false;
bool ready = false;

// Fonction permettant de calculer le décalage en `x` permettant de centrer un texte d'une certaine longueur et taille.
int yToCenterText(int size, int length)
{
  return ceil((SCREEN_WIDTH - double((6 * size) * length)) / 2);
}

// Initialisation du programme.
void setup()
{
  GPS_SERIAL.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();
}

// Boucle d'exécution principale.
void loop()
{
  // Lecture des données provenant du module GPS.
  while (GPS_SERIAL.available())
  {
    if (gps.encode(GPS_SERIAL.read()))
      newData = true;
  }

  // Affichage de l'animation de démarrage et vérification de l'état du module GPS lorsque le signal GPS n'a pas encore été détecté.
  if (!ready)
  {
    // Passage en mode opérationnel lorsque le signal GPS est détecté.
    if (gps.speed.isValid())
      ready = true;

    // Affichage de l'animation de démarrage.
    else
    {
      // Gestion de l'animation de démarrage pour afficher logo animé.
      if ((millis() - lastAnimation) >= 750)
      {
        // Gestion de la boucle d'animation.
        lastAnimation = millis();
        animationStep++;
        if (animationStep > 2)
          animationStep = 0;

        // Affichage des informations sur l'écran.
        display.clearDisplay();
        display.setCursor(ceil((128.0 - double((6 * 1) * 19)) / 2), 0);
        display.print("D");
        display.write(0x82);
        display.print("tection du signal");
        display.drawBitmap(0, 0, connectionBitmaps[animationStep], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
        display.display();
      }
      return;
    }
  }

  // Mise à jour des informations sur l'écran.
  if (newData && ((millis() - lastDisplayUpdate) >= (1000.0 / FPS)))
  {
    lastDisplayUpdate = millis();

    display.clearDisplay();

    // Affichage de la vitesse actuelle.
    // Calcul du centrage de la vitesse.
    int speed = int(gps.speed.kmph());
    if (speed < 10)
      display.setCursor(yToCenterText(SPEED_SIZE, 1), 0);
    else if (speed < 100)
      display.setCursor(yToCenterText(SPEED_SIZE, 2), 0);
    else
      display.setCursor(yToCenterText(SPEED_SIZE, 3), 0);
    // Impression de la vitesse.
    display.setTextSize(SPEED_SIZE);
    display.print(speed);
    display.setTextSize(1);

    // Calcul de l'affichage de l'heure.
    int hourOffset = 1;
    if (gps.date.month() > 3 && gps.date.month() < 11)
      hourOffset = 2;
    String hour = String(gps.time.hour() + hourOffset);
    if (gps.time.hour() < 10)
      hour = "0" + String(gps.time.hour() + hourOffset);
    String minute = String(gps.time.minute());
    if (gps.time.minute() < 10)
      minute = "0" + String(gps.time.minute());
    String time = hour + ":" + minute;
    // Affichage de l'heure
    display.setCursor(68, 50);
    display.setTextSize(2);
    display.print(time);

    // Affichage de l'orientation.
    display.setCursor(0, 50);
    display.print(gps.cardinal(gps.course.value()));
    display.setTextSize(1);

    display.display();
  }
}