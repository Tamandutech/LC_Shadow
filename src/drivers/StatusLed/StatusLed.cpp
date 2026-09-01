#include "StatusLed.hpp"

#include "Arduino.h"

StatusLed::StatusLed(int pin) : pin_(pin), ledState_(false), lastToggleMs_(0) {}

void StatusLed::begin() {
  pinMode(pin_, OUTPUT);
  off();
}

void StatusLed::on() {
  digitalWrite(pin_, HIGH);
  ledState_ = true;
}

void StatusLed::off() {
  digitalWrite(pin_, LOW);
  ledState_ = false;
}

void StatusLed::blinkBlocking(int times, unsigned long onMs,
                              unsigned long offMs) {
  for(int i = 0; i < times; i++) {
    on();
    delay(onMs);
    off();
    delay(offMs);
  }
}

void StatusLed::updateBlinking(unsigned long intervalMs) {
  unsigned long now = millis();
  if(now - lastToggleMs_ >= intervalMs) {
    ledState_ ? off() : on();
    lastToggleMs_ = now;
  }
}