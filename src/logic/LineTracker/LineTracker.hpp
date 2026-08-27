#ifndef LINE_TRACKER_HPP
#define LINE_TRACKER_HPP

#include "env.hpp"

// -----------------------------------------------------------------------------
// Nenhuma linha aqui sabe o que é um "pino". Só recebe e devolve números.
// Dá pra testar isso tudo no PC, antes de ter o robô físico
// (test/test_LineTracker/lineTracker.test.cpp, rodado no env:desktop).
// -----------------------------------------------------------------------------

// Resultado do cálculo de erro de posição da linha.
struct LineError {
  float value;        // negativo = linha à esquerda, positivo = à direita
  bool  lineDetected; // false quando nenhum sensor está vendo linha
};

// Estado interno do controlador PID. Fica fora da classe LineTracker (em vez
// de ser privado) porque assim calculatePid() continua testável sozinha,
// sem precisar montar um LineTracker inteiro.
struct PidState {
  float kp;
  float ki;
  float kd;
  float integral;
  float lastError;

  PidState(float kp_, float ki_, float kd_)
      : kp(kp_), ki(ki_), kd(kd_), integral(0), lastError(0) {}
};

// Converte uma leitura crua pra 0-1000, usando o min/max vistos na calibração.
// 'invert' = true quando a leitura fica MENOR sobre a linha
int normalize(int rawReading, int calMin, int calMax, bool invert);

// Recebe leituras JÁ NORMALIZADAS (0 a 1000) de N sensores, da esquerda pra
// direita, e devolve o erro de posição.
LineError calculateLineError(const int *readings, int count,
                             int detectionThreshold = 150);

// Calcula a saída do PID a partir do erro atual e do estado acumulado
// (integral, erro anterior). O estado é passado por referência porque o PID
// precisa "lembrar" do ciclo anterior.
float calculatePid(PidState &state, float error);

// -----------------------------------------------------------------------------
// LineTracker
// -----------------------------------------------------------------------------
// Orquestra as três funções acima: recebe o array bruto do LineSensorArray
// (driver), normaliza cada sensor, calcula o erro de posição e alimenta o
// PID. Devolve a correção pronta pra mandar pro MotorDriver.
//
// Guarda a calibração (min/max de cada sensor) e o estado do PID entre
// ciclos, então mantenha UMA instância viva durante toda a execução do robô.
// -----------------------------------------------------------------------------
class LineTracker {
public:
  // calibrationMin/Max: um valor por sensor (índices 0 a NUM_LINE_SENSORS-1),
  // vindos da calibração feita antes da prova.
  LineTracker(float kp, float ki, float kd,
              const int calibrationMin[NUM_LINE_SENSORS],
              const int calibrationMax[NUM_LINE_SENSORS],
              bool      invertReadings = true);

  // Processa um ciclo completo: normaliza -> calcula erro -> roda o PID.
  // Devolve a saída do PID (a correção de direção a aplicar nos motores).
  float update(const int rawReadings[NUM_LINE_SENSORS]);

  // Diz se a última chamada a update() detectou a linha.
  bool isLineDetected() const;

private:
  PidState pidState_;
  int      calibrationMin_[NUM_LINE_SENSORS];
  int      calibrationMax_[NUM_LINE_SENSORS];
  bool     invertReadings_;
  bool     lineDetected_;
};

#endif
