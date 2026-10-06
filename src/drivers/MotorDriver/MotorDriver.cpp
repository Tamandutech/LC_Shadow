#include "MotorDriver.hpp"

#include "Arduino.h"
#include "env.hpp"

MotorDriver::MotorDriver(int directionPin, int pwmPin)
    : directionPin_(directionPin), pwmPin_(pwmPin) {}

void MotorDriver::begin() {
  pinMode(directionPin_, OUTPUT);


  analogWriteResolution(PWM_RESOLUTION_BITS);
  analogWriteFrequency(PWM_FREQUENCY_HZ);

  analogWrite(pwmPin_, 0);
}

void MotorDriver::pwmOutput(int32_t value) {
  // O sinal do valor define o sentido (HIGH = frente, LOW = ré).
  digitalWrite(directionPin_, value >= 0 ? HIGH : LOW);

  // O PWM só entende intensidade (sem sinal), então usamos o valor absoluto,
  int32_t magnitude = value < 0 ? -value : value;
  if(magnitude > MAX_PWM_VALUE) magnitude = MAX_PWM_VALUE;

  analogWrite(pwmPin_, magnitude);
}

VacuumDriver::VacuumDriver(int pwmPin) : pwmPin_(pwmPin) {}

void VacuumDriver::begin() {
  if(pwmPin_ < 0) return; // sucção ainda não ligada

  analogWriteResolution(PWM_RESOLUTION_BITS);
  analogWriteFrequency(PWM_FREQUENCY_HZ);

  analogWrite(pwmPin_, 0);
}

void VacuumDriver::pwmOutput(int32_t value) {
  if(pwmPin_ < 0) return; // sucção ainda não ligada

  int32_t magnitude = value < 0 ? -value : value;
  if(magnitude > MAX_PWM_VALUE) magnitude = MAX_PWM_VALUE;

  analogWrite(pwmPin_, magnitude);
}