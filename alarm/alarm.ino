#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

// LCD Pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

RTC_DS1307 rtc;

#define BUZZER_PIN 6
#define BUTTON_PIN 4

// Set alarm time (24-hour format)
int alarmHour = 1;
int alarmMinute = 20;
int selectedTone = 3; // Choose from 1–5

bool alarmTriggered = false;
bool alarmStopped = false;

// For blinking colon
bool colonVisible = true;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 1000; // milliseconds

void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  lcd.begin(16, 2);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 🕒 Set RTC to compile time (DO THIS ONCE, then comment out!)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // Handle colon blinking every 1 sec
  if (millis() - lastBlinkTime >= blinkInterval) {
    colonVisible = !colonVisible;
    lastBlinkTime = millis();
  }

  // Display time with blinking colon
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  printTimeAMPM(now.hour(), now.minute(), colonVisible);

  // Display alarm (no blinking colon)
  lcd.setCursor(0, 1);
  if (alarmStopped) {
    lcd.print("Alarm: OFF         ");
  } else {
    lcd.print("Alarm: ");
    printTimeAMPM(alarmHour, alarmMinute, true); // colon always ON for alarm
  }

  // Handle button press to stop alarm
  if (digitalRead(BUTTON_PIN) == LOW && alarmTriggered) {
    alarmStopped = true;
    noTone(BUZZER_PIN);
    lcd.setCursor(0, 1);
    lcd.print("Alarm: OFF         ");
  }

  // Trigger alarm
  if (!alarmStopped && now.hour() == alarmHour && now.minute() == alarmMinute) {
    if (!alarmTriggered) {
      alarmTriggered = true;
      lcd.setCursor(0, 1);
      lcd.print("ALARM RINGING!!!   ");
    }
    playSelectedTone(selectedTone); // continuous
  }

  // Reset after minute passes
  if (now.minute() != alarmMinute) {
    alarmTriggered = false;
    alarmStopped = false;
    noTone(BUZZER_PIN);
  }

  delay(100);
}

// 🕒 Print time with optional colon blinking
void printTimeAMPM(int hour, int minute, bool showColon) {
  int displayHour = hour % 12;
  if (displayHour == 0) displayHour = 12;

  if (displayHour < 10) lcd.print('0');
  lcd.print(displayHour);

  lcd.print(showColon ? ":" : " "); // Blinking colon

  if (minute < 10) lcd.print('0');
  lcd.print(minute);

  lcd.print(hour < 12 ? " AM" : " PM");
  lcd.print(" ");
}

void playSelectedTone(int toneNumber) {
  switch (toneNumber) {
    case 1: tone(BUZZER_PIN, 1000); break;
    case 2: tone(BUZZER_PIN, 880); delay(300); noTone(BUZZER_PIN); delay(200); tone(BUZZER_PIN, 988); delay(300); break;
    case 3: playMelody3(); break;
    case 4: tone(BUZZER_PIN, 440); delay(300); tone(BUZZER_PIN, 554); delay(300); tone(BUZZER_PIN, 659); delay(300); break;
    case 5: tone(BUZZER_PIN, 1000); delay(100); noTone(BUZZER_PIN); delay(100); break;
    default: tone(BUZZER_PIN, 1000); break;
  }
}

void playMelody3() {
  tone(BUZZER_PIN, 660, 100); delay(150);
  tone(BUZZER_PIN, 660, 100); delay(300);
  tone(BUZZER_PIN, 660, 100); delay(300);
  tone(BUZZER_PIN, 510, 100); delay(100);
  tone(BUZZER_PIN, 660, 100); delay(300);
  tone(BUZZER_PIN, 770, 100); delay(550);
  tone(BUZZER_PIN, 380, 100); delay(500);
}
