#ifndef BATTERY_DEFINITIONS
#define BATTERY_DEFINITIONS

#include "display.hpp"
#include "GPSModule.hpp"

// Nombre de valeurs prises pour faire une moyenne des tensions de la batterie.
#define BATTERY_BUFFER_SIZE 100

// La définition de cet élément permet d'entrer en mode d'enregistrement des données pour calibrer l'estimation du pourcentage de batterie restante en fonction de la tension de la batterie. Les données sont traitées par le programme dans `tools/decoder/dataDecoder.py`.
// #define MEASURE_BATTERY
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