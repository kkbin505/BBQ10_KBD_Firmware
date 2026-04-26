#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Adafruit_NeoPixel.h>

class StatusLED {
public:
    static void begin();
    static void update();
    static void setColor(uint8_t r, uint8_t g, uint8_t b);

private:
    static Adafruit_NeoPixel _pixel;
};

#endif
