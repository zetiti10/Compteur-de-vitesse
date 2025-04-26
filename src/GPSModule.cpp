/**
 * @file GPSModule.cpp
 * @author Louis L
 * @brief Fichier contenant les méthodes de la classe de gestion du module GPS NEO-6M.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>
#include <TinyGPSPlus.h>

// Autres fichiers du programme
#include "GPSModule.hpp"
#include "display.hpp"

/// @brief Instancie la classe gérant la communication avec le module GPS.
GPSModule::GPSModule(UartClass &serial, unsigned long baud, Display &display) : m_GPSDecoder(), m_serial(serial), m_baud(baud), m_display(display) {}

/// @brief Initialise la communication avec le module GPS.
void GPSModule::begin()
{
    m_serial.begin(m_baud);
}

/// @brief Lis les données du module GPS et informe l'objet écran des mises à jours.
void GPSModule::loop()
{
    this->decodeData();

    if (!this->isReady())
        return;
    if (m_GPSDecoder.speed.isUpdated() || m_GPSDecoder.time.isUpdated())
    {
        // Envoi de la mise à jour d'une information à l'écran.
        m_display.dataUpdated();

        // Mise à jour de la vitesse maximale enregistrée.
        if (m_GPSDecoder.speed.kmph() > m_maxSpeed)
            m_maxSpeed = m_GPSDecoder.speed.kmph();
    }
}

/// @brief Permet de savoir si le module GPS a récupéré des informations des satelittes GPS.
/// @return Renvoie `true` si le module GPS est connecté.
bool GPSModule::isReady()
{
    this->decodeData();

    return m_GPSDecoder.speed.isValid();
}

/// @brief Méthode permettant d'obtenir la vitesse actuelle calculée à partir des coordonnées GPS.
/// @return La vitesse actuelle (`0` si non connecté).
unsigned int GPSModule::getSpeed()
{
    this->decodeData();

    return m_GPSDecoder.speed.kmph();
}

/// @brief Permet de savoir si la valeur de la vitesse actuelle a été récupérée récemment.
/// @return Renvoie `true` si la vitesse connue actuelle date de moins de cinq secondes.
bool GPSModule::isSpeedValueRecent()
{
    this->decodeData();

    if (m_GPSDecoder.speed.age() > 5000)
        return false;

    return true;
}

/// @brief Méthode permettant de récupérer la vitesse maximale enregistrée depuis le démarrage du programme.
/// @return La vitesse maximale.
unsigned int GPSModule::getMaxSpeed()
{
    return m_maxSpeed;
}

/// @brief Méthode permettant de récupérer la composante heure du temps actuel au décalage horaire de Paris.
/// @return L'heure actuelle (`0` si l'heure est inconnue).
unsigned int GPSModule::getHour()
{
    this->decodeData();

    // Renvoie `0` si le temps ou la date n'est pas connue (nécessaire pour appliquer l'heure d'été).
    if (!m_GPSDecoder.date.isValid() || !m_GPSDecoder.time.isValid())
        return 0;

    // Calcul du décalage horaire prennant en compte l'heure d'été.
    unsigned int offset = 1;
    unsigned int year = m_GPSDecoder.date.year();
    unsigned int month = m_GPSDecoder.date.month();
    unsigned int day = m_GPSDecoder.date.day();
    unsigned int hour = m_GPSDecoder.time.hour();
    // Heure d'été après le dernier dimanche de mars.
    if (month == 3)
    {
        byte lastMarchSunday = 31 - ((5 + year + (year >> 2)) % 7);
        if (day > lastMarchSunday || (day == lastMarchSunday && hour != 0))
            offset = 2;
    }

    // Heure d'été avant le dernier dimanche d'octobre.
    else if (month == 10)
    {
        byte lastOctoberSunday = 31 - ((2 + year + (year >> 2)) % 7);
        if (day < lastOctoberSunday || (day == lastOctoberSunday && hour == 0))
            offset = 2;
    }

    // Heure d'été d'avril à septembre.
    else if (month > 3 && month < 10)
        offset = 2;

    return (hour + offset);
}

/// @brief Méthode permettant de récupérer la composante minutes du temps actuel au décalage horaire de Paris.
/// @return Les minutes actuelles (`0` si le temps est inconnu).
unsigned int GPSModule::getMinutes()
{
    this->decodeData();

    if (!m_GPSDecoder.date.isValid() || !m_GPSDecoder.time.isValid())
        return 0;

    return m_GPSDecoder.time.minute();
}

/// @brief Méthode interne permettant de traiter les informations reçues par le module GPS.
void GPSModule::decodeData()
{
    while (m_serial.available())
        m_GPSDecoder.encode(m_serial.read());
}
