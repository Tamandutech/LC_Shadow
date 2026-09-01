#include "MotorDriver.hpp"

#include "Arduino.h"
#include "env.hpp"

MotorDriver::MotorDriver(int directionPin, int pwmPin, uint8_t pwmChannel)
    : directionPin_(directionPin), pwmPin_(pwmPin), pwmChannel_(pwmChannel) {
  pinMode(directionPin_, OUTPUT);

  // Configura o canal de PWM (frequência + resolução) e liga ele ao pino.
  ledcSetup(pwmChannel_, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcAttachPin(pwmPin_, pwmChannel_);
}

void MotorDriver::pwmOutput(int32_t value) {
  // O sinal do valor define o sentido (HIGH = frente, LOW = ré).
  digitalWrite(directionPin_, value >= 0 ? HIGH : LOW);

  // O PWM só entende intensidade (sem sinal).
  int32_t magnitude = value < 0 ? -value : value;

  // Limita a velocidade entre 0% e 100%.
  if(magnitude > 100) magnitude = 100;

  // Converte porcentagem (0-100%) para PWM (0-255).
  int32_t pwmValue = (magnitude * MAX_PWM_VALUE) / 100;

  ledcWrite(pwmChannel_, pwmValue);
}
