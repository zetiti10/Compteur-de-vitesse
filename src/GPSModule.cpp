/**
 * @file GPSModule.cpp
 * @author Louis L
 * @brief Fichier contenant les méthodes de la classe de gestion du module GPS NEO-6M.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>

// Autres fichiers du programme
#include "GPSModule.hpp"

GPSModule::GPSModule(UartClass &serial, unsigned long baud, Display &display) : m_GPSDecoder(), m_serial(serial), m_baud(baud), m_display(display) {}

void GPSModule::begin()
{
    m_serial.begin(m_baud);
}

void GPSModule::loop()
{
    this->decodeData();

    if (m_GPSDecoder.speed.isUpdated() || m_GPSDecoder.time.isUpdated())
    {
        m_display.dataUpdated();

        if (m_GPSDecoder.speed.kmph() > m_maxSpeed)
        {
            m_maxSpeed = m_GPSDecoder.speed.kmph();
            m_display.triggerSpeedRecord();
        }
    }
}

void GPSModule::sleep()
{
    // TODO Que faire ?
}

bool GPSModule::isReady()
{
    this->decodeData();

    return m_GPSDecoder.speed.isValid();
}

unsigned int GPSModule::getSpeed()
{
    this->decodeData();

    return m_GPSDecoder.speed.kmph();
}

unsigned int GPSModule::getMaxSpeed()
{
    return m_maxSpeed;
}

unsigned int GPSModule::getHour()
{
    this->decodeData();

    unsigned int offset = 1;
    // En France métropolitaine :
    // Passage de l'heure d'hiver à l'heure d'été le dernier dimanche de mars à 1h00 UTC (à 2h00 locales il est 3h00)
    // Passage de l'heure d'été à l'heure d'hiver le dernier dimanche d'octobre à 1h00 UTC (à 3h00 locales il est 2h00) TODO Enlever après vérifications !
    unsigned int year = m_GPSDecoder.date.year();
    unsigned int month = m_GPSDecoder.date.month();
    unsigned int day = m_GPSDecoder.date.day();
    unsigned int hour = m_GPSDecoder.time.hour();
    if (month == 3)
    {
        byte lastMarchSunday = 31 - ((5 + year + (year >> 2)) % 7);
        if (day > lastMarchSunday || (day == lastMarchSunday && hour != 0))
            offset = 2;
    }

    else if (month == 10)
    {
        byte lastOctoberSunday = 31 - ((2 + year + (year >> 2)) % 7);
        if (day < lastOctoberSunday || (day == lastOctoberSunday && hour == 0))
            offset = 2;
    }
    
    else if (month > 3 && month < 10)
        offset = 2;

    return (hour + offset);
}

unsigned int GPSModule::getMinutes()
{
    this->decodeData();

    return m_GPSDecoder.time.minute();
}

void GPSModule::decodeData()
{
    while (m_serial.available())
        m_GPSDecoder.encode(m_serial.read());
}
