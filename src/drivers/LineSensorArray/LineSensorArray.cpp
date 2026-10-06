#include "LineSensorArray.hpp"

#include "Arduino.h"
#include "env.hpp"

// Pino analógico de cada sensor, da esquerda pra direita (env.hpp).
static const int kLineSensorPins[NUM_LINE_SENSORS] = GPIO_LINE_SENSORS;

void LineSensorArray::begin() {
  for(uint8_t i = 0; i < NUM_LINE_SENSORS; i++) {
    pinMode(kLineSensorPins[i], INPUT_ANALOG);
  }
}

std::array<int32_t, NUM_LINE_SENSORS> LineSensorArray::readAll() {
  std::array<int32_t, NUM_LINE_SENSORS> values{};

  for(uint8_t i = 0; i < NUM_LINE_SENSORS; i++) {
    values[i] = analogRead(kLineSensorPins[i]);
  }

  return values;
}
