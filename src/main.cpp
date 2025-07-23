// Proyecto: Adaptador de mando PS2 a Nintendo Switch
// Descripción: Lee las entradas de un mando PS2 (botones, D-Pad, ejes analógicos)
//              y las traduce a comandos compatibles con la biblioteca PsxNewLib para
//              emular un gamepad de Nintendo Switch.
// Fecha: 2025-01-19

#include <Arduino.h>
#include "PsxNewLib.h"
#include "switch_ESP32.h"

// Configuración de depuración: definir para activar logs
#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
  #define LOG_INFO(fmt, ...)    Serial.printf("[INFO] " fmt "\n", ##__VA_ARGS__)
  #define LOG_DEBUG(fmt, ...)   Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
  #define LOG_ERROR(fmt, ...)   Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
  #define LOG_INFO(...)
  #define LOG_DEBUG(...)
  #define LOG_ERROR(...)
#endif

// Pines PS2
#define PS2_DAT    37
#define PS2_CMD    35
#define PS2_CLK    36
#define PS2_SEL     5

// Pines analógicos
#define VOLANTE    15
#define FRENO      18
#define ACELERADOR 17
#define EMBRAGUE   10

// Constantes de valores máximos ADC
#define MAX_THROTTLE 2900
#define MAX_BRAKE    4095

NSGamepad Gamepad;

void sendCommand(uint8_t command) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(PS2_CMD, command & (1 << i));
    digitalWrite(PS2_CLK, LOW);
    delayMicroseconds(10);
    digitalWrite(PS2_CLK, HIGH);
    delayMicroseconds(10);
  }
}

uint8_t readData() {
  uint8_t data = 0;
  for (int i = 0; i < 8; i++) {
    digitalWrite(PS2_CLK, LOW);
    delayMicroseconds(10);
    if (digitalRead(PS2_DAT)) data |= (1 << i);
    digitalWrite(PS2_CLK, HIGH);
    delayMicroseconds(10);
  }
  return data;
}

uint16_t readButtons() {
  digitalWrite(PS2_SEL, LOW);
  sendCommand(0x01);
  sendCommand(0x42);
  uint8_t response = readData();
  if (response != 0x5A) {
    LOG_ERROR("Respuesta incorrecta del mando: 0x%02X", response);
    digitalWrite(PS2_SEL, HIGH);
    return 0xFFFF;
  }
  uint8_t lowByte  = readData();
  uint8_t highByte = readData();
  digitalWrite(PS2_SEL, HIGH);
  return ~(uint16_t)((highByte << 8) | lowByte);
}

void setup() {
  pinMode(PS2_DAT, INPUT_PULLUP);
  pinMode(PS2_CMD, OUTPUT);
  pinMode(PS2_CLK, OUTPUT);
  pinMode(PS2_SEL, OUTPUT);
  pinMode(VOLANTE, INPUT);
  pinMode(FRENO, INPUT);
  pinMode(ACELERADOR, INPUT);
  pinMode(EMBRAGUE, INPUT);

  digitalWrite(PS2_CMD, HIGH);
  digitalWrite(PS2_CLK, HIGH);
  digitalWrite(PS2_SEL, HIGH);

  Gamepad.begin();
  delay(100);
  USB.begin();
  delay(100);
  Serial.begin(115200);
  delay(100);

  LOG_INFO("Mando PS2 inicializado correctamente.");
}

