#ifndef STATUS_LED_HPP
#define STATUS_LED_HPP

// -----------------------------------------------------------------------------
// StatusLed
// -----------------------------------------------------------------------------
// Avisa o estado do robô por um LED
//
// Tem dois jeitos de piscar:
//   - blinkBlocking(): usa delay(), trava o resto do código enquanto pisca.
//     Só usar fora de loops que precisam ficar responsivos (ex: uma vez no
//     boot, antes de entrar no loop principal).
//   - updateBlinking(): NÃO trava nada. Chame a cada iteração de um loop
//     (ex: dentro do while da calibração, junto com a leitura dos
//     sensores) e o LED alterna sozinho no intervalo definido.
// -----------------------------------------------------------------------------
class StatusLed {
public:
  explicit StatusLed(int pin);

  // Configura o pino do LED como saída. Chama uma vez, no setup().
  void begin();

  // Acende/apaga o LED direto, sem piscar.
  void on();
  void off();

  // Pisca 'times' vezes de forma bloqueante (usa delay() entre cada
  // liga/desliga).
                     unsigned long offMs = 150);

                     // Alterna o LED sozinho, sem bloquear, baseado no tempo já
                     // passado desde a última troca. Chama isto repetidamente
                     // (a cada iteração de loop) durante todo o período em que
                     // quiser o LED piscando.
                     void updateBlinking(unsigned long intervalMs = 150);

                   private:
                     int           pin_;
                     bool          ledState_;
                     unsigned long lastToggleMs_;
};

#endif