// Arcade Game

#include <LedControl.h>

#define DIN 11
#define CS 10
#define CLK 13
#define BUZZER 3
#define JOY_X A0
#define JOY_SW 4


// TODO 1


LedControl lc = LedControl(DIN, CLK, CS, 1);

// Trophy icon
byte trophy[8] = {
  B01000010,
  B01111110,
  B01000010,
  B01000010,
  B00111100,
  B00011000,
  B01111110,
  B01111110
};

// Sad face
byte sad[8] = {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10011001,
  B10100101,
  B01000010,
  B00111100
};

// Player
int playerX = 2;
const int PLAYER_Y = 7;
const int PLAYER_SHOOT_Y = 6;

// Bullets
#define MAX_BULLETS 20
struct Bullet {
  int x, y;
  bool active;
  bool fromPlayer;
  unsigned long lastMoveTime;
};
Bullet bullets[MAX_BULLETS];

// Bugs
#define MAX_BUGS 4
struct Bug {
  int x, y;
  int health;
  bool alive;
};
Bug bugs[MAX_BUGS];

int killCount = 0;
int damageTaken = 0;
bool isGameOver = false;
bool isGameWon = false;

unsigned long lastShotTime = 0;
unsigned long lastBugMove = 0;
unsigned long lastPlayerShot = 0;
unsigned long lastGameTick = 0;
unsigned long lastMusicNoteTime = 0;
int musicIndex = 0;

int melody[] = {262, 294, 330, 392, 330, 294};
int melodyLength = sizeof(melody) / sizeof(int);

const int playerBulletSpeed = 100;
const int bugBulletSpeed = 200;
const int bugBulletCount = 3; // Change this to control how many bullets each bug shoots


// TODO 2


//TODO 3


void resetGame() {
  lc.clearDisplay(0);
  playerX = 2;
  killCount = 0;
  damageTaken = 0;
  isGameOver = false;
  isGameWon = false;
  musicIndex = 0;
  lastMusicNoteTime = millis();

  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_BUGS; i++) {
    bugs[i].alive = false;
    bugs[i].health = BugHealth;
  }
}

void waitForRestart() {
  if (isGameOver) {
    for (int i = 0; i < 8; i++) lc.setRow(0, i, sad[i]);
  }
  if (isGameWon) {
    for (int i = 0; i < 8; i++) lc.setRow(0, i, trophy[i]);
    playWinMusic();
  }

  while (digitalRead(JOY_SW) == HIGH) {
    if (isGameOver) playSadMusic();
    if (isGameWon) playWinMusic();
  }
  delay(300);
  resetGame();
}

void drawEverything() {
  lc.clearDisplay(0);

  lc.setLed(0, PLAYER_Y, playerX, true);
  lc.setLed(0, PLAYER_Y, playerX + 1, true);
  lc.setLed(0, PLAYER_Y, playerX + 2, true);
  lc.setLed(0, PLAYER_SHOOT_Y, playerX + 1, true);

  for (int i = 0; i < MAX_BUGS; i++) {
    if (bugs[i].alive) {
      lc.setLed(0, bugs[i].y, bugs[i].x, true);
      lc.setLed(0, bugs[i].y, bugs[i].x + 1, true);
      lc.setLed(0, bugs[i].y, bugs[i].x + 2, true);
      lc.setLed(0, bugs[i].y + 1, bugs[i].x + 1, true);
    }
  }

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      lc.setLed(0, bullets[i].y, bullets[i].x, true);
    }
  }
}

void handleJoystick() {
  int xVal = analogRead(JOY_X);
  if (xVal < 400 && playerX > 0) playerX--;
  if (xVal > 600 && playerX < 5) playerX++;
}

void shootPlayerBullet() {
  if (millis() - lastPlayerShot > 400) {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i] = {playerX + 1, PLAYER_SHOOT_Y - 1, true, true, millis()};
        tone(BUZZER, 880, 50);
        lastPlayerShot = millis();
        break;
      }
    }
  }
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;
    int speed = bullets[i].fromPlayer ? playerBulletSpeed : bugBulletSpeed;
    if (millis() - bullets[i].lastMoveTime >= speed) {
      bullets[i].y += bullets[i].fromPlayer ? -1 : 1;
      bullets[i].lastMoveTime = millis();

      if (bullets[i].y < 0 || bullets[i].y > 7) {
        bullets[i].active = false;
        continue;
      }

      if (bullets[i].fromPlayer) {
        for (int j = 0; j < MAX_BUGS; j++) {
          if (bugs[j].alive && bullets[i].x >= bugs[j].x && bullets[i].x <= bugs[j].x + 2 && bullets[i].y == bugs[j].y + 1) {
            bullets[i].active = false;
            bugs[j].health--;
            tone(BUZZER, 700, 50);
            if (bugs[j].health == 0) {
              bugs[j].alive = false;
              killCount++;
              tone(BUZZER, 300, 150);
              if (killCount >= NoOfBugsToWin) {
                isGameWon = true;
              }
            }
          }
        }
      } else {
        if (bullets[i].y == PLAYER_SHOOT_Y && bullets[i].x >= playerX && bullets[i].x <= playerX + 2) {
          bullets[i].active = false;
          damageTaken++;
          tone(BUZZER, 200, 100);
          if (damageTaken >= UserHealth) isGameOver = true;
        }
      }
    }
  }
}

void updateBugs() {
  if (millis() - lastBugMove > 1000) {
    for (int i = 0; i < MAX_BUGS; i++) {
      if (bugs[i].alive) {
        bugs[i].y++;
        if (bugs[i].y >= 6) bugs[i].y = 0;

        if (random(0, 10) > 6) {
          int bulletsShot = 0;
          for (int b = 0; b < MAX_BULLETS && bulletsShot < bugBulletCount; b++) {
            if (!bullets[b].active) {
              bullets[b] = {bugs[i].x + 1, bugs[i].y + 2, true, false, millis()};
              bulletsShot++;
            }
          }
        }
      }
    }
    lastBugMove = millis();
  }

  for (int i = 0; i < MAX_BUGS; i++) {
    if (!bugs[i].alive && random(0, 100) > 95) {
      bugs[i] = {random(0, 6), 0, 2, true};
    }
  }
}

void playGameMusic() {
  if (millis() - lastMusicNoteTime > 300) {
    tone(BUZZER, melody[musicIndex], 150);
    musicIndex = (musicIndex + 1) % melodyLength;
    lastMusicNoteTime = millis();
  }
}

void playSadMusic() {
  tone(BUZZER, 150, 200);
  delay(200);
}

void playWinMusic() {
  tone(BUZZER, 523, 150); delay(150);
  tone(BUZZER, 659, 150); delay(150);
  tone(BUZZER, 784, 300); delay(300);
}
