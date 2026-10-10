#ifndef BLUETOOTH_BLE_HPP
#define BLUETOOTH_BLE_HPP

#include <Arduino.h>

// -----------------------------------------------------------------------------
// BluetoothBLE
// -----------------------------------------------------------------------------

class BLE {
public:
  static const char &lastCharacter;

  // AJUSTADO: Agora aceita o baudrate enviado pelo main.cpp
  static void initialize(uint32_t baudrate);
  static void start();
  static void restart();
  static void setup();
  static void resetLastCharacter();
};

#endif
