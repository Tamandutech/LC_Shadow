#include <unity.h>

#include "logic/LineTracker/LineTracker.hpp"

void setUp(void) {}    // antes de cada teste
void tearDown(void) {} // depois de cada teste

// -----------------------------------------------------------------------------
// normalize()
// -----------------------------------------------------------------------------

void test_NormalizeMapsRawReadingToZeroToThousandRange(void) {
  // Leitura no meio da faixa de calibração deve virar ~500.
  int result = normalize(500, 0, 1000, false);
  TEST_ASSERT_EQUAL_INT(500, result);
}

void test_NormalizeClampsBelowCalibrationMin(void) {
  // Leitura abaixo do mínimo calibrado não pode passar de 0.
  int result = normalize(-50, 0, 1000, false);
  TEST_ASSERT_EQUAL_INT(0, result);
}

void test_NormalizeClampsAboveCalibrationMax(void) {
  // Leitura acima do máximo calibrado não pode passar de 1000.
  int result = normalize(5000, 0, 1000, false);
  TEST_ASSERT_EQUAL_INT(1000, result);
}

void test_NormalizeInvertsWhenTrackIsBlackAndLineIsWhite(void) {
  // Nosso caso: pista preta / linha branca -> leitura crua BAIXA sobre a
  // linha deve virar valor normalizado ALTO.
  int lowRawReadingOnLine = 0;
  int result              = normalize(lowRawReadingOnLine, 0, 1000, true);
  TEST_ASSERT_EQUAL_INT(1000, result);
}

void test_NormalizeReturnsZeroWhenCalibrationIsInvalid(void) {
  // calMax <= calMin: calibração inválida, não pode dividir por zero/negativo.
  int result = normalize(500, 800, 800, false);
  TEST_ASSERT_EQUAL_INT(0, result);
}

// -----------------------------------------------------------------------------
// calculateLineError()
// -----------------------------------------------------------------------------

void test_CalculateLineErrorIsZeroWhenLineIsCentered(void) {
  // Linha exatamente embaixo do sensor do meio (índice 2 de 5).
  int readings[5] = {0, 0, 1000, 0, 0};
  LineError error = calculateLineError(readings, 5);

  TEST_ASSERT_TRUE(error.lineDetected);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, error.value);
}

void test_CalculateLineErrorIsNegativeWhenLineIsToTheLeft(void) {
  // Linha no sensor mais à esquerda (índice 0).
  int readings[5] = {1000, 0, 0, 0, 0};
  LineError error = calculateLineError(readings, 5);

  TEST_ASSERT_TRUE(error.lineDetected);
  TEST_ASSERT_TRUE(error.value < 0);
}

void test_CalculateLineErrorIsPositiveWhenLineIsToTheRight(void) {
  // Linha no sensor mais à direita (índice 4).
  int readings[5] = {0, 0, 0, 0, 1000};
  LineError error = calculateLineError(readings, 5);

  TEST_ASSERT_TRUE(error.lineDetected);
  TEST_ASSERT_TRUE(error.value > 0);
}

void test_CalculateLineErrorReportsNoLineWhenBelowThreshold(void) {
  // Nenhum sensor vendo nada relevante: soma fica abaixo do limiar.
  int readings[5] = {10, 10, 10, 10, 10};
  LineError error = calculateLineError(readings, 5, 150);

  TEST_ASSERT_FALSE(error.lineDetected);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, error.value);
}

// -----------------------------------------------------------------------------
// calculatePid()
// -----------------------------------------------------------------------------

void test_CalculatePidReturnsProportionalTermOnFirstCall(void) {
  // Na primeira chamada, integral e erro anterior começam em 0, então a
  // saída deve ser só o termo proporcional (kp * erro).
  PidState state(2.0f, 0.0f, 0.0f);
  float    output = calculatePid(state, 10.0f);

  TEST_ASSERT_EQUAL_FLOAT(20.0f, output);
}

