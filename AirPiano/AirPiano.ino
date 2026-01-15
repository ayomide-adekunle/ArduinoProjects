#include <LiquidCrystal.h>
#include <LedControl.h>

// ---------- CONFIG ----------
#define SHOW_BOTH_NAMES false
// If true: LCD line1 shows "Do (C4)"; if false: shows only Solfa ("Do").

// ---------- Pins ----------
const byte PIN_TRIG = 6;
const byte PIN_ECHO = 5;
const byte PIN_BUZZ = 3;

// MAX7219 (DIN, CLK, CS)
const byte PIN_DIN = 12, PIN_CLK = 11, PIN_CS = 10;
LedControl lc(PIN_DIN, PIN_CLK, PIN_CS, 1); // one device

// LCD parallel pins (RS, E, D4, D5, D6, D7)
const byte LCD_RS = 7;
const byte LCD_EN = 8;
const byte LCD_D4 = 9;
const byte LCD_D5 = 4;
const byte LCD_D6 = 2;
const byte LCD_D7 = 13;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ---------- Notes (C major, C4..C5) ----------
const char* NOTE_NAMES[8]  = {"C4","D4","E4","F4","G4","A4","B4","C5"};
const char* SOLFA_NAMES[8] = {"Do","Re","Mi","Fa","So","La","Ti","Do"};
const uint16_t NOTE_FREQ[8] = {262, 294, 330, 349, 392, 440, 494, 523};

// ---------- Distance -> note mapping (cm) ----------
const float DIST_MIN_CM = 5.0;   // closer than this = top note
const float DIST_MAX_CM = 35.0;  // farther than this = silent

// Smoothing
const uint8_t AVG_N = 5;
float dist_buf[AVG_N];
uint8_t buf_i = 0;
bool buf_filled = false;

// Debounce note changes
int current_note = -1;
int stable_note = -1;
unsigned long note_change_ms = 0;
const unsigned long NOTE_DEBOUNCE_MS = 120;

// Display update pacing
unsigned long last_update_ms = 0;
const unsigned long UPDATE_INTERVAL_MS = 40;

float readDistanceCM() {
  // Trigger 10us pulse
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Read echo with timeout
  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000UL); // ~30ms
  if (duration == 0) return NAN;

  // Convert to cm
  float cm = duration / 58.2; // standard conversion
  return cm;
}

// Map distance to note index (-1 for silent)
int distanceToNote(float cm) {
  if (isnan(cm)) return -1;
  if (cm > DIST_MAX_CM) return -1;

  float c = constrain(cm, DIST_MIN_CM, DIST_MAX_CM);
  // Invert so near = higher note
  float t = (c - DIST_MIN_CM) / (DIST_MAX_CM - DIST_MIN_CM); // 0..1
  float inv = 1.0f - t;
  int idx = (int)round(inv * 7.0f); // 0..7
  idx = constrain(idx, 0, 7);
  return idx;
}

// Draw a centered vertical bar whose height reflects proximity (near = taller)
void drawBarFromDistance(float cm) {
  lc.clearDisplay(0);

  if (isnan(cm) || cm > DIST_MAX_CM) return;

  float c = constrain(cm, DIST_MIN_CM, DIST_MAX_CM);
  float inv = 1.0f - (c - DIST_MIN_CM) / (DIST_MAX_CM - DIST_MIN_CM);
  int height = (int)round(inv * 8.0f); // 0..8
  height = constrain(height, 0, 8);

  // columns 3 and 4 as the bar
  for (int r = 0; r < height; r++) {
    int row = 7 - r; // light from bottom up
    lc.setLed(0, row, 3, true);
    lc.setLed(0, row, 4, true);
  }
}

void showNoteOnLCD(int note_idx, float cm) {
  lcd.setCursor(0, 0);
  if (note_idx >= 0) {
    if (SHOW_BOTH_NAMES) {
      // Example: "Do (C4)"
      lcd.print(" ");
      lcd.print(SOLFA_NAMES[note_idx]);
      lcd.print(" (");
      lcd.print(NOTE_NAMES[note_idx]);
      lcd.print(")   ");
    } else {
      lcd.print("Solfa: ");
      lcd.print(SOLFA_NAMES[note_idx]);
      lcd.print("      "); // clear tail
    }
  } else {
    lcd.print("Wave hand to play ");
  }

  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  if (isnan(cm)) {
    lcd.print("--.-");
  } else {
    char buf[8];
    dtostrf(cm, 4, 1, buf); // width=4, 1 decimal
    lcd.print(buf);
  }
  lcd.print(" cm   ");
}

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZ, OUTPUT);

  // LCD parallel init
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Air Piano (Solfa)");
  lcd.setCursor(0, 1);
  lcd.print("Wave to play...");

  // MAX7219
  lc.shutdown(0, false);
  lc.setIntensity(0, 6); // 0..15
  lc.clearDisplay(0);

  // Pre-fill buffer with far distances (silent)
  for (uint8_t i = 0; i < AVG_N; i++) dist_buf[i] = DIST_MAX_CM + 10;
}

void loop() {
  unsigned long now = millis();
  if (now - last_update_ms < UPDATE_INTERVAL_MS) return;
  last_update_ms = now;

  // Read & smooth distance
  float cm = readDistanceCM();
  dist_buf[buf_i++] = cm;
  if (buf_i >= AVG_N) { buf_i = 0; buf_filled = true; }

  float sum = 0; int count = 0;
  for (uint8_t i = 0; i < (buf_filled ? AVG_N : buf_i); i++) {
    if (!isnan(dist_buf[i])) { sum += dist_buf[i]; count++; }
  }
  float cm_avg = (count > 0) ? (sum / count) : NAN;

  // Candidate note from distance
  int cand_note = distanceToNote(cm_avg);

  // Debounce note changes
  if (cand_note != current_note) {
    current_note = cand_note;
    note_change_ms = now;
  }
  if (now - note_change_ms >= NOTE_DEBOUNCE_MS) {
    stable_note = current_note;
  }

  // Visuals
  drawBarFromDistance(cm_avg);
  showNoteOnLCD(stable_note, cm_avg);

  // Sound
  if (stable_note >= 0) {
    tone(PIN_BUZZ, NOTE_FREQ[stable_note]);
  } else {
    noTone(PIN_BUZZ);
  }
}
