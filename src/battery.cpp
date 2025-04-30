/**
 * @file battery.cpp
 * @author Louis L
 * @brief Fichier contenant les méthodes de la classe de gestion de la batterie.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>
#include <EEPROM.h>

// Autres fichiers du programme
#include "battery.hpp"
#include "display.hpp"
#include "GPSModule.hpp"
#include "utils.hpp"

/// @brief Instancie la classe gérant la batterie du système.
Battery::Battery(unsigned int voltagePin, unsigned int capacity, unsigned int current, Display &display) : m_voltagePin(voltagePin), m_capacity(capacity), m_current(current), m_display(display), m_lowBatteryMessageDisplayed(false), m_lastCheck(0), m_bufferIndex(0), m_bufferCount(0)
{
    for (int i = 0; i < BATTERY_BUFFER_SIZE; i++)
        m_voltageBuffer[i] = 3.7f;

#ifdef MEASURE_BATTERY
    m_lastValueRegistered = 0;
    m_recording = false;
#endif
}

/// @brief Initialise l'objet.
void Battery::begin()
{
    pinMode(m_voltagePin, INPUT);

    // Mise en place de la communication dans le cas de la mesure de la batterie.
#ifdef MEASURE_BATTERY
    Serial.begin(115200);
    pinMode(13, OUTPUT);

    if (EEPROM.read(2) == 0)
    {
        digitalWrite(13, HIGH);
        m_recording = true;
        delay(500);
        digitalWrite(13, LOW);
    }
#endif
}

/// @brief Vérifie que la batterie n'est pas déchargée.
void Battery::startupCheck()
{
    float percentage = this->getPercentage();

    if (percentage <= 0.00f)
    {
        this->shutdown();
    }
}

/// @brief Vérifie le pourcentage de charge de la batterie restante, et effectue des actions en conséquence.
void Battery::loopCheck()
{
    if ((millis() - m_lastCheck) >= 1000)
    {
        m_lastCheck = millis();

        float percentage = this->getPercentage();

        // Éteint le système si la batterie est déchargée.
        if (percentage <= 0.00f)
            this->shutdown();

        // Affiche le message de batterie faible.
        if (!m_lowBatteryMessageDisplayed && this->getRemainingAutonomyHours() == 0 && this->getRemainingAutonomyMinutes() <= 30)
        {
            m_display.displayLowBatteryMessage();
            m_lowBatteryMessageDisplayed = true;
        }
    }

    // Mesure de la batterie.
#ifdef MEASURE_BATTERY
    // Envoi des données à la connexion à un ordinateur.
    if (Serial.available())
    {
        digitalWrite(13, HIGH);
        delay(1000);
        for (int i = 0; i < 255; i++)
        {
            // Arrêt de l'envoi lorsque toutes les données ont été envoyées.
            unsigned int value = EEPROM.read(i);
            if (value == 0 && i > 12)
                break;

            // Retour à un float puis envoi de la tension.
            float voltage = float(value) / 10.0f;
            int time = i * MEASURE_INTERVAL;
            Serial.print(time);
            Serial.print(F(","));
            Serial.println(voltage, 1);
        }

        // Réinitialisation de l'EEPROM une fois la lecture faite.
        for (int i = 0; i < 255; i++)
            EEPROM.update(i, 0);
        digitalWrite(13, LOW);

        while (true)
            delay(1);
    }

    if (!m_recording)
        return;

    unsigned int currentValue = int(millis() / (1000UL * MEASURE_INTERVAL));
    if (currentValue > m_lastValueRegistered)
    {
        m_lastValueRegistered = currentValue;

        if (currentValue >= 255)
            return;

        digitalWrite(13, HIGH);
        unsigned int voltage = int(this->calculateAverageVoltage() * 10.0f);
        EEPROM.update(currentValue, voltage);
        delay(100);
        digitalWrite(13, LOW);
    }
#endif
}

// Valeurs permettant d'estimer le pourcentage de batterie restante à partir de la tension mesurée.
const float voltageLevels[] = {
    3.20f, 3.24f, 3.28f, 3.33f, 3.37f, 3.41f, 3.45f, 3.49f, 3.54f, 3.58f, 3.62f, 3.66f, 3.71f, 3.75f, 3.79f, 3.83f, 3.87f, 3.92f, 3.96f, 4.00f};
const float chargePercentages[] = {
    0.00f, 0.05f, 0.11f, 0.16f, 0.21f, 0.26f, 0.32f, 0.37f, 0.42f, 0.47f, 0.53f, 0.58f, 0.63f, 0.68f, 0.74f, 0.79f, 0.84f, 0.89f, 0.95f, 1.00f};
const int numberOfVoltageValues = 20;

/// @brief Permet de calculer le pourcentage de batterie restante basé sur un ensemble de mesures de la tension de la batterie.
/// @return L'estimation du pourcentage de batterie restante.
const float Battery::getPercentage()
{
    // Lecture de la tension aux bornes de la batterie.
    float voltage = (float(analogRead(m_voltagePin)) / 1024.0f) * 5.0f;

    // Mise à jour du buffer circulaire.
    m_voltageBuffer[m_bufferIndex] = voltage;
    m_bufferIndex = (m_bufferIndex + 1) % BATTERY_BUFFER_SIZE;
    if (m_bufferCount < BATTERY_BUFFER_SIZE)
        m_bufferCount++;

    // Calcul de la moyenne des tensions.
    float averageVoltage = calculateAverageVoltage();

    // Calcul du pourcentage en fonction de la tension moyenne de la batterie et des valeurs ci-dessus.
    for (int i = 0; i < numberOfVoltageValues - 1; i++)
    {
        if (averageVoltage <= voltageLevels[i + 1])
        {
            // Effectue une sorte de `map` entre les deux paliers les plus proches définis dans les tableaux ci-dessus.
            float voltageDiff = voltageLevels[i + 1] - voltageLevels[i];
            float chargeDiff = chargePercentages[i + 1] - chargePercentages[i];
            float ratio = (averageVoltage - voltageLevels[i]) / voltageDiff;
            int percentage = int((chargePercentages[i] + ratio * chargeDiff) * 100.0f);
            return (float(percentage) / 100.0f);
        }
    }

    return 1.0f;
}

/// @brief Méthode permettant de récupérer l'autonomie totale de la batterie.
/// @return L'autonomie totale de la batterie.
const unsigned int Battery::getTotalAutonomyHours()
{
    return int(m_capacity / m_current);
}

/// @brief Méthode permettant de récupérer la capacité de la batterie.
/// @return La capacité de la batterie en `mAh`.
const unsigned int Battery::getCapacity()
{
    return m_capacity;
}

/// @brief Méthode permettant de récupérer la composante heure de l'estimation de l'autonomie restante.
/// @return
const unsigned int Battery::getRemainingAutonomyHours()
{
    float autonomyInHours = (m_capacity / m_current) * this->getPercentage();
    return int(autonomyInHours);
}

/// @brief Méthode permettant de récupérer la composante minutes de l'estimation de l'autonomie restante.
const unsigned int Battery::getRemainingAutonomyMinutes()
{
    float autonomyInHours = (m_capacity / m_current) * this->getPercentage();
    unsigned int autonomyHours = int(autonomyInHours);
    return int((autonomyInHours - float(autonomyHours)) * 60.0f);
}

/// @brief Éteint le système après avoir affiché un message de batterie vide.
void Battery::shutdown()
{
    m_display.displayEmptyBatteryMessage();

    while (1)
    {
    }
}

/// @brief Méthode calculant la tension moyenne d'un ensemble de mesures de la tension aux bornes de la batterie.
/// @return La tension moyenne de la batterie.
float Battery::calculateAverageVoltage()
{
    float sum = 0.0f;
    for (unsigned int i = 0; i < m_bufferCount; i++)
        sum += m_voltageBuffer[i];

    return sum / m_bufferCount;
}