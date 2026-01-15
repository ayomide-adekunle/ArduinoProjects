#include <Wire.h>
#include <math.h>

// =====================================================
// Gesture Mouse (Serial Edition) — UNO R3
// MPU-6050 GYRO rate -> dx/dy over Serial: dx,dy,L,R\n
// Buttons: D2 (left), D3 (right) using INPUT_PULLUP
// LEDs: D4 move, D5 left, D6 right
//
// Notes:
// - Cursor moves only while rotating (much less runaway).
// - Hold BOTH buttons ~0.8s to re-calibrate gyro offsets.
// =====================================================

// -------- MPU-6050 --------
static const uint8_t MPU_ADDR         = 0x68;
static const uint8_t REG_PWR_MGMT_1   = 0x6B;
static const uint8_t REG_ACCEL_XOUT_H = 0x3B; // read 14 bytes: accel(6) + temp(2) + gyro(6)

// -------- Pins --------
const uint8_t PIN_BTN_LEFT  = 2; // Button left
const uint8_t PIN_BTN_RIGHT = 3; // Button right

const uint8_t PIN_LED_MOVE  = 4; // LED move
const uint8_t PIN_LED_LEFT  = 5; // LED left
const uint8_t PIN_LED_RIGHT = 6; // LED right

// SDA = A4
// SCL = A5

// -------- Update rate --------
const uint16_t UPDATE_HZ = 80;
const uint16_t UPDATE_MS = 1000 / UPDATE_HZ;

// -------- Gyro tuning --------
// Deadzone in deg/sec (increase if still moves when not rotating)
float GYRO_DEADZONE_DPS = 2.0f;

// Gain: bigger = faster cursor
float GYRO_GAIN = 0.28f;

// Max pixels per update
int MAX_STEP = 18;

// Smoothing (higher = smoother, more lag)
const float ALPHA = 0.85f;

// -------- Gyro offsets (calibrated) --------
float gx0 = 0, gy0 = 0, gz0 = 0;
float gxF = 0, gyF = 0, gzF = 0;

// -------- Buttons debounce --------
struct DebouncedButton {
  uint8_t pin;
  bool stableState;
  bool lastReading;
  uint32_t lastChangeMs;
};
DebouncedButton btnL{PIN_BTN_LEFT, HIGH, HIGH, 0};
DebouncedButton btnR{PIN_BTN_RIGHT, HIGH, HIGH, 0};
const uint16_t DEBOUNCE_MS = 25;

// -------- Recenter shortcut --------
uint32_t bothPressedSince = 0;

// -------- Helpers --------
static inline int16_t toInt16(uint8_t hi, uint8_t lo) {
  return (int16_t)((hi << 8) | lo);
}

static inline float softDeadzone(float v, float dz) {
  float a = fabsf(v);
  if (a <= dz) return 0.0f;
  return (v > 0) ? (a - dz) : -(a - dz);
}

static inline void i2cWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

static inline bool i2cReadBytes(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  Wire.requestFrom((int)MPU_ADDR, (int)len, (int)true);
  if (Wire.available() < len) return false;

  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

// Read gyro raw from MPU (14 bytes starting at ACCEL_XOUT_H)
bool readGyroRaw(int16_t &gx, int16_t &gy, int16_t &gz) {
  uint8_t b[14];
  if (!i2cReadBytes(REG_ACCEL_XOUT_H, b, 14)) return false;
  gx = toInt16(b[8],  b[9]);
  gy = toInt16(b[10], b[11]);
  gz = toInt16(b[12], b[13]);
  return true;
}

// Debounce buttons
bool updateDebounced(DebouncedButton &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastReading) {
    b.lastChangeMs = millis();
    b.lastReading = reading;
  }
  if ((millis() - b.lastChangeMs) > DEBOUNCE_MS && b.stableState != reading) {
    b.stableState = reading;
    return true;
  }
  return false;
}

void blinkMoveConfirm() {
  digitalWrite(PIN_LED_MOVE, HIGH); delay(120);
  digitalWrite(PIN_LED_MOVE, LOW);  delay(120);
  digitalWrite(PIN_LED_MOVE, HIGH); delay(120);
  digitalWrite(PIN_LED_MOVE, LOW);
}

