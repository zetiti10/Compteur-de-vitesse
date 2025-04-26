/**
 * @file battery.cpp
 * @author Louis L
 * @brief Fichier contenant les méthodes de la classe de gestion de la batterie.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>

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
}

/// @brief Initialise l'objet.
void Battery::begin()
{
    pinMode(m_voltagePin, INPUT);
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
}

// Valeurs permettant d'estimer le pourcentage de batterie restante à partir de la tension mesurée.
const float voltageLevels[] = {
    3.0f, 3.2f, 3.3f, 3.4f, 3.45f, 3.5f, 3.55f, 3.6f,
    3.65f, 3.7f, 3.71f, 3.72f, 3.75f, 3.8f, 3.85f, 3.9f,
    3.95f, 4.0f, 4.05f, 4.1f};
const float chargePercentages[] = {
    0.0f, 0.01f, 0.02f, 0.03f, 0.05f, 0.07f, 0.1f, 0.15f,
    0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.5f, 0.6f, 0.7f,
    0.8f, 0.9f, 0.95f, 1.0f};
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