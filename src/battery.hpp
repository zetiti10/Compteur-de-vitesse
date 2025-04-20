#ifndef BATTERY_DEFINITIONS
#define BATTERY_DEFINITIONS

#include <Adafruit_SSD1306.h>

#include "display.hpp"

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

    const unsigned int m_voltagePin;
    const unsigned int m_capacity;
    const unsigned int m_current;
    Display &m_display;
    bool m_lowBatteryMessageDisplayed;
    unsigned long m_lastCheck;
};

#endif