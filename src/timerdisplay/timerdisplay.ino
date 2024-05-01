// RGB Panel GFX Demo example for 16x32 panel
// By Marc MERLIN <marc_soft@merlins.org>
// Contains code (c) Adafruit, license BSD

// WILL NOT FIT on ARDUINO UNO -- requires a Mega, M0 or M4 board

#include <EEPROM.h>
#include <esp_now.h>

#include <GPxMatrix.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#define TAG_BUTTONW_PREFIX "mad760:btn:"
#define TAG_BUTTONW_START TAG_BUTTONW_PREFIX "start"
#define TAG_BUTTONW_STOP TAG_BUTTONW_PREFIX "stop"
#define TAG_BUTTONW_RESET TAG_BUTTONW_PREFIX "reset"

#define P_A    12
#define P_B    16
#define P_C    17
#define P_D    18
#define P_E    5
#define P_CLK  15
#define P_LAT  32
#define P_OE   33

//                        R1, G1, B1, R2, G2, B2  (RGB Pins)
uint8_t listrgbpins[6] = {26, 25, 27, 21, 22, 23};
GPxMatrix *matrix = new GPxMatrix(P_A, P_B, P_C, P_D, P_E, P_CLK, P_LAT, P_OE, true, 128, listrgbpins); 

// Panel Matrix doesn't fully work like Neomatrix (which I originally
// wrote this demo for), so map a few calls to be compatible. The rest
// comes from Adafruit_GFX and works the same on both backends.
#define clear()          fillScreen(0)
#define show()           swapBuffers(true)

// Define matrix width and height.
#define mw 128
#define mh 64

// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for Adafruit::GFX backends that are lacking color()
#define LED_BLACK           0

#define LED_RED_VERYLOW    (3 <<  11)
#define LED_RED_LOW        (7 <<  11)
#define LED_RED_MEDIUM     (15 << 11)
#define LED_RED_HIGH       (31 << 11)

#define LED_GREEN_VERYLOW  (1 <<  5)
#define LED_GREEN_LOW      (15 << 5)
#define LED_GREEN_MEDIUM   (31 << 5)
#define LED_GREEN_HIGH     (63 << 5)

#define LED_BLUE_VERYLOW     3
#define LED_BLUE_LOW         7
#define LED_BLUE_MEDIUM     15
#define LED_BLUE_HIGH       31

#define LED_ORANGE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW     (LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM  (LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH    (LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW (LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW     (LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM  (LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH    (LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW   (LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW       (LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM    (LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH      (LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW  (LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW      (LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM   (LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH     (LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

typedef enum {
  IM_START_STOP = 0,
  IM_START_STOP_RESET = 1,
  IM_TOGGLE = 2,
  IM_TOGGLE_RESET = 3,
  IM_MAX = 4
} interaction_mode_t;

interaction_mode_t mode = IM_START_STOP;

#define PIN_START 35 // Button 1
#define PIN_STOP 34  // Button 2
#define PIN_RESET 39 // Button 3
#define PIN_MODE 24  // mode-select button TODO: find correct pin

unsigned long timeStart = 0;
unsigned long timeEnd = 0;
unsigned long timeCountdownStart = 0;

void printTime(int t) {
  int mil = t % 1000;
  t = t / 1000;
  int sec = t % 60;
  t = t / 60;
  int min = t % 60;

  matrix->setFont(&FreeMonoBold24pt7b);
  matrix->setTextColor(LED_WHITE_HIGH);
  matrix->setCursor(0, 31);
  matrix->printf("%02d", min);
  matrix->setCursor(50, 27);
  matrix->print(":");
  matrix->setCursor(71, 31);
  matrix->printf("%02d", sec);

  matrix->setFont(&FreeMonoBold18pt7b);
  matrix->setTextColor(LED_ORANGE_LOW);
  matrix->setCursor(62, 58);
  matrix->printf("%03d", mil);
}

void printTag(int color, char* text, int textcolor) {
  printTag(color, text, textcolor, 0);
}

void printTag(int color, char* text, int textcolor, int textoffset) {
  matrix->fillRoundRect(3, 35, 58, 26, 4, color);
  matrix->setFont(&FreeSansBold12pt7b);
  matrix->setTextColor(textcolor);
  int16_t tx, ty; uint16_t tw, th;
  matrix->getTextBounds(text, 0, 0, &tx, &tx, &tw, &th);
  matrix->setCursor(32-tw/2+textoffset, 56);
  matrix->print(text);
}

void printCurrentTime() {
  unsigned long actualTimeEnd = timeStart ? (timeEnd ? timeEnd : millis()) : 0;
  printTime(actualTimeEnd - timeStart);
}

