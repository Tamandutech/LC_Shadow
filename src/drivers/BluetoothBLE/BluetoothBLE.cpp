#include "BluetoothBLE.hpp"

#include "env.hpp"

void bluetoothBegin(unsigned long baud) {
  // O baud rate precisa ser IGUAL ao configurado no módulo Bluetooth.
  NuSerial.begin(baud);
}