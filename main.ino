#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <PT2258.h>
#include <Wire.h>

// TM1628 IC - 7 segment display
const int TM1628_DIO_PIN = 9;    // Data In/Out
const int TM1628_SCLK_PIN = 10;  // Serial Clock
const int TM1628_STB_PIN = 11;   // Strobe

// PT2258 IC - Volume controller
const int PT2258_CLK = A5;
const int PT2258_SDA = A4;
const int AUDIO_OUT_ENABLE = A3;

// LCD PINS
const int rs = 2, bl = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;

// ROTARY PINS
const int rotaryD1 = A0, rotaryD2 = A1;
int rotaryLastState, rotaryStepDrop = 0;

// CONFIGS
const bool EEPROM_ENABLED = true;
const int brightnessStep = 5;   // Amount to increase/decrease brightness
const int maxBrightness = 255;  // Maximum PWM value
const int minBrightness = 0;    // Minimum PWM value

// Button States
const int BTN_INITIAL = 1;
const int BTN_PRESSED = 2;
const int BTN_RELEASED = 3;

//
const int MODE_BTN_INPUT = 12;
const int MODE_BTN_INPUT_ANALOG = A2;
int MODE_ANALOG_BTN_STATE = BTN_INITIAL;
int MODE_ANALOG_BTN_VALUE = 0;
int MODE_BTN_STATE = BTN_INITIAL;

const int MODE_PT2258_MASTER = 0;
const int MODE_PT2258_SB = 2;
const int MODE_PT2258_FL = 4;
const int MODE_PT2258_FR = 6;
const int MODE_PT2258_BL = 8;
const int MODE_PT2258_BR = 10;
const int MODE_PT2258_CN = 12;
const int MODE_LCD_BL = 14;

const int PT2258_FR = 1;
const int PT2258_FL = 2;
const int PT2258_BR = 3;
const int PT2258_CN = 4;
const int PT2258_SB = 5;
const int PT2258_BL = 6;

const int MAX_MODE = 14, MIN_MODE = 0;
int currentMode = MODE_PT2258_MASTER;

int MODE_PT2258_MASTER_VAL = 0;
int MODE_PT2258_SB_VAL = 0;
int MODE_PT2258_FL_VAL = 0;
int MODE_PT2258_FR_VAL = 0;
int MODE_PT2258_BL_VAL = 0;
int MODE_PT2258_BR_VAL = 0;
int MODE_PT2258_CN_VAL = 0;
int MODE_LCD_BL_VAL = 0;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
PT2258 pt2258(0x88);

const uint8_t digitValues[10] = {
  0b11101011,  // 0
  0b00101000,  // 1
  0b01110011,  // 2
  0b01111010,  // 3
  0b10111000,  // 4
  0b11011010,  // 5
  0b11011011,  // 6
  0b01101000,  // 7
  0b11111011,  // 8
  0b11111010   // 9
};

byte bl_char_0[8] = { B11001, B11011, B11111, B11110, B11100, B11100, B01111, B00111 };
byte tl_char_0[8] = { B00111, B01111, B11100, B11000, B11000, B11000, B11000, B11001 };
byte tr_char_0[8] = { B11100, B11110, B00111, B00111, B01111, B11111, B11011, B10011 };
byte br_char_0[8] = { B10011, B00011, B00011, B00011, B00011, B00111, B11110, B11100 };

byte tl_char_1[8] = { B00001, B00001, B00111, B00111, B00001, B00001, B00001, B00001 };
byte tr_char_1[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 };
byte bl_char_1[8] = { B00001, B00001, B00001, B00001, B00001, B00001, B00111, B00111 };
byte br_char_1[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B11100, B11100 };

byte tr_char_2[8] = { B11000, B11110, B11111, B00111, B00011, B00011, B00011, B00111 };
byte tl_char_2[8] = { B00011, B00111, B11111, B11100, B11000, B00000, B00000, B00000 };
byte bl_char_2[8] = { B00000, B00000, B00001, B00011, B00111, B01110, B11111, B11111 };
byte br_char_2[8] = { B01110, B11100, B11000, B10000, B00000, B00000, B11111, B11111 };

byte bl_char_3[8] = { B00000, B00000, B00000, B00000, B00000, B11000, B11111, B01111 };
byte tl_char_3[8] = { B11111, B11111, B00000, B00000, B00000, B00000, B00001, B00001 };
byte tr_char_3[8] = { B11111, B11111, B00011, B00011, B00111, B01110, B11100, B11100 };
byte br_char_3[8] = { B01110, B00111, B00011, B00011, B00011, B00111, B11110, B11100 };

