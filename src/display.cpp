/**
 * @file display.cpp
 * @author Louis L
 * @brief Fichier contenant les méthodes de la classe de gestion de l'écran.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

// Autres fichiers du programme.
#include "display.hpp"
#include "GPSModule.hpp"
#include "battery.hpp"
#include "bitmaps.hpp"
#include "utils.hpp"

/// @brief Instancie la classe gérant l'écran du compteur.
Display::Display(unsigned int busAddress, unsigned int messageShowTime) : m_display(128, 64, &Wire, -1), m_address(busAddress), m_currentMenu(MAIN_MENU), m_battery(nullptr), m_GPSModule(nullptr), m_messageShowTime(messageShowTime), m_menuTimer(0), m_connected(false), m_timeSeparatorPhase(false) {}

/// @brief Méthode permettant de définir les objets nécessaires au fonctionnement de l'instance.
/// @param battery Un pointeur vers l'objet batterie du système.
/// @param gpsModule Un pointeur vers l'objet gérant le module GPS du système.
void Display::setDevices(Battery *battery, GPSModule *gpsModule)
{
    m_battery = battery;
    m_GPSModule = gpsModule;
}

/// @brief Initialise l'écran.
void Display::begin()
{
    m_display.begin(SSD1306_SWITCHCAPVCC, m_address);

    m_display.clearDisplay();
    m_display.display();
}

/// @brief Gère le retour au menu principal après un certain temps à afficher un autre menu et gère le clignotement de la séparation heure/minute de l'affichage de l'heure actuelle.
void Display::loop()
{
    // Pour les menus ou les messages spéciaux, on retourne à l'écran d'accueil après un certain temps.
    if (m_currentMenu == DRIVING_MENU || m_currentMenu == BATTERY_MENU || m_currentMenu == LOW_BATTERY_MESSAGE)
    {
        if ((millis() - m_menuTimer) >= m_messageShowTime)
        {
            m_currentMenu = MAIN_MENU;
            m_menuTimer = millis();

            this->dataUpdated();
        }
    }

    // Animation du ":" de l'heure.
    if (m_currentMenu == MAIN_MENU && m_connected)
    {
        unsigned int speed = 1000;
        bool shouldBlink = (millis() % speed) < (speed / 2);

        if (shouldBlink != m_timeSeparatorPhase)
        {
            m_timeSeparatorPhase = shouldBlink;
            this->displayTimeSeparator(m_timeSeparatorPhase);
        }
    }
}

/// @brief Affiche l'écran de démarrage.
void Display::displayStartupMessage()
{
    m_display.clearDisplay();
    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(3, 15);
    m_display.print("Compteur GPS");

    m_display.drawBitmap(30, 25, image_speed_meter_bits, 68, 39, 1);

    m_display.display();

    // Temporise pendant trois secondes pour afficher le message (le blocage n'est pas gênant car le module GPS prend du temps à se connecter).
    delay(3000);

    this->dataUpdated();
}

/// @brief Méthode a appeler lorsque le bouton est pressé pour afficher le menu suivant.
void Display::displayNextMenu()
{
    switch (m_currentMenu)
    {
    // Affichage des informations sur la conduite : vitesse maximale et temps depuis le démarrage.
    case MAIN_MENU:
    {
        m_display.clearDisplay();

        m_display.drawBitmap(124, 29, image_next_menu_bits, 4, 7, 1);

        m_display.setTextColor(1);
        m_display.setTextWrap(false);
        m_display.setFont(&FreeSans9pt7b);
        m_display.setCursor(28, 12);
        m_display.print("Conduite");

        m_display.setCursor(18, 34);
        m_display.print(m_GPSModule->getMaxSpeed());
        m_display.print(F(" km/h"));

        m_display.setCursor(18, 52);
        unsigned long minutes = millis() / 60000;
        unsigned long hours = minutes / 60;
        minutes = minutes % 60;
        m_display.print(hours);
        m_display.print(F("h et "));
        m_display.print(minutes);
        m_display.print(F("min"));

        m_display.drawBitmap(0, 39, image_clock_bits, 15, 16, 1);
        m_display.drawBitmap(0, 20, image_wind_bits, 15, 16, 1);
        m_display.drawRect(26, -1, 76, 16, 1);

        m_display.display();

        m_currentMenu = DRIVING_MENU;
        m_menuTimer = millis();

        break;
    }

    // Affichage des informations de la batterie : pourcentage de charge et estimation de l'autonomie restante.
    case DRIVING_MENU:
    {
        m_display.clearDisplay();

        m_display.setTextColor(1);
        m_display.setTextWrap(false);
        m_display.setFont(&FreeSans9pt7b);
        m_display.setCursor(15, 12);
        m_display.print("Alimentation");

        m_display.drawRect(13, -1, 103, 16, 1);

        m_display.setCursor(18, 34);
        m_display.print(int(m_battery->getPercentage() * 100.0f));
        m_display.print(F(" %"));

        m_display.setCursor(18, 52);
        m_display.print(m_battery->getRemainingAutonomyHours());
        m_display.print(F("h et "));
        m_display.print(m_battery->getRemainingAutonomyMinutes());
        m_display.print(F("min"));

        m_display.setFont();
        m_display.setCursor(26, 57);
        m_display.print(m_battery->getCapacity());
        m_display.print(F("mAh - "));
        m_display.print(m_battery->getTotalAutonomyHours());
        m_display.print(F("h"));

        m_display.drawBitmap(2, 39, image_timer_bits, 11, 16, 1);
        m_display.drawBitmap(0, 20, image_power_bits, 15, 16, 1);
        m_display.drawBitmap(124, 29, image_next_menu_bits, 4, 7, 1);

        m_display.display();

        m_currentMenu = BATTERY_MENU;
        m_menuTimer = millis();

        break;
    }

    // Affichage du menu principal.
    default:
    {
        m_currentMenu = MAIN_MENU;
        m_menuTimer = millis();

        this->dataUpdated();

        break;
    }
    }
}

/// @brief Méthode à appeler pour mettre à jour l'affichage de l'écran principal. Peut-être utile lorsque l'écran retourne au menu principal, ou qu'une donnée a été mise à jour (heure, vitesse).
void Display::dataUpdated()
{
    if (m_currentMenu != MAIN_MENU)
        return;

    // Si le module GPS n'est pas encore connecté, on affiche le menu de chargement.
    if (!m_connected)
    {
        if (m_GPSModule->isReady())
            m_connected = true;

        else
        {
            m_display.clearDisplay();

            m_display.setTextColor(1);
            m_display.setTextWrap(false);
            m_display.setFont(&FreeSans9pt7b);
            m_display.setCursor(21, 45);
            m_display.print("Connexion");

            m_display.setFont();
            m_display.setCursor(11, 50);
            m_display.print("aux satellites GPS");

            m_display.drawBitmap(104, 0, this->batteryIconChooser(), 24, 16, 1);
            m_display.drawBitmap(52, 4, image_big_timer_bits, 24, 24, 1);
            m_display.drawBitmap(124, 29, image_next_menu_bits, 4, 7, 1);
            m_display.drawBitmap(0, 0, image_no_connection_bits, 15, 16, 1);

            m_display.display();

            return;
        }
    }

    // Affichage du menu principal : vitesse, heure...
    m_display.clearDisplay();

    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans24pt7b);

    // Affichage de la vitesse uniquement si la valeur calculée est récente.
    if (m_GPSModule->isSpeedValueRecent())
    {
        unsigned int speed = m_GPSModule->getSpeed();

        // Sélection du décallage en fonction du nombre de chiffres de la vitesse (pour garder centré le nombre).
        if (speed < 10)
            m_display.setCursor(51, 56);
        else if (speed < 100)
            m_display.setCursor(38, 56);
        else
            m_display.setCursor(25, 56);

        m_display.print(speed);
    }

    // Affichage de traits dans le cas d'une vitesse non mise à jour depuis un certain temps.
    else
    {
        m_display.setCursor(48, 56);
        m_display.print(F("--"));
    }
    
    m_display.drawBitmap(104, 0, this->batteryIconChooser(), 24, 16, 1);
    m_display.drawBitmap(0, 0, image_connected_bits, 15, 16, 1);
    m_display.drawBitmap(124, 29, image_next_menu_bits, 4, 7, 1);
    m_display.drawRect(40, -1, 49, 16, 1);

    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(42, 12);
    m_display.print(addZeros(m_GPSModule->getHour(), 2) + " " + addZeros(m_GPSModule->getMinutes(), 2));

    m_display.display();
}

/// @brief Demande de l'affichage du message de batterie faible pendant un certain temps, avec autonomie restante.
void Display::displayLowBatteryMessage()
{
    unsigned int minutes = m_battery->getRemainingAutonomyMinutes();

    m_display.clearDisplay();

    m_display.drawBitmap(52, 2, this->batteryIconChooser(), 24, 16, 1);

    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(9, 38);
    m_display.print("Batterie faible");

    m_display.setCursor(0, 55);
    m_display.print(minutes);
    m_display.print(F("min restantes"));

    m_display.display();

    m_currentMenu = LOW_BATTERY_MESSAGE;
    m_menuTimer = millis();
}

/// @brief Affichage du message de batterie déchargée.
void Display::displayEmptyBatteryMessage()
{
    m_display.clearDisplay();

    m_display.drawBitmap(52, 2, image_battery_empty_bits, 24, 16, 1);

    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(33, 38);
    m_display.print("Batterie");

    m_display.setCursor(22, 54);
    m_display.print("dechargee");
    delay(m_messageShowTime);

    m_display.clearDisplay();
    m_display.display();
}

/// @brief Méthode permettant d'obtenir le pictogramme adapté au pourcentage restant de batterie.
/// @return Un pointeur vers le pictogramme de batterie plus ou moins chargée.
const unsigned char *Display::batteryIconChooser() const
{
    float percentage = m_battery->getPercentage();

    if (percentage >= 0.83f)
        return image_battery_100_bits;

    if (percentage >= 0.67f)
        return image_battery_83_bits;

    if (percentage >= 0.33f)
        return image_battery_67_bits;

    if (percentage >= 0.17f)
        return image_battery_33_bits;

    if (percentage >= 0.0f)
        return image_battery_17_bits;

    return image_battery_0_bits;
}

/// @brief Méthode gérant l'affichage des ":" de l'heure du menu principal.
/// @param state Affiche ou non les ":".
void Display::displayTimeSeparator(bool state)
{
    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(62, 12);
    m_display.setTextColor(state ? 1 : 0);
    m_display.print(":");
    m_display.display();
}