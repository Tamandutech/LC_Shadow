#include "BluetoothBLE.hpp"
#include "env.hpp"

static uint8_t rx_buffer[32]  = {0};
static char    lastCharacter_ = ' ';

const char &BLE::lastCharacter = lastCharacter_;

// ADICIONADO: Definição da função que estava faltando
void BLE::initialize(uint32_t baudrate) { NuSerial.begin(baudrate); }

void BLE::setup() {
  const uint32_t timeout     = 500; // ms
  const char    *start       = "AT\r\n";
  const char    *setName     = "AT+NAME"
                               "TT_SHADOW"
                               "\r\n";
  const char    *setPin      = "AT+PIN" BLE_PIN "\r\n";
  const char    *setBaudRate = "AT+BAUD"
                               "9"
                               "\r\n";

  HAL_Delay(5000);
  NuSerial.print(start);

  // (I) Set name
  HAL_Delay(1000);
  NuSerial.print(setName);

  // (II) Set pin
  HAL_Delay(1000);
  NuSerial.print(setPin);

  // (III) Set baud rate
  HAL_Delay(1000);
  NuSerial.print(setBaudRate);

  HAL_Delay(1000);
}

// Se o compilador reclamar das outras funções vazias, declare-as assim:
void BLE::start() {}
void BLE::restart() {}
void BLE::resetLastCharacter() {}
