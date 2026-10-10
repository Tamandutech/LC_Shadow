#include "BluetoothBLE.hpp"

#include <string.h>

#include "env.hpp"

BluetoothPort NuSerial;

// -----------------------------------------------------------------------------
// Detecção automática de baud rate
// -----------------------------------------------------------------------------
static const unsigned long kOtherBauds[] = {9600,  115200, 57600,  38400,
                                            19200, 230400, 460800, 921600};
static const int kNumOtherBauds = sizeof(kOtherBauds) / sizeof(kOtherBauds[0]);

// Tempo em cada baud antes de tentar o próximo.
static const unsigned long kDwellMs = 2500;
// Sem novos bytes por este tempo = a linha terminou (comandos sem \n).
static const unsigned long kLineGapMs = 80;

// Comandos que o main.cpp entende. Receber um deles = baud correto.
static const char *const kKnownCommands[] = {
    "Calibrate", "CalibrateManual", "Start", "Stop", "kd",
    "kp",        "setspeed",        "setvac"};
static const int kNumKnownCommands =
    sizeof(kKnownCommands) / sizeof(kKnownCommands[0]);

static bool equalsIgnoreCase(const char *a, const char *b) {
  while(*a && *b) {
    char ca = *a;
    char cb = *b;
    if(ca >= 'A' && ca <= 'Z') ca = ca - 'A' + 'a';
    if(cb >= 'A' && cb <= 'Z') cb = cb - 'A' + 'a';
    if(ca != cb) return false;
    a++;
    b++;
  }
  return *a == *b;
}

void BluetoothPort::begin(unsigned long baud) {
  baseBaud_    = baud;
  currentBaud_ = baud;
  otherIdx_    = -1;
  lineLen_     = 0;
  pushLen_     = 0;
  pushPos_     = 0;
  lastByteAt_  = 0;
#if BT_AUTOBAUD
  locked_ = false;
#else
  locked_ = true;
#endif
  Serial1.begin(currentBaud_);
  switchedAt_ = millis();
}

void BluetoothPort::nextBaud() {
  // Avança na lista, pulando o baud base (já foi tentado primeiro).
  do {
    otherIdx_++;
    if(otherIdx_ >= kNumOtherBauds) otherIdx_ = -1;
    currentBaud_ = (otherIdx_ < 0) ? baseBaud_ : kOtherBauds[otherIdx_];
  } while(otherIdx_ >= 0 && kOtherBauds[otherIdx_] == baseBaud_);

  Serial1.end();
  Serial1.begin(currentBaud_);
  lineLen_    = 0;
  switchedAt_ = millis();
}

// Olha a linha acumulada. Se for um comando conhecido, trava no baud atual.
bool BluetoothPort::lockIfCommand() {
  line_[lineLen_] = '\0';

  char *start = line_;
  while(*start == ' ')
    start++;
  size_t len = strlen(start);
  while(len > 0 && start[len - 1] == ' ')
    start[--len] = '\0';
  if(len == 0) return false;

  for(int i = 0; i < kNumKnownCommands; i++) {
    if(equalsIgnoreCase(start, kKnownCommands[i])) {
      memcpy(push_, start, len);
      push_[len] = '\n';
      pushLen_   = (uint8_t)(len + 1);
      pushPos_   = 0;
      locked_    = true;
      lineLen_   = 0;

      Serial1.print("[BT] baud detectado: ");
      Serial1.println(currentBaud_);
      return true;
    }
  }
  return false;
}

// Roda a busca de baud enquanto não travou. Chamada a cada available(), que o
// loop() do main.cpp já chama o tempo todo.
void BluetoothPort::service() {
  if(locked_) return;

  unsigned long now = millis();

  while(Serial1.available() > 0) {
    int c       = Serial1.read();
    lastByteAt_ = now;
    if(c == '\n' || c == '\r') {
      if(lockIfCommand()) return;
      lineLen_ = 0;
    } else if(lineLen_ < sizeof(line_) - 1) {
      line_[lineLen_++] = (char)c;
    } else {
      lineLen_ = 0; // lixo longo demais pra ser comando
    }
  }

  // Comando enviado sem quebra de linha: fecha a linha quando os bytes param.
  if(lineLen_ > 0 && now - lastByteAt_ >= kLineGapMs) {
    if(lockIfCommand()) return;
    lineLen_ = 0;
  }

  if(now - switchedAt_ >= kDwellMs) nextBaud();
}

int BluetoothPort::available() {
  service();
  if(!locked_) return 0;
  return (pushLen_ - pushPos_) + Serial1.available();
}

int BluetoothPort::read() {
  if(pushPos_ < pushLen_) {
    int c = (uint8_t)push_[pushPos_++];
    if(pushPos_ >= pushLen_) pushPos_ = pushLen_ = 0;
    return c;
  }
  return locked_ ? Serial1.read() : -1;
}

int BluetoothPort::peek() {
  if(pushPos_ < pushLen_) return (uint8_t)push_[pushPos_];
  return locked_ ? Serial1.peek() : -1;
}

size_t BluetoothPort::write(uint8_t byte) { return Serial1.write(byte); }

size_t BluetoothPort::write(const uint8_t *buffer, size_t size) {
  return Serial1.write(buffer, size);
}

void BluetoothPort::flush() { Serial1.flush(); }

// -----------------------------------------------------------------------------
// Modo de diagnóstico (só entra se compilar com -DBT_DEBUG no platformio.ini).
// Percorre vários baud rates, mandando "BAUD xxxx" a cada segundo, e devolve em
// hexadecimal todo byte recebido. Prende o robô aqui de propósito.
// -----------------------------------------------------------------------------
#ifdef BT_DEBUG
static void bluetoothBringUp() {
  const unsigned long bauds[] = {9600,   19200,  38400,  57600,
                                 115200, 230400, 460800, 921600};
  const int           count   = sizeof(bauds) / sizeof(bauds[0]);

  while(true) {
    for(int i = 0; i < count; i++) {
      Serial1.end();
      Serial1.begin(bauds[i]);

      unsigned long start     = millis();
      unsigned long lastHello = 0;

      while(millis() - start < 8000) {
        if(millis() - lastHello >= 1000) {
          lastHello = millis();
          Serial1.print("BAUD ");
          Serial1.println(bauds[i]);
        }

        while(Serial1.available() > 0) {
          int b = Serial1.read();
          Serial1.print("RX(");
          Serial1.print(bauds[i]);
          Serial1.print("): 0x");
          if(b < 16) Serial1.print('0');
          Serial1.println(b, HEX);
        }
      }
    }
  }
}
#endif

void bluetoothBegin(unsigned long baud) {
#ifdef BT_DEBUG
  (void)baud;
  bluetoothBringUp();
#else
  NuSerial.begin(baud);
#endif
}