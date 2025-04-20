/**
 * @file display.cpp
 * @author Louis L
 * @brief Fichier contenant les méthodes de la classe de gestion de l'écran.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>
#include <fonts/FreeSans9pt7b.h>
#include <fonts/FreeSans24pt7b.h>

// Autres fichiers du programme
#include "display.hpp"
#include "GPSModule.hpp"
#include "battery.hpp"
#include "bitmaps.hpp"
#include "utils.hpp"

Display::Display(unsigned int busAddress, unsigned int messageShowTime) : m_display(128, 64, &Wire, -1), m_address(busAddress), m_currentMenu(MAIN_MENU), m_battery(nullptr), m_GPSModule(nullptr), m_messageShowTime(messageShowTime), m_menuTimer(0), m_connected(false), m_lastSpeedRecord(0) {}

void Display::setDevices(Battery *battery, GPSModule *gpsModule)
{
    m_battery = battery;
    m_GPSModule = gpsModule;
}

void Display::begin()
{
    m_display.begin(SSD1306_SWITCHCAPVCC, m_address);

    m_display.clearDisplay();
    m_display.display();
}

void Display::loop()
{
    if (m_currentMenu == DRIVING_MENU || m_currentMenu == BATTERY_MENU)
    {
        if ((millis() - m_menuTimer) >= m_messageShowTime)
        {
            m_currentMenu = MAIN_MENU;
            m_menuTimer = millis();

            this->dataUpdated();
        }
    }

    if (m_currentMenu == MAIN_MENU && m_connected)
    {
        if ((millis() - m_lastSpeedRecord) <= 2000)
        {
            unsigned int speed = 500;
            if ((millis() % speed) < (speed / 2))
                m_display.drawBitmap(13, 21, image_speed_record_bits, 109, 37, 1);

            else
                m_display.drawBitmap(13, 21, image_speed_record_bits, 109, 37, 0);
        }

        unsigned int speed = 1000;
        m_display.setTextColor(1);
        m_display.setTextWrap(false);
        m_display.setFont(&FreeSans9pt7b);
        m_display.setCursor(62, 12);
        if ((millis() % speed) < (speed / 2)) // TODO Refaire un système plus optimisé qui n'écrit sur l'écran que lorsque c'est nécessaire !
            m_display.setTextColor(1);

        else
            m_display.setTextColor(0);
        m_display.print(":");
        m_display.display();
    }
}

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

    delay(3000);

    this->dataUpdated();
}

void Display::displayNextMenu()
{
    switch (m_currentMenu)
    {
    case MAIN_MENU:
    {
        if (!m_connected)
            break;

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

    default:
    {
        m_currentMenu = MAIN_MENU;
        m_menuTimer = millis();

        this->dataUpdated();

        break;
    }
    }
}

void Display::dataUpdated()
{
    if (m_currentMenu != MAIN_MENU)
        return;

    if (!m_connected)
    {
        if (m_GPSModule->isReady())
        {
            m_connected = true;
        }

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

            m_display.drawBitmap(104, 0, this->batteryIconChooser(m_battery->getPercentage()), 24, 16, 1);
            m_display.drawBitmap(52, 4, image_big_timer_bits, 24, 24, 1);
            m_display.drawBitmap(124, 29, image_next_menu_bits, 4, 7, 1);
            m_display.drawBitmap(0, 0, image_no_connection_bits, 15, 16, 1); // TODO Faire un système de perte de connexion ?

            m_display.display();

            return;
        }
    }

    m_display.clearDisplay();

    m_display.setTextColor(1);
    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans24pt7b);
    unsigned int speed = m_GPSModule->getSpeed();
    if (speed < 10)
        m_display.setCursor(51, 56);
    else if (speed < 100)
        m_display.setCursor(38, 56);
    else
        m_display.setCursor(25, 56);
    m_display.print(speed);

    m_display.drawBitmap(104, 0, this->batteryIconChooser(m_battery->getPercentage()), 24, 16, 1);
    m_display.drawBitmap(0, 0, image_connected_bits, 15, 16, 1);
    m_display.drawBitmap(124, 29, image_next_menu_bits, 4, 7, 1);
    m_display.drawRect(40, -1, 49, 16, 1);

    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(42, 12);
    m_display.print(addZeros(m_GPSModule->getHour(), 2) + " " + addZeros(m_GPSModule->getMinutes(), 2));

    m_display.display();
}

/*void Display::displayAutonomy(float percentage, unsigned int hour, unsigned int minutes)
{
    m_display.clearDisplay();

    m_display.setTextColor(1);
    m_display.setTextWrap(false);
    m_display.setFont(&FreeSans9pt7b);
    m_display.setCursor(34, 17);
    m_display.print("Autonomie");

    m_display.setCursor(12, 40);
    m_display.print(m_battery->getRemainingAutonomyHours());
    m_display.print(F("h et "));
    m_display.print(m_battery->getRemainingAutonomyMinutes());
    m_display.print(F("min"));

    m_display.drawBitmap(9, 4, image_info_bits, 15, 16, 1);
    m_display.drawBitmap(52, 47, this->batteryIconChooser(percentage), 24, 16, 1);

    m_display.display();

    m_currentMenu = AUTONOMY_MESSAGE;
    m_menuTimer = millis();
}*/

void Display::displayLowBatteryMessage(float percentage, unsigned int minutes)
{
    m_display.clearDisplay();

    m_display.drawBitmap(52, 2, this->batteryIconChooser(percentage), 24, 16, 1);

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

    m_display.display();

    delay(m_messageShowTime);

    m_display.clearDisplay();
    m_display.display();
}

void Display::triggerSpeedRecord()
{
    m_lastSpeedRecord = millis();
}

const unsigned char *Display::batteryIconChooser(float percentage) const
{
    if (percentage >= 0.95f)
        return image_battery_100_bits;

    if (percentage >= 0.83f)
        return image_battery_83_bits;

    if (percentage >= 0.67f)
        return image_battery_67_bits;

    if (percentage >= 0.33f)
        return image_battery_33_bits;

    if (percentage >= 0.17f)
        return image_battery_17_bits;

    return image_battery_0_bits;
}
