#ifndef ENV_HPP
#define ENV_HPP

// =============================================================================
// PINOS DO ROBÔ
// -----------------------------------------------------------------------------
// Substituir todos os valores -1 pelos pinos reais assim que o
// hardware estiver montado.
// =============================================================================

// Quantidade de sensores de linha.
#define NUM_LINE_SENSORS (12)

// --- Sensores de linha ------------------------------------

#define GPIO_LINE_SENSORS \
  {PB0, PC5, PC4, PA7, PA6, PA5, PA4, PA3, PA2, PA1, PA0, PC3}

// --- Motores ------------------------------------------------------------
// Cada motor tem um pino de direção (gira sentido horário/anti-horário) e
// um pino de PWM (controla a velocidade).
#define GPIO_DIRECTION_A (PC7) // pino de direção do motor A (esquerdo)
#define GPIO_PWM_A       (PC6) // pino PWM do motor A (esquerdo)

#define GPIO_DIRECTION_B (PC9) // pino de direção do motor B (direito)
#define GPIO_PWM_B       (PC8) // pino PWM do motor B (direito)

// --- Motor de sucção -------------------------------
#define GPIO_PWM_VACUUM (PC12)

/* Canais do periférico LEDC do ESP32 usados por cada motor.
Não são pinos físicos, só um número de canal interno.
#define PWM_CHANNEL_MOTOR_A (0)
#define PWM_CHANNEL_MOTOR_B (1)
#define PWM_CHANNEL_VACUUM  (6) */

// Frequência e resolução d3o PWM. 8 bits = valores de 0 a 255.
#define PWM_FREQUENCY_HZ    (5000)
#define PWM_RESOLUTION_BITS (8)
#define MAX_PWM_VALUE       (255)


// --- Calibração Automática----------------------------------
// Ao ligar, o robô espera os micro segundos definidos e depois calibra sozinho.
#define POSITIONING_DELAY_MS (2000)

// Velocidade usada só durante a calibração, girando o robô no próprio eixo.
// Pode ser mais baixa que BASE_SPEED pra girar de forma mais controlada.
// ajustar conforme o motor/bateria do robô.
#define CALIBRATION_SPEED (60)

// Duração da varredura de calibração automática (ver runCalibration()).
#define CALIBRATION_DURATION_MS (3000)

// --- Bluetooth (módulo externo por UART) -------------------------------------

#define BT_UART_RX_PIN (PA10)
#define BT_UART_TX_PIN (PA9)

#define BLE_BUS huart1

#define BT_BAUD (230400)

#endif