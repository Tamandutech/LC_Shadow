#include "LineTracker.hpp"

// Função auxiliar interna: limita um valor entre um mínimo e um máximo.
static int clampInt(int v, int lo, int hi) {
  if(v < lo) return lo;
  if(v > hi) return hi;
  return v;
}

int normalize(int rawReading, int calMin, int calMax, bool invert) {
  // Calibração inválida (min >= max): não dá pra normalizar, devolve 0.
  if(calMax <= calMin) return 0;

  // mapeia [calMin, calMax] pra [0, 1000].
  long n = (long)(rawReading - calMin) * 1000L / (calMax - calMin);
  n      = clampInt((int)n, 0, 1000);

  // Pista preta / linha branca: a leitura crua é MENOR em cima da linha,
  // então invertemos pra manter a convenção "valor alto = vendo a linha".
  if(invert) n = 1000 - n;
  return (int)n;
}

LineError calculateLineError(const int *readings, int count,
                             int detectionThreshold) {
  long sum         = 0;
  long weightedSum = 0;

  // Média ponderada: cada sensor "pesa" de acordo com sua posição
  // (sensor 0 = peso 0, sensor 1 = peso 1000, sensor 2 = peso 2000, ...).
  // Quanto mais a soma puxar pra um lado, mais a linha está deslocada pra
  // aquele lado.
  for(int i = 0; i < count; i++) {
    sum += readings[i];
    weightedSum += (long)readings[i] * (i * 1000);
  }

  LineError result;

  // Soma abaixo do limiar = nenhum sensor relevante está vendo a linha.
  if(sum < detectionThreshold) {
    result.value        = 0;
    result.lineDetected = false;
    return result;
  }

  // Posição do "centro de massa" da linha entre os sensores.
  float position = (float)weightedSum / (float)sum;

  // Posição que representaria a linha exatamente no meio do robô.
  float center = ((count - 1) / 2.0f) * 1000.0f;

  // Erro = o quanto a linha está desviada do centro.
  // Negativo = esquerda, positivo = direita.
  result.value        = position - center;
  result.lineDetected = true;
  return result;
}

float calculatePid(PidState &state, float error) {
  state.integral += error;
  float derivative = error - state.lastError;
  state.lastError  = error;

  return (state.kp * error) + (state.ki * state.integral) +
         (state.kd * derivative);
}

LineTracker::LineTracker(float kp, float ki, float kd,
                         const int calibrationMin[NUM_LINE_SENSORS],
                         const int calibrationMax[NUM_LINE_SENSORS],
                         bool      invertReadings)
    : pidState_(kp, ki, kd), invertReadings_(invertReadings),
      lineDetected_(false) {
  for(int i = 0; i < NUM_LINE_SENSORS; i++) {
    calibrationMin_[i] = calibrationMin[i];
    calibrationMax_[i] = calibrationMax[i];
  }
}

float LineTracker::update(const int rawReadings[NUM_LINE_SENSORS]) {
  int normalizedReadings[NUM_LINE_SENSORS];

  // 1: normaliza cada sensor individualmente, usando a calibração específica
  // daquele sensor
  for(int i = 0; i < NUM_LINE_SENSORS; i++) {
    normalizedReadings[i] = normalize(rawReadings[i], calibrationMin_[i],
                                      calibrationMax_[i], invertReadings_);
  }

  // 2: calcula o erro de posição a partir dos valores já normalizados.
  LineError error = calculateLineError(normalizedReadings, NUM_LINE_SENSORS);
  lineDetected_   = error.lineDetected;

  // 3: alimenta o PID com o erro e devolve a correção.
  return calculatePid(pidState_, error.value);
}

bool LineTracker::isLineDetected() const { return lineDetected_; }
