#ifndef MOTOR_DRIVER_HPP
#define MOTOR_DRIVER_HPP

#include <cstdint>

// -----------------------------------------------------------------------------
// MotorDriver
// -----------------------------------------------------------------------------
// Controla UM motor: 1 pino diz o SENTIDO de giro (direção), o outro controla a
// VELOCIDADE via PWM. Para o Shadow, que tem 2 motores, cria duas
// instâncias (uma pra esquerda, outra pra direita) em main.cpp.
// -----------------------------------------------------------------------------

class MotorDriver {
public:
  // directionPin: pino digital que define o sentido de giro (HIGH/LOW).
  // pwmPin: pino que recebe o sinal PWM (controla a velocidade).
  // pwmChannel: canal do periférico LEDC do ESP32 (0 a 7 disponíveis).
  // Cada motor precisa de um canal DIFERENTE (ex: esquerda=0, direita=1).
  MotorDriver(int directionPin, int pwmPin, uint8_t pwmChannel);

  // Aplica uma velocidade ao motor.
  // value: de -maxPwm a +maxPwm. Sinal define o sentido (positivo = frente,
  // negativo = ré), o valor absoluto define a intensidade do PWM.
  void pwmOutput(int32_t value);

private:
  int     directionPin_;
  int     pwmPin_;
  uint8_t pwmChannel_;
};

#endif