// Calibrate gyro offsets while sensor is still
void calibrateGyroOffsets() {
  const int N = 500;
  long sx = 0, sy = 0, sz = 0;
  int got = 0;

  for (int i = 0; i < N; i++) {
    int16_t gx, gy, gz;
    if (readGyroRaw(gx, gy, gz)) {
      sx += gx; sy += gy; sz += gz;
      got++;
    }
    delay(3);
  }

  if (got > 0) {
    gx0 = (float)sx / got;
    gy0 = (float)sy / got;
    gz0 = (float)sz / got;
    gxF = gyF = gzF = 0; // reset filters
  }
}

void setup() {
  pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);

  pinMode(PIN_LED_MOVE, OUTPUT);
  pinMode(PIN_LED_LEFT, OUTPUT);
  pinMode(PIN_LED_RIGHT, OUTPUT);

  Serial.begin(9600);
  Wire.begin();

  // Wake MPU-6050
  i2cWrite(REG_PWR_MGMT_1, 0x00);
  delay(150);

  // Keep sensor still for a second
  calibrateGyroOffsets();
}

void loop() {
  static uint32_t lastMs = 0;
  uint32_t now = millis();
  if (now - lastMs < UPDATE_MS) return;
  lastMs = now;

  // Buttons
  updateDebounced(btnL);
  updateDebounced(btnR);

  bool leftPressed  = (btnL.stableState == LOW);
  bool rightPressed = (btnR.stableState == LOW);

  // Recenter: hold BOTH buttons ~0.8s (keep sensor still)
  if (leftPressed && rightPressed) {
    if (bothPressedSince == 0) bothPressedSince = millis();
    if (millis() - bothPressedSince > 800) {
      calibrateGyroOffsets();
      bothPressedSince = 0;
      blinkMoveConfirm();
    }
  } else {
    bothPressedSince = 0;
  }

  // Read gyro
  int16_t gxRaw, gyRaw, gzRaw;
  if (!readGyroRaw(gxRaw, gyRaw, gzRaw)) return;

  // Convert to deg/sec (typical default sensitivity: 131 LSB/(deg/s) at ±250 dps)
  float gx = ((float)gxRaw - gx0) / 131.0f;
  float gy = ((float)gyRaw - gy0) / 131.0f;
  float gz = ((float)gzRaw - gz0) / 131.0f;

  // Smooth
  gxF = ALPHA * gxF + (1.0f - ALPHA) * gx;
  gyF = ALPHA * gyF + (1.0f - ALPHA) * gy;
  gzF = ALPHA * gzF + (1.0f - ALPHA) * gz;

  // Map rotation to cursor movement
  // Common mapping:
  //  - yaw (gz) -> left/right
  //  - pitch rate (gx) -> up/down
  float vx = softDeadzone(gzF, GYRO_DEADZONE_DPS);
  float vy = softDeadzone(gxF, GYRO_DEADZONE_DPS);

  int dx = (int)lroundf(vx * GYRO_GAIN);
  int dy = (int)lroundf(vy * GYRO_GAIN);

  // If it still goes the "wrong way", flip signs:
  // dx = -dx;
  // dy = -dy;

  dx = constrain(dx, -MAX_STEP, MAX_STEP);
  dy = constrain(dy, -MAX_STEP, MAX_STEP);

  // LEDs
  digitalWrite(PIN_LED_LEFT,  leftPressed ? HIGH : LOW);
  digitalWrite(PIN_LED_RIGHT, rightPressed ? HIGH : LOW);
  digitalWrite(PIN_LED_MOVE,  (dx != 0 || dy != 0) ? HIGH : LOW);

  // Serial: dx,dy,L,R
  Serial.print(dx); Serial.print(',');
  Serial.print(dy); Serial.print(',');
  Serial.print(leftPressed ? 1 : 0); Serial.print(',');
  Serial.print(rightPressed ? 1 : 0);
  Serial.print('\n');
}