void loop() {
  uint16_t buttons = readButtons();

  // Botones principales
  if (buttons & PSB_CIRCLE)      { Gamepad.press(NSButton_X); }
  else                           { Gamepad.release(NSButton_X); }
  if (buttons & PSB_SQUARE)      { Gamepad.press(NSButton_LeftThrottle); }
  else                           { Gamepad.release(NSButton_LeftThrottle); }
  if (buttons & PSB_L1)          { Gamepad.press(NSButton_Y); }
  else                           { Gamepad.release(NSButton_Y); }
  if (buttons & PSB_R1)          { Gamepad.press(NSButton_B); }
  else                           { Gamepad.release(NSButton_B); }
  if (buttons & PSB_TRIANGLE)    { Gamepad.press(NSButton_A); }
  else                           { Gamepad.release(NSButton_A); }
  if (buttons & PSB_CROSS)       { Gamepad.press(NSButton_RightThrottle); }
  else                           { Gamepad.release(NSButton_RightThrottle); }

  // Mapeo D-Pad
  if      (buttons & PSB_PAD_UP && !(buttons & (PSB_PAD_RIGHT|PSB_PAD_LEFT)))           { Gamepad.dPad(NSGAMEPAD_DPAD_UP); }
  else if (buttons & PSB_PAD_UP && buttons & PSB_PAD_RIGHT)                             { Gamepad.dPad(NSGAMEPAD_DPAD_UP_RIGHT); }
  else if (buttons & PSB_PAD_RIGHT && !(buttons & (PSB_PAD_UP|PSB_PAD_DOWN)))           { Gamepad.dPad(NSGAMEPAD_DPAD_RIGHT); }
  else if (buttons & PSB_PAD_DOWN && buttons & PSB_PAD_RIGHT)                           { Gamepad.dPad(NSGAMEPAD_DPAD_DOWN_RIGHT); }
  else if (buttons & PSB_PAD_DOWN && !(buttons & (PSB_PAD_LEFT|PSB_PAD_RIGHT)))         { Gamepad.dPad(NSGAMEPAD_DPAD_DOWN); }
  else if (buttons & PSB_PAD_DOWN && buttons & PSB_PAD_LEFT)                            { Gamepad.dPad(NSGAMEPAD_DPAD_DOWN_LEFT); }
  else if (buttons & PSB_PAD_LEFT && !(buttons & (PSB_PAD_UP|PSB_PAD_DOWN)))            { Gamepad.dPad(NSGAMEPAD_DPAD_LEFT); }
  else if (buttons & PSB_PAD_UP && buttons & PSB_PAD_LEFT)                              { Gamepad.dPad(NSGAMEPAD_DPAD_UP_LEFT); }
  else                                                                               { Gamepad.dPad(NSGAMEPAD_DPAD_CENTERED); }

  // Joystick izquierdo
  int rawVolante     = analogRead(VOLANTE);
  int rawAcelerador  = analogRead(ACELERADOR);
  int rawFreno       = analogRead(FRENO);
  int rawEmbrague    = analogRead(EMBRAGUE);
  int16_t xAxis      = map(rawVolante, 0, MAX_BRAKE, 0, 255);
  int frenoPct       = map(rawFreno, 0, MAX_BRAKE, 0, 100);
  int aceleradorPct  = map(rawAcelerador, 0, MAX_THROTTLE, 0, 100);
  int16_t yAxis      = (buttons & PSB_PAD_UP)   ? 0 : (buttons & PSB_PAD_DOWN) ? 255 : 127;

  // Frenado gradual
  if (frenoPct > 0) {
    unsigned long pressMs   = frenoPct;
    unsigned long releaseMs = 100 - frenoPct;
    LOG_DEBUG("Freno gradual -> %d%% (press=%lums, release=%lums)", frenoPct, pressMs, releaseMs);
    Gamepad.press(NSButton_LeftThrottle);
    delayMicroseconds(pressMs * 1000UL);
    Gamepad.release(NSButton_LeftThrottle);
    delayMicroseconds(releaseMs * 1000UL);
  } else {
    Gamepad.release(NSButton_LeftThrottle);
  }

  // Aceleración gradual
  if (aceleradorPct > 0) {
    unsigned long pressMs   = aceleradorPct;
    unsigned long releaseMs = 100 - aceleradorPct;
    LOG_DEBUG("Acelerador gradual -> %d%% (press=%lums, release=%lums)", aceleradorPct, pressMs, releaseMs);
    Gamepad.press(NSButton_RightThrottle);
    delayMicroseconds(pressMs * 1000UL);
    Gamepad.release(NSButton_RightThrottle);
    delayMicroseconds(releaseMs * 1000UL);
  } else {
    Gamepad.release(NSButton_RightThrottle);
  }

  Gamepad.leftXAxis(xAxis);
  Gamepad.leftYAxis(yAxis);

  if (!Gamepad.write()) {
    LOG_ERROR("Error al enviar reporte a Switch");
  } else {
    LOG_DEBUG("Reporte enviado correctamente");
  }

  delay(10);
}