void test_CalculatePidAccumulatesIntegralAcrossCalls(void) {
  PidState state(0.0f, 1.0f, 0.0f);
  calculatePid(state, 5.0f);              // integral vira 5
  float output = calculatePid(state, 5.0f); // integral vira 10

  TEST_ASSERT_EQUAL_FLOAT(10.0f, output);
}

void test_CalculatePidComputesDerivativeFromPreviousError(void) {
  PidState state(0.0f, 0.0f, 1.0f);
  calculatePid(state, 10.0f);                // erroAnterior vira 10
  float output = calculatePid(state, 15.0f); // derivada = 15 - 10 = 5

  TEST_ASSERT_EQUAL_FLOAT(5.0f, output);
}

// -----------------------------------------------------------------------------
// LineTracker (integração das três funções acima)
// -----------------------------------------------------------------------------

void test_LineTrackerDetectsLineAndReturnsCorrection(void) {
  int calMin[NUM_LINE_SENSORS];
  int calMax[NUM_LINE_SENSORS];
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    calMin[i] = 0;
    calMax[i] = 1000;
  }

  LineTracker tracker(1.0f, 0.0f, 0.0f, calMin, calMax, /*invertReadings=*/true);

  // Pista preta / linha branca: leitura BAIXA = está em cima da linha.
  // Linha embaixo do sensor mais à esquerda -> erro negativo esperado.
  int rawReadings[NUM_LINE_SENSORS] = {0, 1000, 1000, 1000, 1000,
                                        1000, 1000, 1000, 1000, 1000, 1000, 1000};

  float correction = tracker.update(rawReadings);

  TEST_ASSERT_TRUE(tracker.isLineDetected());
  TEST_ASSERT_TRUE(correction < 0);
}

void test_LineTrackerReportsNoLineWhenAllSensorsSeeTrack(void) {
  int calMin[NUM_LINE_SENSORS];
  int calMax[NUM_LINE_SENSORS];
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    calMin[i] = 0;
    calMax[i] = 1000;
  }

  LineTracker tracker(1.0f, 0.0f, 0.0f, calMin, calMax, /*invertReadings=*/true);

  // Todos os sensores veem só a pista preta (leitura crua alta = fundo).
  int rawReadings[NUM_LINE_SENSORS] = {1000, 1000, 1000, 1000, 1000, 1000,
                                        1000, 1000, 1000, 1000, 1000, 1000};

  tracker.update(rawReadings);

  TEST_ASSERT_FALSE(tracker.isLineDetected());
}

void process() {
  UNITY_BEGIN();

  RUN_TEST(test_NormalizeMapsRawReadingToZeroToThousandRange);
  RUN_TEST(test_NormalizeClampsBelowCalibrationMin);
  RUN_TEST(test_NormalizeClampsAboveCalibrationMax);
  RUN_TEST(test_NormalizeInvertsWhenTrackIsBlackAndLineIsWhite);
  RUN_TEST(test_NormalizeReturnsZeroWhenCalibrationIsInvalid);

  RUN_TEST(test_CalculateLineErrorIsZeroWhenLineIsCentered);
  RUN_TEST(test_CalculateLineErrorIsNegativeWhenLineIsToTheLeft);
  RUN_TEST(test_CalculateLineErrorIsPositiveWhenLineIsToTheRight);
  RUN_TEST(test_CalculateLineErrorReportsNoLineWhenBelowThreshold);

  RUN_TEST(test_CalculatePidReturnsProportionalTermOnFirstCall);
  RUN_TEST(test_CalculatePidAccumulatesIntegralAcrossCalls);
  RUN_TEST(test_CalculatePidComputesDerivativeFromPreviousError);

  RUN_TEST(test_LineTrackerDetectsLineAndReturnsCorrection);
  RUN_TEST(test_LineTrackerReportsNoLineWhenAllSensorsSeeTrack);

  UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
  // Espera >2s para placas que não suportam reset via Serial.DTR/RTS.
  delay(2000);

  process();
}

void loop() {}

#else

int main(int argc, char** argv) {
  process();
  return 0;
}

#endif
