#ifndef BLUETOOTH_BLE_HPP
#define BLUETOOTH_BLE_HPP

#include <Arduino.h>

// -----------------------------------------------------------------------------
// BluetoothBLE
// -----------------------------------------------------------------------------

#define NuSerial Serial1

// Inicializa a UART do Bluetooth com o baud rate informado (ex: BT_BAUD).
// Chamar no setup().
void bluetoothBegin(unsigned long baud);

#endif