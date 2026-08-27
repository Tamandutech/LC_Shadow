#ifndef LINE_SENSOR_ARRAY_HPP
#define LINE_SENSOR_ARRAY_HPP

#include <array>
#include <cstdint>

#include "env.hpp" // NUM_LINE_SENSORS

// -----------------------------------------------------------------------------
// LineSensorArray
// -----------------------------------------------------------------------------
// Driver responsável apenas por HARDWARE: falar com o multiplexador e devolver
// os valores brutos lidos em cada um dos 12 sensores de linha.
//
// Esta classe NÃO faz nenhuma interpretação dos valores. Isso é papel do
// LineTracker (em src/logic/LineTracker), que recebe o array bruto e decide o
// que fazer com ele. Essa separação é o que permite testar toda a lógica de
// PID no PC, sem precisar do ESP ligado.
// -----------------------------------------------------------------------------
class LineSensorArray {
public:
  LineSensorArray();

  // Varre os 12 canais do multiplexador (endereça cada um e faz analogRead)
  // e devolve um array com os 12 valores brutos, na mesma ordem definida em
  // GPIO_MULTIPLEXER_LINE_SENSORS_INDEX (env.hpp).
  std::array<int32_t, NUM_LINE_SENSORS> readAll();

private:
  // Configura os pinos de endereço e o pino analógico como entrada/saída.
  void configurePins_();

  // Escreve o endereço binário (0 a 11) nos pinos de endereço do
  // multiplexador, selecionando qual sensor será lido em seguida.
  void selectChannel_(uint8_t channel);
};

#endif
