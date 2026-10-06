#ifndef BLUETOOTH_BLE_HPP

#define BLUETOOTH_BLE_HPP

#include <Arduino.h>

extern HardwareSerial &NuSerial;
void                   bluetoothBegin(unsigned long baud);

#endif