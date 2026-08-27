#ifndef MOTOR_DRIVER_HPP
#define MOTOR_DRIVER_HPP

#include <cstdint>

class MotorDriver {
public:
  MotorDriver();

  void    pwmOutput(int32_t value);
  int32_t getPulse();
};

#endif // MOTOR_DRIVER_HPP
