#include <Wire.h>
#include <LedControl.h>


const byte goalMax = 5;
// --------------- MAX7219 setup ---------------
const byte PIN_DIN = 12, PIN_CLK = 11, PIN_CS = 10;
LedControl lc(PIN_DIN, PIN_CLK, PIN_CS, 1); // 1 device

// --------------- Buzzer (optional) ----------
const int PIN_BUZZ = 3;
inline void beep(unsigned f=1200, unsigned d=60){ tone(PIN_BUZZ, f, d); }

// --------------- MPU-6050 (raw I2C) --------
const byte MPU_ADDR = 0x68;
// Axis inversion & swap if your board orientation differs
bool INVERT_X = false;   // set true if left/right feels backwards
bool INVERT_Y = false;   // set true if up/down feels backwards
bool SWAP_XY  = false;   // set true if axes are swapped

// thresholds (raw accel units; 16384 ~= 1 g)
const int16_t TILT_THRESH = 2200;   // deadzone threshold (~0.13 g)
const int16_t STILL_THRESH = 800;   // "still" band for auto recal

// movement pacing
const uint16_t MOVE_DELAY_MS = 140; // ms between steps when leaning

// --------------- Maze data ------------------
// 1 bit = wall, 0 = free. Bit7 is x=0 (left), Bit0 is x=7 (right).
const byte LEVELS[][8] = {
  // Level 0
  { 0xFF, 0x81, 0xBD, 0x81, 0xBD, 0x81, 0xBD, 0xFF },
  // Level 1
  { 0xFF, 0x91, 0xA9, 0x89, 0xB9, 0x81, 0xBD, 0xFF },
  // Level 2
  { 0xFF, 0xA1, 0x93, 0xC1, 0xB9, 0x81, 0xB5, 0xFF }
};
const byte NUM_LEVELS = sizeof(LEVELS) / sizeof(LEVELS[0]);

// start & goal for each level (x,y)
const byte STARTS[][2] = {{1,1},{1,1},{1,1}};
const byte GOALS[][2]  = {{6,6},{6,6},{6,6}};

byte level = 0;
byte px, py;            // player
byte gx, gy;            // goal

// calibration offsets
long ax_off = 0, ay_off = 0;

// timing
unsigned long lastMove = 0;
unsigned long stillStart = 0;
bool stillCounting = false;
bool blink = false;
unsigned long lastBlink = 0;

// --------- Goals / Trophy / Victory ----------
uint8_t goalsHit = 0;   // counts how many goals the player has reached

// Simple 8x8 trophy bitmap
const byte TROPHY[8] = {
  0x3C, // 00111100
  0x42, // 01000010
  0x5A, // 01011010
  0x7E, // 01111110
  0x24, // 00100100 (stem)
  0x24, // 00100100
  0x18, // 00011000 (base)
  0x3C  // 00111100
};

// Victory melody: 16 notes × 250 ms = 4000 ms total
const int VICTORY_MELODY[16] = {
  523, 659, 784, 1047,   // C5 E5 G5 C6
  988, 880, 784, 659,    // B5 A5 G5 E5
  784, 880, 988, 1047,   // G5 A5 B5 C6
  784, 659, 587, 523     // G5 E5 D5 C5
};
const int VICTORY_NOTE_MS = 250; // 16 * 250 = 4000 ms

// --------------- Helpers --------------------
inline byte bitAt(byte x) { return (1 << (7 - x)); } // x:0..7 -> mask

bool isWall(byte x, byte y){
  return (LEVELS[level][y] & bitAt(x)) != 0;
}

void displayBitmap(const byte bmp[8]) {
  for (byte y=0; y<8; y++) lc.setRow(0, y, bmp[y]);
}

void draw(){
  unsigned long now = millis();
  if (now - lastBlink > 250) { blink = !blink; lastBlink = now; }
  for (byte y=0; y<8; y++){
    byte row = LEVELS[level][y];
    if (y == gy && blink) row |= bitAt(gx);     // blink goal
    if (y == py)          row |= bitAt(px);     // player
    lc.setRow(0, y, row);
  }
}

void loadLevel(byte idx){
  level = idx % NUM_LEVELS;
  px = STARTS[level][0]; py = STARTS[level][1];
  gx = GOALS[level][0];  gy = GOALS[level][1];
  lc.clearDisplay(0);
}

void mpuWrite(byte reg, byte val){
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg); Wire.write(val);
  Wire.endTransmission();
}

