#ifndef BLUETOOTH_BLE_HPP
#define BLUETOOTH_BLE_HPP

#include <Arduino.h>

// -----------------------------------------------------------------------------
// BluetoothBLE
// -----------------------------------------------------------------------------

#ifndef BT_AUTOBAUD
#define BT_AUTOBAUD 1
#endif

class BluetoothPort : public Stream {
public:
  // Em modo BT_AUTOBAUD, 'baud' é o PRIMEIRO baud tentado.
  void begin(unsigned long baud);

  int    available();
  int    read();
  int    peek();
  size_t write(uint8_t byte);
  size_t write(const uint8_t *buffer, size_t size);
  void   flush();
  using Print::write;

  bool          isLocked() const { return locked_; }
  unsigned long currentBaud() const { return currentBaud_; }

private:
  void service();
  bool lockIfCommand();
  void nextBaud();

  bool          locked_      = true;
  unsigned long baseBaud_    = 0;
  unsigned long currentBaud_ = 0;
  int           otherIdx_    = -1; // -1 = baud base; >= 0 = índice na lista
  unsigned long switchedAt_  = 0;
  unsigned long lastByteAt_  = 0;

  char    line_[24];
  uint8_t lineLen_ = 0;

  // Linha que provocou o travamento, devolvida ao main.cpp como se tivesse
  // acabado de chegar (pra esse primeiro comando não se perder).
  char    push_[24];
  uint8_t pushLen_ = 0;
  uint8_t pushPos_ = 0;
};

// Porta serial do Bluetooth (definida em BluetoothBLE.cpp).
extern BluetoothPort NuSerial;

// Inicializa a UART do Bluetooth. Chamar no setup().
void bluetoothBegin(unsigned long baud);

#endif