void printCurrentTag() {
  if (timeEnd) {
    printTag(LED_BLUE_HIGH, "Time", LED_BLACK, -2);
    return;
  }
  if (timeStart) {
    printTag(LED_GREEN_HIGH, "Go! ", LED_BLACK);
    return;
  }
  if (timeCountdownStart) {
    unsigned long timeNow = millis();
    unsigned long timeCountdown = timeNow - timeCountdownStart;
    if (timeCountdown > 3000) {
      timeStart = timeNow;
      timeCountdownStart = 0;
      printTag(LED_GREEN_HIGH, "Go! ", LED_BLACK);
    } else if (timeCountdown > 2000) {
      printTag(LED_ORANGE_HIGH, "1 ", LED_BLACK);
    } else if (timeCountdown > 1000) {
      printTag(LED_RED_HIGH, "2", LED_BLACK);
    } else {
      printTag(LED_RED_HIGH, "3", LED_BLACK);
    }
  }
}

void loop() {
  matrix->clear();
  printCurrentTime();
  printCurrentTag();
  matrix->show();
}

bool IRAM_ATTR isStarted() {
  return timeStart != 0;
}

bool IRAM_ATTR isCountingDown() {
  return timeCountdownStart != 0;
}

bool IRAM_ATTR isStopped() {
  return timeEnd != 0;
}

bool IRAM_ATTR isRunning() {
  return isStarted() && !isStopped();
}

bool IRAM_ATTR isResetted() {
  return (timeStart == 0) 
      && (timeCountdownStart == 0) 
      && (timeEnd == 0);
}

void IRAM_ATTR timerStart() {
  if (isCountingDown()) { return; }
  timerReset();
  timeCountdownStart = millis();
}

void IRAM_ATTR timerStartIfResetted() {
  if (!isResetted()) { return; }
  timerStart();
}

void IRAM_ATTR timerStop() {
  if (isCountingDown()) { return; }
  if (isStopped()) { return; }
  if (!isStarted()) { return; }
  timeEnd = millis();
}

void IRAM_ATTR timerToggle() {
  if (isStarted()) {
    timerStop();
  } else {
    timerStart();
  }
}

void IRAM_ATTR timerToggleIfResetted() {
  if (isStarted()) {
    timerStop();
  } else if (isResetted()) {
    timerStart();
  }
}

void IRAM_ATTR timerReset() {
  timeCountdownStart = 0;
  timeStart = 0;
  timeEnd = 0;
}

void IRAM_ATTR timerResetIfNotCountingDown() {
  if (isCountingDown()) { return; }
  timerReset();
}


void IRAM_ATTR buttonStartPress() {
  switch (mode) {
    case IM_START_STOP:
      timerStart();
      break;
    case IM_START_STOP_RESET:
      timerStartIfResetted();
      break;
    case IM_TOGGLE:
      timerToggle();
      break;
    case IM_TOGGLE_RESET:
      timerToggleIfResetted();
      break;
  }
}

void IRAM_ATTR buttonStopPress() {
  switch (mode) {
    case IM_START_STOP:
    case IM_START_STOP_RESET:
      timerStop();
  }
}

void IRAM_ATTR buttonResetPress() {
  switch (mode) {
    case IM_START_STOP_RESET:
    case IM_TOGGLE_RESET:
      timerReset();
  }
}


void IRAM_ATTR writeMode() {
  EEPROM.write(0, mode);
}

void IRAM_ATTR verifyMode() {
  if (mode >= IM_MAX) { mode = IM_START_STOP; }
}

void IRAM_ATTR nextMode() {
  timerReset();
  mode++;
  verifyMode();
  writeMode();
}


void IRAM_ATTR buttonModePress() {
  nextMode();
}

void initPins() {
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_STOP, INPUT_PULLUP);
  pinMode(PIN_RESET, INPUT_PULLUP);
  pinMode(PIN_MODE, INPUT_PULLUP);

  attachInterrupt(PIN_START, buttonStartPress, FALLING);
  attachInterrupt(PIN_STOP, buttonStopPress, FALLING);
  attachInterrupt(PIN_RESET, buttonResetPress, FALLING);
  attachInterrupt(PIN_MODE, buttonModePress, FALLING);
}

void wirelessDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
  if (strncmp(data, TAG_BUTTONW_START, len) == 0) {
    buttonStartPress();
  } else if (strncmp(data, TAG_BUTTONW_STOP, len) == 0) {
    buttonStopPress();
  } else if (strncmp(data, TAG_BUTTONW_RESET, len) == 0) {
    buttonResetPress();
  }
}

void initWireless() {
  esp_now_register_recv_cb(wirelessDataReceived);
}

void initDisplay() {
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->clear();
  matrix->show();
}

void initMode() {
  EEPROM.begin(1);
  mode = EEPROM.read(0);
  verifyMode();
}

void setup() {
  initMode();
  initPins();
  initWireless();
  initDisplay();
}