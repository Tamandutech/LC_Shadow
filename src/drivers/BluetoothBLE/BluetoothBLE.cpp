#include "BluetoothBLE.hpp"

// UUIDs do serviço e das characteristics BLE. São só identificadores
// aleatórios (versão 4 de UUID)
#define SERVICE_UUID                "ABCD"
#define COMMAND_CHARACTERISTIC_UUID "1234"

BluetoothBLE::BluetoothBLE()
    : server_(nullptr), commandCharacteristic_(nullptr),
      commandCallbacks_(this), pendingCommand_(Command::None) {}

void BluetoothBLE::CommandCallbacks::onWrite(
    NimBLECharacteristic *characteristic, NimBLEConnInfo & /*connInfo*/) {
  std::string value = characteristic->getValue();

  // Converte o valor recebido para string e compara com os comandos esperados.
  /*
    if(value == "Calibrate") {
      owner_->pendingCommand_.store(Command::Calibrate);
    } else if(value == "Start") {
      owner_->pendingCommand_.store(Command::Start);
    } else if(value == "Stop") {
      owner_->pendingCommand_.store(Command::Stop);
    }
      */
}

void BluetoothBLE::begin(const char *deviceName) {
  NimBLEDevice::init(deviceName);

  // Cria o servidor BLE, o serviço e a characteristic de comando.
  server_                = NimBLEDevice::createServer();
  NimBLEService *service = server_->createService(SERVICE_UUID);

  commandCharacteristic_ = service->createCharacteristic(
      COMMAND_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE);
  commandCharacteristic_->setCallbacks(&commandCallbacks_);

  service->start();

  // Deixa o nome e o UUID do NOSSO serviço prontos pro pacote de
  // advertising, mas SEM chamar advertising->start() aqui: quem inicia o
  // advertising é o NuSerial.start()
  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->setName(deviceName);
  advertising->addServiceUUID(SERVICE_UUID);

  // Sobe o Nordic UART Service (lib NuS-NimBLE-Serial) no MESMO servidor
  // BLE.
  NuSerial.start();
}

void BluetoothBLE::sendTelemetry(const String &line) {
  if(NuSerial.isConnected()) {
    NuSerial.println(line);
  }
}

bool BluetoothBLE::isTelemetryConnected() const {
  return NuSerial.isConnected();
}

// exchange() lê o valor atual E escreve None no lugar, tudo numa única
// operação atômica — "ler, depois zerar"
BluetoothBLE::Command BluetoothBLE::consumeCommand() {
  return pendingCommand_.exchange(Command::None);
}