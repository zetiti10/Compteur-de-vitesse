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
#include "utils.hpp"

Battery::Battery(unsigned int voltagePin, unsigned int capacity, unsigned int current, Display &display) : m_voltagePin(voltagePin), m_capacity(capacity), m_current(current), m_display(display), m_lowBatteryMessageDisplayed(false), m_lastCheck(0) {}

void Battery::begin()
{
    pinMode(m_voltagePin, INPUT);
}

void Battery::startupCheck()
{
    float percentage = this->getPercentage();

    if (percentage <= 0.05f)
    {
        this->shutdown();
    }
}

void Battery::loopCheck()
{
    if ((millis() - m_lastCheck) >= 1000)
    {
        m_lastCheck = millis();

        float percentage = this->getPercentage();

        if (percentage <= 0.00f)
            this->shutdown();

        if (!m_lowBatteryMessageDisplayed && this->getRemainingAutonomyHours() == 0 && this->getRemainingAutonomyMinutes() <= 30)
        {
            m_display.displayLowBatteryMessage(percentage, this->getRemainingAutonomyMinutes());
            m_lowBatteryMessageDisplayed = true;
        }
    }
}

// TODO Faire un meilleur système car la décharge n'est pas linéaire !
// TODO Pourquoi ne pas enlever les deux décimales qui ne sont pas significatives ?
// TODO Peut-être faire un système de pourcentage plus stable, basé sur une ensemble de valeurs pour éviter la chute de tension occasionnelle ?
// TODO Quelles bornes mettre ? 3V et 4.2V ou une plage plus réduite pour limiter ne pas trop abimer la batterie ?
const float Battery::getPercentage()
{
    float voltage = (float(analogRead(m_voltagePin)) / 1024.0f) * 5.0f;
    float percentage = mapFloat(voltage, 3.0f, 4.2f, 0.0f, 1.0f);
    return percentage;
}

const unsigned int Battery::getTotalAutonomyHours()
{
    return int(m_capacity / m_current);
}

const unsigned int Battery::getCapacity()
{
    return m_capacity;
}

const unsigned int Battery::getRemainingAutonomyHours()
{
    float autonomyInHours = (m_capacity / m_current) * this->getPercentage();
    return int(autonomyInHours);
}

const unsigned int Battery::getRemainingAutonomyMinutes()
{
    float autonomyInHours = (m_capacity / m_current) * this->getPercentage();
    unsigned int autonomyHours = int(autonomyInHours);
    return int((autonomyInHours - float(autonomyHours)) * 60.0f);
}

void Battery::shutdown()
{
    m_display.displayEmptyBatteryMessage();
    // TODO Désactiver le reste du système

    while (1)
    {
    }
}