byte bl_char_4[8] = { B11100, B11111, B01111, B00000, B00000, B00000, B00000, B00000 };
byte tl_char_4[8] = { B00000, B00000, B00000, B00001, B00011, B00111, B01110, B11100 };
byte tr_char_4[8] = { B00110, B01110, B11110, B11110, B11110, B01110, B01110, B01110 };
byte br_char_4[8] = { B01110, B11111, B11111, B01110, B01110, B01110, B01110, B01110 };

byte tl_char_5[8] = { B11111, B11111, B11000, B11000, B11000, B11000, B11111, B11111 };
byte tr_char_5[8] = { B11111, B11111, B00000, B00000, B00000, B00000, B11110, B11111 };
byte bl_char_5[8] = { B00000, B00000, B00000, B00000, B11000, B11100, B01111, B00111 };
byte br_char_5[8] = { B00011, B00011, B00011, B00011, B00011, B00110, B11110, B11100 };

byte tr_char_6[8] = { B00000, B11110, B11110, B00000, B00000, B00000, B00000, B11100 };
byte tl_char_6[8] = { B00000, B00001, B00011, B00111, B01110, B11100, B11100, B11111 };
byte bl_char_6[8] = { B11111, B11110, B11100, B11100, B11110, B01111, B00111, B00011 };
byte br_char_6[8] = { B11110, B00111, B00011, B00011, B00111, B11111, B11110, B11100 };

byte bl_char_7[8] = { B00111, B00111, B00111, B00111, B00111, B00111, B00111, B00111 };
byte tl_char_7[8] = { B11111, B11111, B00000, B00000, B00000, B00001, B00011, B00111 };
byte tr_char_7[8] = { B11111, B11111, B00111, B01110, B11100, B11000, B10000, B00000 };
byte br_char_7[8] = { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000 };

byte tr_char_8[8] = { B11100, B11110, B00111, B01011, B10011, B00011, B00111, B11110 };
byte tl_char_8[8] = { B00111, B01111, B11100, B11000, B11000, B11001, B11100, B01111 };
byte br_char_8[8] = { B11110, B00111, B01011, B10011, B00011, B00111, B11110, B11100 };
byte bl_char_8[8] = { B01111, B11100, B11000, B11000, B11001, B11100, B01111, B00011 };

byte br_char_9[8] = { B11111, B00011, B00111, B01110, B11100, B11000, B10000, B00000 };
byte tl_char_9[8] = { B00111, B01111, B11100, B11000, B11000, B11000, B11100, B01111 };
byte tr_char_9[8] = { B11100, B11110, B00111, B00011, B00011, B00011, B00011, B11111 };
byte bl_char_9[8] = { B00111, B00000, B00000, B00000, B00000, B00111, B00111, B00000 };

byte* num_chars[10][4] = {
  {
    tl_char_0,
    tr_char_0,
    bl_char_0,
    br_char_0,
  },
  {
    tl_char_1,
    tr_char_1,
    bl_char_1,
    br_char_1,
  },
  {
    tl_char_2,
    tr_char_2,
    bl_char_2,
    br_char_2,
  },
  {
    tl_char_3,
    tr_char_3,
    bl_char_3,
    br_char_3,
  },
  {
    tl_char_4,
    tr_char_4,
    bl_char_4,
    br_char_4,
  },
  {
    tl_char_5,
    tr_char_5,
    bl_char_5,
    br_char_5,
  },
  {
    tl_char_6,
    tr_char_6,
    bl_char_6,
    br_char_6,
  },
  {
    tl_char_7,
    tr_char_7,
    bl_char_7,
    br_char_7,
  },
  {
    tl_char_8,
    tr_char_8,
    bl_char_8,
    br_char_8,
  },
  {
    tl_char_9,
    tr_char_9,
    bl_char_9,
    br_char_9,
  },
};

void setup() {
  initEEPROM();
  initTM1628();
  initLCD();
  initRotary();

  pinMode(MODE_BTN_INPUT, INPUT_PULLUP);
  pinMode(AUDIO_OUT_ENABLE, OUTPUT);
  pinMode(MODE_BTN_INPUT_ANALOG, INPUT_PULLUP);
  pinMode(13, OUTPUT);

  Wire.begin();

  pt2258.begin();
  // pt2258.volumeAll(70);  // at the beginning the volume is by default at 100%. Set the desired volume at startup before un-muting next
  pt2258.mute(false);  // the mute is active when the device powers up. Un-mute it to ear the sound

  digitalWrite(AUDIO_OUT_ENABLE, HIGH);
  displayMode();
}

