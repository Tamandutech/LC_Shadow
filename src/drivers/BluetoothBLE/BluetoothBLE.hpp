#ifndef BLUETOOTH_BLE_HPP

#define BLUETOOTH_BLE_HPP

#include <NimBLEDevice.h>
#include <NuSerial.hpp>
#include <atomic>

class BluetoothBLE {
public:
  enum class Command { None, Calibrate, Start, Stop };

  BluetoothBLE();

  // Inicializa o BLE, cria o serviço/characteristic de comando E sobe o
  // Nordic UART Service (NuS, via a lib NuSerial) no MESMO servidor BLE.
  void begin(const char *deviceName);

  // Devolve o último comando recebido e limpa o estado atomicamente
  Command consumeCommand();

  void sendTelemetry(const String &line);
  
  bool isTelemetryConnected() const;

private:
  // Callback do NimBLE quando o celular escreve na characteristic de
  // comando. Guarda um ponteiro pro BluetoothBLE "dono" (owner_), em vez de
  // usar uma variável static/global

  class CommandCallbacks : public NimBLECharacteristicCallbacks {
  public:
    explicit CommandCallbacks(BluetoothBLE *owner) : owner_(owner) {}
    void onWrite(NimBLECharacteristic *characteristic,
                 NimBLEConnInfo       &connInfo) override;

  private:
    BluetoothBLE *owner_;
  };

  NimBLEServer         *server_;
  NimBLECharacteristic *commandCharacteristic_;
  CommandCallbacks      commandCallbacks_;

  // std::atomic no lugar de volatile: garante leitura/escrita atômica entre
  // a task do NimBLE (que chama onWrite) e o loop() principal, sem as
  // ambiguidades do "volatile". Se tudo correr bem foi essa volatilidade que
  // causou o bug ;-; .
  std::atomic<Command> pendingCommand_;
};

#endif