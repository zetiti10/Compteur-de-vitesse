#ifndef GPS_MODULE_DEFINITIONS
#define GPS_MODULE_DEFINITIONS

#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>

#include "display.hpp"

class GPSModule
{
public:
    GPSModule(UartClass &serial, unsigned long baud, Display &display);

    virtual void begin();
    virtual void loop();
    virtual void sleep();

    virtual bool isReady();
    virtual unsigned int getSpeed();
    virtual unsigned int getMaxSpeed();
    virtual unsigned int getHour();
    virtual unsigned int getMinutes();

protected:
    virtual void decodeData();

    TinyGPSPlus m_GPSDecoder;
    UartClass &m_serial;
    unsigned long m_baud;
    unsigned int m_maxSpeed;
    Display &m_display;
};

#endif