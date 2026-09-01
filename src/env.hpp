#ifndef ENV_HPP
#define ENV_HPP

// =============================================================================
// PINOS DO ROBÔ
// -----------------------------------------------------------------------------
// Substituir todos os valores -1 pelos pinos reais assim que o
// hardware estiver montado.
// =============================================================================

// Quantidade de sensores de linha lidos pelo multiplexador.
#define NUM_LINE_SENSORS (12)

// --- Multiplexador dos sensores de linha ------------------------------------

#define GPIO_MULTIPLEXER_DIGITAL_ADDRESS {39, 40, 41, 42} // 4 pinos de endereço
#define GPIO_MULTIPLEXER_ANALOG_INPUT    (10)             // 1 pino analógico

// Ordem dos 12 sensores de linha nos canais do multiplexador.
#define GPIO_MULTIPLEXER_LINE_SENSORS_INDEX \
  {13, 12, 11, 10, 9, 8, 5, 4, 3, 2, 1, 0}

// --- Motores ------------------------------------------------------------
// Cada motor tem um pino de direção (gira sentido horário/anti-horário) e
// um pino de PWM (controla a velocidade).
#define GPIO_DIRECTION_A (9)  // pino de direção do motor A (esquerdo)
#define GPIO_PWM_A       (3)  // pino PWM do motor A (esquerdo)

#define GPIO_DIRECTION_B (37) // pino de direção do motor B (direito)
#define GPIO_PWM_B       (38) // pino PWM do motor B (direito)

// Canais do periférico LEDC do ESP32 usados por cada motor.
// Não são pinos físicos, só um número de canal interno.
#define PWM_CHANNEL_MOTOR_A (0)
#define PWM_CHANNEL_MOTOR_B (1)

// Frequência e resolução do PWM. 8 bits = valores de 0 a 255.
#define PWM_FREQUENCY_HZ    (5000)
#define PWM_RESOLUTION_BITS (8)
#define MAX_PWM_VALUE       (255)

// --- Calibração Automática
// Ao ligar, o robô espera os micro segundos definidos e depois calibra sozinho.
#define POSITIONING_DELAY_MS (3000)

// --- Parâmetros de controle ------------------------------------------------
// Ganhos do PID. Ainda não calibrados/testados.
// ajustar esses valores durante os testes na pista qnd robô estiver montado.
#define PID_KP (1.0f)
#define PID_KI (0.0f)
#define PID_KD (0.0f)

// Velocidade base aplicada aos dois motores antes de somar a correção do PID.
// ajustar conforme o motor/bateria do robô.
#define BASE_SPEED (100)

// Velocidade usada só durante a calibração, girando o robô no próprio eixo.
// Pode ser mais baixa que BASE_SPEED pra girar de forma mais controlada.
// ajustar conforme o motor/bateria do robô.
#define CALIBRATION_SPEED (50)

// Duração da varredura de calibração automática (ver runCalibration()).
#define CALIBRATION_DURATION_MS (3000)

// Bluetooth BLE
#define BLE_DEVICE_NAME "Shadow"

#endif