void loop() {
  handleRotaryChange();
  handleModeChange();
  handleAnalogInputChange();
}

void displayModeValue(String title, int value) {
  lcd.setCursor(0, 0);
  lcd.print("          ");
  lcd.setCursor(0, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);
  lcd.print(value);
  lcd.print("%   ");
  displayTM1628(value);
  displayLCDCustomNumber(value);
  sendDataTM1628(6, digitValues[(currentMode == 0 ? 0 : currentMode / 2) + 1]);
}

void displayLCDCustomNumber(int num) {
  int colAddress = 15;
  int charAddress = 0;
  int digitCount = 0;

  do {
    int lastDigit = num % 10;
    lcd.createChar(charAddress++, num_chars[lastDigit][1]);
    lcd.createChar(charAddress++, num_chars[lastDigit][3]);
    lcd.createChar(charAddress++, num_chars[lastDigit][0]);
    lcd.createChar(charAddress++, num_chars[lastDigit][2]);

    lcd.setCursor(colAddress, 0);
    lcd.write(byte(charAddress - 4));
    lcd.setCursor(colAddress, 1);
    lcd.write(byte(charAddress - 3));

    lcd.setCursor(colAddress - 1, 0);
    lcd.write(byte(charAddress - 2));
    lcd.setCursor(colAddress - 1, 1);
    lcd.write(byte(charAddress - 1));

    if(num == 100 && digitCount == 0) charAddress = 0; 

    colAddress -= 2;
    digitCount += 2;

    num = num - lastDigit;
    num = num / 10;
  } while (num > 0);

  for (int i = digitCount; i < 6; i++) {
    lcd.setCursor(15 - i, 0);
    lcd.print(" ");
    lcd.setCursor(15 - i, 1);
    lcd.print(" ");
  }
}

void decMode() {
  if (currentMode <= MIN_MODE) currentMode = MAX_MODE;
  else currentMode -= 2;
  displayMode();
}

void incMode() {
  if (currentMode >= MAX_MODE) currentMode = MIN_MODE;
  else currentMode += 2;
  displayMode();
}

void setVolumePT2258() {
  pt2258.volume(PT2258_FR, map(MODE_PT2258_MASTER_VAL, 0, 100, 0, MODE_PT2258_FR_VAL));
  pt2258.volume(PT2258_FL, map(MODE_PT2258_MASTER_VAL, 0, 100, 0, MODE_PT2258_FL_VAL));
  pt2258.volume(PT2258_BR, map(MODE_PT2258_MASTER_VAL, 0, 100, 0, MODE_PT2258_BR_VAL));
  pt2258.volume(PT2258_BL, map(MODE_PT2258_MASTER_VAL, 0, 100, 0, MODE_PT2258_BL_VAL));
  pt2258.volume(PT2258_CN, map(MODE_PT2258_MASTER_VAL, 0, 100, 0, MODE_PT2258_CN_VAL));
  pt2258.volume(PT2258_SB, map(MODE_PT2258_MASTER_VAL, 0, 100, 0, MODE_PT2258_SB_VAL));
}

void mute(bool mute) {
  digitalWrite(AUDIO_OUT_ENABLE, mute ? LOW : HIGH);
}

void displayMode() {
  switch (currentMode) {
    case MODE_LCD_BL:
      // control brightness
      displayModeValue("Brightness", getBrightnessValue());
      break;

    case MODE_PT2258_MASTER:
      displayModeValue("Master", MODE_PT2258_MASTER_VAL * 20 / 100);
      break;

    case MODE_PT2258_BL:
      // BACK LEFT
      displayModeValue("Back L", MODE_PT2258_BL_VAL * 20 / 100);
      // pt2258.volume(uint8_t channel, uint8_t volume)
      break;

    case MODE_PT2258_BR:
      // BACK RIGHT
      displayModeValue("Back R", MODE_PT2258_BR_VAL * 20 / 100);
      break;

    case MODE_PT2258_CN:
      // CENTER
      displayModeValue("Center", MODE_PT2258_CN_VAL * 20 / 100);
      break;

    case MODE_PT2258_FL:
      // FRONT LEFT
      displayModeValue("Front L", MODE_PT2258_FL_VAL * 20 / 100);
      break;

    case MODE_PT2258_FR:
      // FRONT RIGHT
      displayModeValue("Front R", MODE_PT2258_FR_VAL * 20 / 100);
      break;

    case MODE_PT2258_SB:
      // SUB WOOFER
      displayModeValue("Sub woofer", MODE_PT2258_SB_VAL * 20 / 100);
      break;
  }

  setVolumePT2258();
}

