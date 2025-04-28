#ifndef BATTERY_DEFINITIONS
#define BATTERY_DEFINITIONS

#include "display.hpp"
#include "GPSModule.hpp"

#define BATTERY_BUFFER_SIZE 100

// Si cet élément est défini, le système enregistrera les valeurs de tension de la batterie à intervalles régulières pour pouvoir par la suite les récupérer sur un ordinateur et ajuster le calcul d'estimation du pourcentage restant de la batterie.
// L'enregistrement ne se fait que si aucune donnée n'a déjà été écrite à la première minute. La lecture des données efface les valeurs.
// Pour enregistrer la sortie série sur l'ordinateur, utiliser PuTTY en configurant l'enregistrement de la session et taper un caractère lorsque le port série est ouvert.
// Un script Python génère les associations tension/pourcentage qui sont intégrables dans le programme Arduino.
//#define MEASURE_BATTERY
// L'intervale des enregistrements des tensions en secondes. Il est à noter que l'EEPROM de la carte est limité à 256 octets.
#define MEASURE_INTERVAL 240

class Battery
{
public:
    Battery(unsigned int voltagePin, unsigned int capacity, unsigned int current, Display &display);

    virtual void begin();
    virtual void startupCheck();
    virtual void loopCheck();

    const virtual float getPercentage();
    const virtual unsigned int getTotalAutonomyHours();
    const virtual unsigned int getCapacity();
    const virtual unsigned int getRemainingAutonomyHours();
    const virtual unsigned int getRemainingAutonomyMinutes();

protected:
    virtual void shutdown();
    float calculateAverageVoltage();

    const unsigned int m_voltagePin;
    const unsigned int m_capacity;
    const unsigned int m_current;
    Display &m_display;
    bool m_lowBatteryMessageDisplayed;
    unsigned long m_lastCheck;
    float m_voltageBuffer[BATTERY_BUFFER_SIZE];
    unsigned int m_bufferIndex;
    unsigned int m_bufferCount;

#ifdef MEASURE_BATTERY
    unsigned int m_lastValueRegistered;
    bool m_recording;
#endif
};

#endif