#include "LineSensorArray.hpp"

#include "Arduino.h"
#include "env.hpp"

// Guarda os pinos de endereço num array fixo para poder iterar sobre eles
// (escrever bit a bit) em selectChannel_().
static const int kAddressPins[4] = GPIO_MULTIPLEXER_DIGITAL_ADDRESS;

// Ordem física dos sensores nos canais do multiplexador (ver env.hpp).
static const uint8_t kLineSensorsIndex[NUM_LINE_SENSORS] =
    GPIO_MULTIPLEXER_LINE_SENSORS_INDEX;

LineSensorArray::LineSensorArray() { configurePins_(); }

void LineSensorArray::configurePins_() {
  // Os 4 pinos de endereço são saídas: é o ESP32 que "escolhe" o canal.
  for(uint8_t i = 0; i < 4; i++) {
    pinMode(kAddressPins[i], OUTPUT);
  }
  // O pino analógico é entrada: é por onde o valor do sensor chega.
  pinMode(GPIO_MULTIPLEXER_ANALOG_INPUT, INPUT);
}

void LineSensorArray::selectChannel_(uint8_t channel) {
  // Escreve o endereço em binário, um bit por pino de endereço.
  // Ex: channel = 5 (0b0101) -> pino0=1, pino1=0, pino2=1, pino3=0
  for(uint8_t bit = 0; bit < 4; bit++) {
    bool bitValue = (channel >> bit) & 0x01;
    digitalWrite(kAddressPins[bit], bitValue ? HIGH : LOW);
  }

  // Pequeno atraso para o multiplexador estabilizar o sinal antes de ler.
  // ajustar esse valor conforme testar; alguns multiplexadores
  // precisam de poucos microssegundos, outros toleram ler na sequência.
  delayMicroseconds(5);
}

std::array<int32_t, NUM_LINE_SENSORS> LineSensorArray::readAll() {
  std::array<int32_t, NUM_LINE_SENSORS> values{};

  for(uint8_t i = 0; i < NUM_LINE_SENSORS; i++) {
    uint8_t channel = kLineSensorsIndex[i];
    selectChannel_(channel);
    values[i] = analogRead(GPIO_MULTIPLEXER_ANALOG_INPUT);
  }

  return values;
}