void updateValues(bool increase) {
  switch (currentMode) {
    case MODE_LCD_BL:
      // control brightness
      MODE_LCD_BL_VAL = increase ? min(MODE_LCD_BL_VAL + brightnessStep, maxBrightness) : max(MODE_LCD_BL_VAL - brightnessStep, minBrightness);
      analogWrite(bl, MODE_LCD_BL_VAL);
      if (EEPROM_ENABLED) EEPROM.put(MODE_LCD_BL, MODE_LCD_BL_VAL);
      break;

    case MODE_PT2258_MASTER:
      // MASTER VOLUME
      MODE_PT2258_MASTER_VAL = increase ? min(MODE_PT2258_MASTER_VAL + 5, 100) : max(MODE_PT2258_MASTER_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_MASTER, MODE_PT2258_MASTER_VAL);
      break;

    case MODE_PT2258_BL:
      // BACK LEFT
      MODE_PT2258_BL_VAL = increase ? min(MODE_PT2258_BL_VAL + 5, 100) : max(MODE_PT2258_BL_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_BL, MODE_PT2258_BL_VAL);
      break;

    case MODE_PT2258_BR:
      // BACK RIGHT
      MODE_PT2258_BR_VAL = increase ? min(MODE_PT2258_BR_VAL + 5, 100) : max(MODE_PT2258_BR_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_BR, MODE_PT2258_BR_VAL);
      break;

    case MODE_PT2258_CN:
      // CENTER
      MODE_PT2258_CN_VAL = increase ? min(MODE_PT2258_CN_VAL + 5, 100) : max(MODE_PT2258_CN_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_CN, MODE_PT2258_CN_VAL);
      break;

    case MODE_PT2258_FL:
      // FRONT LEFT
      MODE_PT2258_FL_VAL = increase ? min(MODE_PT2258_FL_VAL + 5, 100) : max(MODE_PT2258_FL_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_FL, MODE_PT2258_FL_VAL);
      break;

    case MODE_PT2258_FR:
      // FRONT RIGHT
      MODE_PT2258_FR_VAL = increase ? min(MODE_PT2258_FR_VAL + 5, 100) : max(MODE_PT2258_FR_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_FR, MODE_PT2258_FR_VAL);
      break;

    case MODE_PT2258_SB:
      // SUB WOOFER
      MODE_PT2258_SB_VAL = increase ? min(MODE_PT2258_SB_VAL + 5, 100) : max(MODE_PT2258_SB_VAL - 5, 0);
      if (EEPROM_ENABLED) EEPROM.put(MODE_PT2258_SB, MODE_PT2258_SB_VAL);
      break;
  }

  displayMode();
}

void handleRotaryChange() {
  int clk = digitalRead(rotaryD1);

  if (clk != rotaryLastState) {
    if (rotaryStepDrop == 1) updateValues(clk != digitalRead(rotaryD2));
    rotaryStepDrop = rotaryStepDrop == 0 ? 1 : 0;
  }

  rotaryLastState = clk;
}

void handleModeChange() {
  const int currentState = digitalRead(MODE_BTN_INPUT);

  if (currentState == LOW || MODE_BTN_STATE == BTN_PRESSED) {
    MODE_BTN_STATE = MODE_BTN_STATE == BTN_PRESSED && currentState == HIGH ? BTN_RELEASED : BTN_PRESSED;
  }

  if (MODE_BTN_STATE == BTN_RELEASED) {
    MODE_BTN_STATE = BTN_INITIAL;
    // LOGIC FOR CHANGING MODE
    incMode();
    displayMode();
  }
}

void handleAnalogInputChange() {
  const int currentState = analogRead(MODE_BTN_INPUT_ANALOG);

  if (currentState <= 850 || MODE_ANALOG_BTN_STATE == BTN_PRESSED) {
    MODE_ANALOG_BTN_STATE = MODE_ANALOG_BTN_STATE == BTN_PRESSED && currentState >= 850 ? BTN_RELEASED : BTN_PRESSED;
  }

  if (MODE_ANALOG_BTN_STATE == BTN_PRESSED) MODE_ANALOG_BTN_VALUE = currentState;

  if (MODE_ANALOG_BTN_STATE == BTN_RELEASED) {
    MODE_ANALOG_BTN_STATE = BTN_INITIAL;
    // LOGIC FOR CHANGING MODE

    if (MODE_ANALOG_BTN_VALUE < 200) {
      updateValues(false);
    } else if (MODE_ANALOG_BTN_VALUE > 500 && MODE_ANALOG_BTN_VALUE <= 600) {
      incMode();
    } else if (MODE_ANALOG_BTN_VALUE > 600 && MODE_ANALOG_BTN_VALUE <= 750) {
      updateValues(true);
    } else if (MODE_ANALOG_BTN_VALUE > 750 && MODE_ANALOG_BTN_VALUE <= 850) {
      // currentMode = MODE_PT2258_MASTER;
      // displayMode();
      mute(digitalRead(AUDIO_OUT_ENABLE) == HIGH);
    }
  }
}

