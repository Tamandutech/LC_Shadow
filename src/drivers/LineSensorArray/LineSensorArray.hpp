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
  LineSensorArray() = default;

  // Configura os pinos como entrada analógica.
  void begin();

  // Lê os 12 sensores e devolve os valores brutos, na mesma ordem de
  // GPIO_LINE_SENSORS (esquerda -> direita).
  std::array<int32_t, NUM_LINE_SENSORS> readAll();
};

#endif
