#include <LiquidCrystal.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pins
const int waterSensorPin = A0;
const int buzzerPin = 9;
const int redLED = 8;
const int yellowLED = 7;
const int greenLED = 6;

// 🔧 CALIBRATION VALUES (CHANGE THESE!)
const int DRY_VALUE = 180;   // value when sensor is dry
const int WET_VALUE = 350;   // value when fully submerged

// Alert thresholds (%)
const int SAFE_LEVEL = 40;
const int WARNING_LEVEL = 60;

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("Flood Alert Sys");
  delay(2000);
  lcd.clear();
}

void loop() {
  int rawValue = readWaterLevel();

  int percent = map(rawValue, DRY_VALUE, WET_VALUE, 0, 100);
  percent = constrain(percent, 0, 100);

  // LCD line 1
  lcd.setCursor(0, 0);
  lcd.print("Water Level:   ");
  lcd.setCursor(13, 0);
  lcd.print(percent);
  lcd.print("%");

  // Decide state
  if (percent < SAFE_LEVEL) {
    safeMode();
  }
  else if (percent < WARNING_LEVEL) {
    warningMode();
  }
  else {
    floodMode();
  }

  delay(500);
}

// -------- MODES --------

void safeMode() {
  lcd.setCursor(0, 1);
  lcd.print("Status: SAFE   ");
  digitalWrite(greenLED, HIGH);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  noTone(buzzerPin);
}

void warningMode() {
  lcd.setCursor(0, 1);
  lcd.print("Status: WARNING");
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, HIGH);
  digitalWrite(redLED, LOW);
  tone(buzzerPin, 1000, 200);
}

void floodMode() {
  lcd.setCursor(0, 1);
  lcd.print("!! FLOOD ALERT ");
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);

  digitalWrite(redLED, HIGH);
  tone(buzzerPin, 2000);
  delay(200);
  digitalWrite(redLED, LOW);
  delay(200);
}

// -------- SENSOR AVERAGING --------

int readWaterLevel() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(waterSensorPin);
    delay(10);
  }
  return sum / 10;
}
