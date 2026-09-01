#ifndef BluetoothBLE_HPP
#define BluetoothBLE_HPP

#include "NimBLEDevice.h"

// -----------------------------------------------------------------------------
// Bluetooth
// -----------------------------------------------------------------------------
// Controla o robô via Bluetooth Low Energy (BLE), usando a biblioteca
// NimBLE-Arduino.
// O robô liga, fica anunciando (advertising, é como o celular reconhece o robô
// no bluetooth) e espera comandos remotos antes de fazer qualquer coisa
// sozinho.
//
// O que acontece em main.cpp:
//   1. setup() chama ble.begin(...) logo no início.
//   2. loop() fica parado esperando o comando "Calibrate".
//   3. Depois de calibrar, fica esperando o comando "Start".
//   4. "Stop" pode ser enviado a qualquer momento como parada de emergência.
// -----------------------------------------------------------------------------

class BluetoothBLE {
public:
  enum class Command { None, Calibrate, Start, Stop };

  // Inicializa o BLE, cria o serviço/characteristics e começa o advertising.
  BluetoothBLE();
  void begin(const char *Shadow);

  // Devolve o último comando recebido e limpa o status do comando
  // (para não processar o mesmo comando várias vezes).
  Command consumeCommand();


private:
  // Callbacks do NimBLE quando escreve na characteristic de comando.
  // Fica numa classe separada porque é assim que a API do NimBLE pede

  class CommandCallBacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic,
                 NimBLEConnInfo       &connInfo) override;
  };

  // Ponteiro para a instância de BluetoothBLE que criou esse callback, para
  // poder acessar o pendingCommand_ e setar o comando recebido.
  NimBLEServer         *server_;
  NimBLECharacteristic *commandCharacteristic_;
  CommandCallBacks      commandCallbacks_;

  static volatile Command pendingCommand_;
};

#endif