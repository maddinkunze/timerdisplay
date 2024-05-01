#include <esp_now.h>

#define PIN_BUTTON 34 // TODO: find correct pin
#define PIN_MODE1 35 // TODO: find correct pin
#define PIN_MODE2 36 // TODO: find correct pin

#define TAG_BUTTONW_PREFIX "mad760:btn:"
#define TAG_BUTTONW_START TAG_BUTTONW_PREFIX "start"
#define TAG_BUTTONW_STOP TAG_BUTTONW_PREFIX "stop"
#define TAG_BUTTONW_RESET TAG_BUTTONW_PREFIX "reset"

const uint8_t BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

typedef enum {
  BM_START,
  BM_STOP,
  BM_RESET
} button_mode_t;
button_mode_t mode = BM_START;

bool pressed = false;
unsigned long pressedTime = 0;

void sendButtonPressed() {
  char* message = nullptr;
  switch (mode) {
    case BM_START:
      message = TAG_BUTTONW_START;
    case BM_STOP:
      message = TAG_BUTTONW_STOP;
    case BM_RESET:
      message = TAG_BUTTONW_RESET;
  }

  if (!message) { return; }

  esp_now_send(BROADCAST_ADDRESS, message, strlen(message));
}

void IRAM_ATTR buttonPressed() {
  // debouncing guard clause
  unsigned long lastPressedTime = pressedTime;
  pressedTime = millis();
  if (pressedTime - lastPressedTime < 20) { return; }

  pressed = true;
}

void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(PIN_BUTTON, buttonPressed, FALLING);

  pinMode(PIN_MODE1, INPUT_PULLUP);
  pinMode(PIN_MODE2, INPUT_PULLUP);
  bool mode1 = digitalRead(PIN_MODE1);
  bool mode2 = digitalReal(PIN_MODE2);
  if (mode1 && mode2) {
    mode = BM_START;
  } else if (mode1) {
    mode = BM_STOP;
  } else {
    mode = BM_RESET;
  }
}

void loop() {
  delay(10);
  if (!pressed) { return; }

  sendButtonPressed();
  delay(50);
}