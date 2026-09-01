#include "BluetoothBLE.hpp"

// UUIDs do serviço e das characteristics BLE. São só identificadores
// aleatórios (versão 4 de UUID)
#define SERVICE_UUID                "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define COMMAND_CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

// Variável estática que guarda o último comando recebido via BLE. Inicialmente
// é None, e é atualizada pelo callback CommandCallBacks::OnWrite()
volatile BluetoothBLE::Command BluetoothBLE::pendingCommand_ =
    BluetoothBLE::Command::None;

// Construtor da classe BluetoothBLE. Inicializa os ponteiros do servidor e dá
// characteristic como nulos, para serem criados no begin().
BluetoothBLE::BluetoothBLE()
    : server_(nullptr), commandCharacteristic_(nullptr) {}


void BluetoothBLE::CommandCallBacks::onWrite(
    NimBLECharacteristic *characteristic, NimBLEConnInfo & /*connInfo*/) {
  std::string value = characteristic->getValue();

  // Converte o valor recebido para string e compara com os comandos esperados.
  if(value == "Calibrate") {
    pendingCommand_ = Command::Calibrate;
  } else if(value == "Start") {
    pendingCommand_ = Command::Start;
  } else if(value == "Stop") {
    pendingCommand_ = Command::Stop;
  }
}

// Liga o bluetooth e define o nome
void BluetoothBLE::begin(const char *Shadow) {
  NimBLEDevice::init(Shadow);

  // Cria o servidor BLE, o serviço e a characteristic de comando.
  server_                = NimBLEDevice::createServer();
  NimBLEService *service = server_->createService(SERVICE_UUID);

  // O callback CommandCallBacks::onWrite() é chamado quando o celular
  // escreve.
  commandCharacteristic_ = service->createCharacteristic(
      COMMAND_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE);
  commandCharacteristic_->setCallbacks(&commandCallbacks_);

  service->start();

  // Literalmente advertising -_-
  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();
}

// Devolve o último comando recebido via BLE e limpa a variável, para não
// processar o mesmo comando várias vezes.
BluetoothBLE::Command BluetoothBLE::consumeCommand() {
  Command command = pendingCommand_;
  pendingCommand_ = Command::None;
  return command;
}