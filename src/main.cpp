// Framework
#include "Arduino.h"

// Env
#include "env.hpp"

// Drivers
#include "drivers/BluetoothBLE/BluetoothBLE.hpp"
#include "drivers/LineSensorArray/LineSensorArray.hpp"
#include "drivers/MotorDriver/MotorDriver.hpp"

// Lógica
#include "logic/LineTracker/LineTracker.hpp"


LineSensorArray lineSensors;
MotorDriver     motorLeft(GPIO_DIRECTION_A, GPIO_PWM_A, PWM_CHANNEL_MOTOR_A);
MotorDriver     motorRight(GPIO_DIRECTION_B, GPIO_PWM_B, PWM_CHANNEL_MOTOR_B);
BluetoothBLE    ble;


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

void finalizeCalibration(int *calMin, int *calMax) {
  delete lineTracker; // evita memory leak em recalibração
  lineTracker = new LineTracker(PID_KP, PID_KI, PID_KD, calMin, calMax,
                                /*invertReadings=*/true);
}

// Calibração automática
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
    auto rawReadings = lineSensors.readAll();

    for(int i = 0; i < NUM_LINE_SENSORS; i++) {
      if(rawReadings[i] < calibrationMin[i]) calibrationMin[i] = rawReadings[i];
      if(rawReadings[i] > calibrationMax[i]) calibrationMax[i] = rawReadings[i];
    }
  }

  // Para os motores assim que a calibração termina.
  motorLeft.pwmOutput(0);
  motorRight.pwmOutput(0);

  finalizeCalibration(calibrationMin, calibrationMax);
}

// Calibração manual
bool runManualCalibration() {
  motorLeft.pwmOutput(0);
  motorRight.pwmOutput(0);

  int calibrationMin[NUM_LINE_SENSORS];
  int calibrationMax[NUM_LINE_SENSORS];
  for(int i = 0; i < NUM_LINE_SENSORS; i++) {
    calibrationMin[i] = 4095;
    calibrationMax[i] = 0;
  }

  unsigned long startTime            = millis();
  bool          allSensorsCalibrated = false;

  while(!allSensorsCalibrated &&
        millis() - startTime < MANUAL_CALIBRATION_TIMEOUT_MS) {
    auto rawReadings = lineSensors.readAll();

    allSensorsCalibrated = true; // assume que sim, prova o contrário abaixo
    for(int i = 0; i < NUM_LINE_SENSORS; i++) {
      if(rawReadings[i] < calibrationMin[i]) calibrationMin[i] = rawReadings[i];
      if(rawReadings[i] > calibrationMax[i]) calibrationMax[i] = rawReadings[i];

      if(calibrationMax[i] - calibrationMin[i] < MIN_CALIBRATION_RANGE) {
        allSensorsCalibrated = false;
      }
    }
  }

  finalizeCalibration(calibrationMin, calibrationMax);
  return allSensorsCalibrated;
}

// -----------------------------------------------------------------------------
// Aplica a correção do PID nos dois motores.
// -----------------------------------------------------------------------------
// correction negativa = linha à esquerda -> motor esquerdo desacelera,
// direito acelera (o robô vira pra esquerda) e vice-versa.
void applyMotorSpeeds(float correction) {
  int32_t leftSpeed  = BASE_SPEED - (int32_t)correction;
  int32_t rightSpeed = BASE_SPEED + (int32_t)correction;

  motorLeft.pwmOutput(leftSpeed);
  motorRight.pwmOutput(rightSpeed);
}

void setup() {
  Serial.begin(115200);

  ble.begin(BLE_DEVICE_NAME);

  checkPinsConfigured();
}

void loop() {

  if(NuSerial.available()) {
    String command = NuSerial.readStringUntil('\n');
    command.trim(); // Remove espaços em branco no início e no fim

    if(command == "Calibrate") {
      runCalibration();
      NuSerial.println("Calibrated");
    } else if(command == "CalibrateManual") {
      bool ok = runManualCalibration();
      NuSerial.println(ok ? "Calibrated"
                          : "Calibrated - AVISO: timeout, algum sensor pode "
                            "nao ter sido calibrado direito");
    } else if(command == "Start") {
      if(lineTracker == nullptr) {
        NuSerial.println("Erro: calibre antes de dar Start");
      } else {
        robotState = RobotState::RUNNING;
        NuSerial.println("Running");
      }
    } else if(command == "Stop") {
      motorLeft.pwmOutput(0);
      motorRight.pwmOutput(0);
      robotState = RobotState::WAITING_START;
      NuSerial.println("Stopped");
    } else {
      NuSerial.println("Unknown command");
    }
  }

  // É pra fazer o tobô andar certinho quando der Start
  // NOTA: TEM que fazer o Calibrate antes do Start, senão o lineTracker é nulo
  // e pode crashar ;-;
  if(robotState == RobotState::RUNNING && lineTracker != nullptr) {
    auto  rawReadings = lineSensors.readAll();
    float correction  = lineTracker->update(rawReadings.data());
    applyMotorSpeeds(correction);
  }
}