// LOW LEVEL

void sendTM1628(byte data) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(TM1628_SCLK_PIN, LOW);
    digitalWrite(TM1628_DIO_PIN, data & 1 ? HIGH : LOW);
    data >>= 1;
    digitalWrite(TM1628_SCLK_PIN, HIGH);
  }
}

byte receiveTM1628() {
  byte temp = 0;

  // Pull-up on
  pinMode(TM1628_DIO_PIN, INPUT);
  digitalWrite(TM1628_DIO_PIN, HIGH);

  for (int i = 0; i < 8; i++) {
    temp >>= 1;

    digitalWrite(TM1628_SCLK_PIN, LOW);

    if (digitalRead(TM1628_DIO_PIN)) temp |= 0x80;

    digitalWrite(TM1628_SCLK_PIN, HIGH);
  }

  // Pull-up off
  pinMode(TM1628_DIO_PIN, OUTPUT);
  digitalWrite(TM1628_DIO_PIN, LOW);

  return temp;
}

// MID LEVEL

void sendCommandTM1628(byte data) {
  digitalWrite(TM1628_STB_PIN, LOW);
  sendTM1628(data);
  digitalWrite(TM1628_STB_PIN, HIGH);
}

void sendDataTM1628(byte addr, byte data) {
  sendCommandTM1628(0x44);
  digitalWrite(TM1628_STB_PIN, LOW);
  sendTM1628(0xC0 | addr);
  sendTM1628(data);
  digitalWrite(TM1628_STB_PIN, HIGH);
}

void clearTM1628() {
  for (int i = 0; i < 14; i++) {
    sendDataTM1628(i, 0x00);
  }
}

// Function to turn on all segments to display "8" on all digits
void displayTM1628(int num) {
  int addressList[4] = { 0, 2, 4, 6 };
  int address = 0;

  do {
    int lastDigit = num % 10;
    sendDataTM1628(addressList[address++], digitValues[lastDigit]);
    num = num - lastDigit;
    num = num / 10;
  } while (num > 0);


  for (int i = 0 + address; i < 4; i++) {
    sendDataTM1628(addressList[i], 0x00);
  }
}

int getBrightnessValue() {
  const int maxVal = 100;
  return MODE_LCD_BL_VAL * maxVal / 255;
}

// Function to initialize the TM1628
void initTM1628() {
  pinMode(TM1628_DIO_PIN, OUTPUT);
  pinMode(TM1628_SCLK_PIN, OUTPUT);
  pinMode(TM1628_STB_PIN, OUTPUT);

  sendCommandTM1628(0x40);  // Set auto-increment mode for data
  sendCommandTM1628(0x89);  // Turn on display, pulse width = 1/16
  clearTM1628();
}

void initRotary() {
  pinMode(rotaryD1, INPUT_PULLUP);
  pinMode(rotaryD2, INPUT_PULLUP);
  rotaryLastState = digitalRead(rotaryD1);
}

void initLCD() {
  lcd.begin(16, 2);
  pinMode(bl, OUTPUT);
  analogWrite(bl, MODE_LCD_BL_VAL);
}

void initEEPROM() {
  if (!EEPROM_ENABLED) return;
  EEPROM.get(MODE_LCD_BL, MODE_LCD_BL_VAL);
  EEPROM.get(MODE_PT2258_BL, MODE_PT2258_BL_VAL);
  EEPROM.get(MODE_PT2258_BR, MODE_PT2258_BR_VAL);
  EEPROM.get(MODE_PT2258_CN, MODE_PT2258_CN_VAL);
  EEPROM.get(MODE_PT2258_FL, MODE_PT2258_FL_VAL);
  EEPROM.get(MODE_PT2258_FR, MODE_PT2258_FR_VAL);
  EEPROM.get(MODE_PT2258_MASTER, MODE_PT2258_MASTER_VAL);
  EEPROM.get(MODE_PT2258_SB, MODE_PT2258_SB_VAL);
}