void readAccelRaw(int16_t &ax, int16_t &ay, int16_t &az){
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (byte)6);
  while (Wire.available() < 6);
  ax = (Wire.read()<<8) | Wire.read();
  ay = (Wire.read()<<8) | Wire.read();
  az = (Wire.read()<<8) | Wire.read();
}

void calibrate(unsigned samples=300){
  long sumx=0, sumy=0; int16_t ax, ay, az;
  for (unsigned i=0;i<samples;i++){
    readAccelRaw(ax,ay,az);
    sumx += ax; sumy += ay;
    delay(3);
  }
  ax_off = sumx / (long)samples;
  ay_off = sumy / (long)samples;
}

void getTilt(int16_t &tx, int16_t &ty){
  int16_t ax, ay, az;
  readAccelRaw(ax,ay,az);
  // subtract offsets
  long x = (long)ax - ax_off;
  long y = (long)ay - ay_off;
  // optional axis swap/invert to match your physical mounting
  long outX = SWAP_XY ? y : x;
  long outY = SWAP_XY ? x : y;
  if (INVERT_X) outX = -outX;
  if (INVERT_Y) outY = -outY;
  tx = (int16_t)outX; ty = (int16_t)outY;
}

void tryMove(byte nx, byte ny){
  if (nx>7 || ny>7) return;
  if (isWall(nx, ny)) return;
  px = nx; py = ny;
  beep(1600, 20);
}

// Show trophy for 4s with victory tune, then restart game
void showTrophyAndRestart(){
  // show trophy
  lc.clearDisplay(0);
  displayBitmap(TROPHY);

  // play 4s victory tune (16 notes × 250ms)
  for (byte i=0; i<16; i++){
    tone(PIN_BUZZ, VICTORY_MELODY[i], VICTORY_NOTE_MS - 10);
    // keep the trophy visible (optionally blink a tiny sparkle)
    delay(VICTORY_NOTE_MS);
  }
  noTone(PIN_BUZZ);

  // restart: reset counters & level
  goalsHit = 0;
  loadLevel(0);
  calibrate();   // quick re-cal so restart feels fair
}

// pick the dominant axis, move once per cooldown
void maybeStep(){
  int16_t tx, ty; getTilt(tx,ty);

  // still-detection for on-the-fly recalibration
  bool still = (abs(tx) < STILL_THRESH && abs(ty) < STILL_THRESH);
  unsigned long now = millis();
  if (still) {
    if (!stillCounting){ stillCounting = true; stillStart = now; }
    else if (now - stillStart > 3000){ calibrate(); stillCounting=false; }
  } else stillCounting = false;

  if (now - lastMove < MOVE_DELAY_MS) return;

  int16_t ax = abs(tx), ay = abs(ty);
  if (ax < TILT_THRESH && ay < TILT_THRESH) return;

  // Dominant direction
  if (ax >= ay){
    if (tx > 0) { if (px < 7) tryMove(px+1, py); }      // right
    else         { if (px > 0) tryMove(px-1, py); }      // left
  } else {
    if (ty > 0) { if (py > 0) tryMove(px, py-1); }       // up
    else         { if (py < 7) tryMove(px, py+1); }      // down
  }
  lastMove = millis();
}

// --------------- Arduino setup/loop ----------
void setup() {
  // Display init
  lc.shutdown(0,false);
  lc.setIntensity(0, 8);   // 0..15
  lc.clearDisplay(0);

  // I2C + MPU init
  Wire.begin();
  mpuWrite(0x6B, 0x00);    // PWR_MGMT_1: wake up
  mpuWrite(0x1C, 0x00);    // ACCEL_CONFIG: +/-2g

  // quick "hello" sweep
  for (byte r=0;r<8;r++){ lc.setRow(0,r,0xFF); delay(30); lc.setRow(0,r,0x00); }

  loadLevel(0);
  calibrate();             // hold flat for ~2s
  beep(1000, 70);
  beep(1500, 70);
}

void loop() {
  maybeStep();
  draw();

  // level complete?
  if (px==gx && py==gy){
    goalsHit++;  // count this goal
    if (goalsHit >= goalMax) {
      showTrophyAndRestart();
    } else {
      // quick win jingle, then next level
      for (int i=0;i<3;i++){ beep(1800+i*200, 90); delay(100); }
      delay(150);
      loadLevel(level+1);  // cycles through levels
    }
  }

  delay(8);
}
