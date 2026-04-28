#include "Dimmer.h"

Dimmer* Dimmer::instance = nullptr;

#define MIN_LED_POWER   0     // หลอดไส้ dim ได้ถึง 0
#define MAX_DELAY_US    9000  // สว่างน้อยสุด (เกือบดับ)
#define MIN_DELAY_US    500   // สว่างเต็ม
#define TRIAC_PULSE_US  500   // หลอดไส้จุดง่าย ไม่ต้องยาว
#define ZC_DEBOUNCE_US  4500

volatile unsigned long lastZcMicros = 0;

Dimmer::Dimmer(uint8_t dim, uint8_t zc) {
  dimPin = dim;
  zcPin = zc;
}

void Dimmer::begin() {
    pinMode(dimPin, OUTPUT);
    digitalWrite(dimPin, LOW);
    pinMode(zcPin, INPUT_PULLUP);

    instance = this;

    attachInterrupt(digitalPinToInterrupt(zcPin), isrWrapper,  CHANGE);
}

void ICACHE_RAM_ATTR Dimmer::isrWrapper() {
  if (instance) {
    instance->onZeroCross();
  }
}

void ICACHE_RAM_ATTR Dimmer::onZeroCross() {
  unsigned long now = micros();

  if (now - lastZcMicros < ZC_DEBOUNCE_US) {
    return;
  }

  lastZcMicros = now;
  zcFlag = true;
}

void Dimmer::setBrightness(int percent) {
  percent = constrain(percent, 0, 100);

  if (percent == 0) {
    brightness = 0;
  } else {
    brightness = constrain(percent, MIN_LED_POWER, 100);
  }
}

void Dimmer::handle() {
  if (!zcFlag) return;

  noInterrupts();
  zcFlag = false;
  int power = brightness;
  interrupts();

  if (power <= 0) {
    digitalWrite(dimPin, LOW);
    return;
  }

  int delayUs = map(power, MIN_LED_POWER, 100, MAX_DELAY_US, MIN_DELAY_US);

  delayMicroseconds(delayUs);

  digitalWrite(dimPin, HIGH);
  delayMicroseconds(TRIAC_PULSE_US);
  digitalWrite(dimPin, LOW);
}