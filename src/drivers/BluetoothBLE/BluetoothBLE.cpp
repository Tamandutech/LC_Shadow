#include "BluetoothBLE.hpp"

#include "env.hpp"

HardwareSerial &NuSerial = bluetoothSerial;

void bluetoothBegin(unsigned long baud) { bluetoothSerial.begin(baud); }