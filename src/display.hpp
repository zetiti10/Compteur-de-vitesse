#ifndef DISPLAY_DEFINITIONS
#define DISPLAY_DEFINITIONS

#include <Adafruit_SSD1306.h>

class Battery;
class GPSModule;

enum Menu
{
    MAIN_MENU,
    DRIVING_MENU,
    BATTERY_MENU,
    LOW_BATTERY_MESSAGE,
};

class Display
{
public:
    Display(unsigned int busAddress, unsigned int messageShowTime);
    virtual void setDevices(Battery *battery, GPSModule *gpsModule);

    virtual void begin();
    virtual void loop();

    virtual void displayStartupMessage();
    virtual void displayNextMenu();
    virtual void dataUpdated();
    virtual void displayLowBatteryMessage();
    virtual void displayEmptyBatteryMessage();

protected:
    virtual const unsigned char *batteryIconChooser() const;
    virtual void displayTimeSeparator(bool state);

    Adafruit_SSD1306 m_display;
    const unsigned int m_address;
    Menu m_currentMenu;
    Battery *m_battery;
    GPSModule *m_GPSModule;
    const unsigned int m_messageShowTime;
    unsigned long m_menuTimer;
    bool m_connected;
    bool m_timeSeparatorPhase;
};

#endif