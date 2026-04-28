#ifndef DIMMER_H
#define DIMMER_H

#include <Arduino.h>

class Dimmer {
  private:
    uint8_t dimPin;
    uint8_t zcPin;

    volatile bool zcFlag = false;
    int brightness = 0;

    static Dimmer* instance;
    static void ICACHE_RAM_ATTR isrWrapper();

  public:
    Dimmer(uint8_t dim, uint8_t zc);

    void begin();
    void setBrightness(int percent);
    void handle();

    void ICACHE_RAM_ATTR onZeroCross();
};

#endif