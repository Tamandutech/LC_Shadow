// Framework
#include "Arduino.h"

// Env
#include "env.hpp"

// Drivers
#include "drivers/BluetoothBLE/BluetoothBLE.hpp"
#include "drivers/LineSensorArray/LineSensorArray.hpp"
#include "drivers/MotorDriver/MotorDriver.hpp"
#include "drivers/StatusLed/StatusLed.hpp"


// Lógica
#include "logic/LineTracker/LineTracker.hpp"

LineSensorArray lineSensors;
MotorDriver     motorLeft(GPIO_DIRECTION_A, GPIO_PWM_A, PWM_CHANNEL_MOTOR_A);
MotorDriver     motorRight(GPIO_DIRECTION_B, GPIO_PWM_B, PWM_CHANNEL_MOTOR_B);
BluetoothBLE    ble;
StatusLed       statusLed(GPIO_STATUS_LED);


// O LineTracker só pode ser criado DEPOIS da calibração (ele precisa do
// min/max de cada sensor no construtor), então começa como ponteiro nulo e é
// criado dentro de setup(), depois da calibração rodar.
LineTracker *lineTracker = nullptr;

// Estado do robô, usado para controlar o fluxo de execução no loop().
enum class RobotState { WAITING_CALIBRATION, WAITING_START, RUNNING };
RobotState robotState = RobotState::WAITING_CALIBRATION;

// -----------------------------------------------------------------------------
// Safety lock
// -----------------------------------------------------------------------------
// Trava a execução se qualquer pino ainda estiver com o valor placeholder
// (-1). Evita que o robô tente rodar com pinagem incompleta e faça algo
// inesperado, tipo sair voando ou rodar sei lá.
// -----------------------------------------------------------------------------
void haltWithError(const char *message) {
  Serial.begin(115200);
  while(true) {
    Serial.println(message);
    delay(1000);
  }
}

void checkPinsConfigured() {
  const int addressPins[4]   = GPIO_MULTIPLEXER_DIGITAL_ADDRESS;
  bool      allAddressPinsOk = true;
  for(int i = 0; i < 4; i++) {
    if(addressPins[i] == -1) allAddressPinsOk = false;
  }

  bool allPinsOk = allAddressPinsOk && GPIO_MULTIPLEXER_ANALOG_INPUT != -1 &&
                   GPIO_DIRECTION_A != -1 && GPIO_DIRECTION_B != -1 &&
                   GPIO_PWM_A != -1 && GPIO_PWM_B != -1;

  if(!allPinsOk) {
    haltWithError("ERRO: existe pino com valor -1 em env.hpp. "
                  "Preencha os TODOs antes de rodar o robo.");
  }
}

void runCalibration() {
  delay(POSITIONING_DELAY_MS);

  int calibrationMin[NUM_LINE_SENSORS];
  int calibrationMax[NUM_LINE_SENSORS];

  // Começa cada mínimo "artificialmente alto" e cada máximo "artificialmente
  // baixo", assim a primeira leitura real sempre corrige os dois.
  for(int i = 0; i < NUM_LINE_SENSORS; i++) {
    calibrationMin[i] = 4095;
    calibrationMax[i] = 0;
  }

  // Gira no próprio eixo: motor esquerdo pra frente, direito pra trás.
  // Isso faz a linha do multiplexador passar sob todos os 12 sensores em
  // algum momento, sem o robô sair do lugar.
  motorLeft.pwmOutput(CALIBRATION_SPEED);
  motorRight.pwmOutput(-CALIBRATION_SPEED);

  unsigned long startTime = millis();
  while(millis() - startTime < CALIBRATION_DURATION_MS) {
    statusLed.updateBlinking();
    auto rawReadings = lineSensors.readAll();

    for(int i = 0; i < NUM_LINE_SENSORS; i++) {
      if(rawReadings[i] < calibrationMin[i]) calibrationMin[i] = rawReadings[i];
      if(rawReadings[i] > calibrationMax[i]) calibrationMax[i] = rawReadings[i];
    }
  }
  statusLed.off();

  // Para os motores assim que a calibração termina.
  motorLeft.pwmOutput(0);
  motorRight.pwmOutput(0);

  // Evita memory leak se o lineTracker já tinha sido criado antes
  // (recalibração).
  delete lineTracker;
  // invertReadings = true: pista preta, linha branca (LineTracker.hpp).
  lineTracker = new LineTracker(PID_KP, PID_KI, PID_KD, calibrationMin,
                                calibrationMax, /*invertReadings=*/true);
  statusLed.on(); // aceso fixo = pronto pra receber o Start
}

// -----------------------------------------------------------------------------
// Aplica a correção do PID nos dois motores.
// -----------------------------------------------------------------------------
// correction negativa = linha à esquerda -> motor esquerdo desacelera,
// direito acelera (o robô vira pra esquerda) e vice-versa.
void applyMotorSpeeds(float correction) {
  if(correction > BASE_SPEED) correction = BASE_SPEED;
  if(correction < -BASE_SPEED) correction = -BASE_SPEED;

  int32_t leftSpeed  = BASE_SPEED - (int32_t)correction;
  int32_t rightSpeed = BASE_SPEED + (int32_t)correction;

  motorLeft.pwmOutput(leftSpeed);
  motorRight.pwmOutput(rightSpeed);
}

void setup() {
  Serial.begin(115200);

  checkPinsConfigured();

  ble.begin(BLE_DEVICE_NAME);

  statusLed.begin();
  statusLed.blinkBlocking(1); // 1 pisca = ligado, esperando Calibrate
}

void loop() {
  BluetoothBLE::Command command = ble.consumeCommand();

  switch(robotState) {
  case RobotState::WAITING_CALIBRATION:
    if(command == BluetoothBLE::Command::Calibrate) {
      runCalibration();
      robotState = RobotState::WAITING_START;
    }
    break;

  case RobotState::WAITING_START:
    if(command == BluetoothBLE::Command::Start) {
      statusLed.off(); // apagado = rodando
      robotState = RobotState::RUNNING;
    } else if(command == BluetoothBLE::Command::Calibrate) {
      // Permite recalibrar antes de começar a andar de verdade.
      runCalibration();
    }
    break;

  case RobotState::RUNNING: {
    if(command == BluetoothBLE::Command::Stop) {
      motorLeft.pwmOutput(0);
      motorRight.pwmOutput(0);
      statusLed.on(); // aceso fixo de novo = pronto pra receber Start
      robotState = RobotState::WAITING_START;
      break;
    }

    auto  rawReadings = lineSensors.readAll();
    float correction  = lineTracker->update(rawReadings.data());
    applyMotorSpeeds(correction);
    break;
  }
  }
}