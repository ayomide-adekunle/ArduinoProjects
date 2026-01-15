#include <LedControl.h>

#define DIN 11
#define CS 10
#define CLK 13

#define JOY_X A0
#define JOY_Y A1
#define JOY_SW 4 // press the middle of joystick to start a new game
#define BUZZER 3

LedControl lc = LedControl(DIN, CLK, CS, 1);

enum Direction { UP, DOWN, LEFT, RIGHT };
Direction dir = RIGHT;

// TODO 1: 


int snakeX[maxLength];
int snakeY[maxLength];
int snakeLength = 2;

int appleX = 5;
int appleY = 5;

int snakeSpeed = 500;

bool isGameOver = false;
bool isGameWon = false;

// Background music notes
int music[] = {262, 294, 330, 392, 330, 294};
int musicIndex = 0;
unsigned long lastNoteTime = 0;

// Smiley face
byte smiley[8] = {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100
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

byte trophy[8] = {
  B01000010,  // Handles (symmetric)
  B01111110,  // Top of the cup
  B01000010,  // Curved body
  B01000010,  // Curved body
  B00111100,  // Bottom of cup
  B00011000,  // Neck
  B01111110,  // Base
  B01111110   // Bottom base
};

// Dedicated state for each melody
int winIndex = 0;
unsigned long winNoteStart = 0;

int sadIndex = 0;
unsigned long sadNoteStart = 0;


// Extended Win Melody (notes & durations)
int winMelody[] = {
  523, 587, 659, 698, 784, 880, 988, 1046, // Ascending triumphant
  784, 784, 880, 784, 659, 587, 523,      // Bounce-back theme
  659, 698, 784, 880, 988, 1046, 1175     // Build up finale
};

int winNoteDurations[] = {
  150, 150, 150, 150, 150, 150, 150, 300,
  150, 150, 150, 150, 150, 150, 300,
  150, 150, 150, 150, 150, 150, 400
};

const int winMelodyLength = sizeof(winMelody) / sizeof(int);


// TODO 2:


// TODO 3: 



void startGame() {
  snakeLength = 2;
  snakeX[0] = 2; snakeY[0] = 4;
  snakeX[1] = 1; snakeY[1] = 4;
  dir = RIGHT;
  snakeSpeed = 500;
  isGameOver = false;
  isGameWon = false;
  musicIndex = 0;
  lastNoteTime = millis();
  spawnApple();
  lc.clearDisplay(0);
  winIndex = 0;
  winNoteStart = millis();
  sadNoteStart = millis();
  sadIndex = 0;
}

void showFace(byte face[8], int duration) {
  for (int i = 0; i < 8; i++) lc.setRow(0, i, face[i]);
  if (duration > 0) {
    delay(duration);
    lc.clearDisplay(0);
  }
}

void playTone(int freq, int dur) {
  tone(BUZZER, freq, dur);
  delay(dur);
  noTone(BUZZER);
}

void gameOver() {
  isGameOver = true;
  sadIndex = 0;
  sadNoteStart = millis();
}

void playSadMusic() {
  static int sadMelody[] = {196, 174, 155, 130}; // Lower-pitched sad tones
  const int sadMelodyLength = sizeof(sadMelody) / sizeof(int);

  if (millis() - sadNoteStart > 400) {
    noTone(BUZZER);  // Stop previous note
    tone(BUZZER, sadMelody[sadIndex]);
    sadNoteStart = millis();
    sadIndex = (sadIndex + 1) % sadMelodyLength;
  }
}





void winGame() {
  isGameWon = true;
  winIndex = 0;
  winNoteStart = millis();
}

void playWinMusic() {
  if (millis() - winNoteStart > winNoteDurations[winIndex]) {
    noTone(BUZZER);  // Stop the previous note
    tone(BUZZER, winMelody[winIndex]);
    winNoteStart = millis();
    winIndex = (winIndex + 1) % winMelodyLength;
  }
}




void waitForRestart() {
  while (digitalRead(JOY_SW) == HIGH) {
    if (isGameWon) {
      showFace(trophy, 0);
      playWinMusic();
    } else if (isGameOver) {
      showFace(sad, 0);
      playSadMusic();
    }
  }

  delay(300); // Debounce
  showFace(smiley, 1000);
  startGame();
}


void draw() {
  lc.clearDisplay(0);

  // Draw apple
  lc.setLed(0, appleY, appleX, true);

  // Draw snake
  for (int i = 0; i < snakeLength; i++) {
    lc.setLed(0, snakeY[i], snakeX[i], true);
  }
}

void spawnApple() {
  bool valid = false;
  while (!valid) {
    appleX = random(0, 8);
    appleY = random(0, 8);
    valid = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snakeX[i] == appleX && snakeY[i] == appleY) {
        valid = false;
        break;
      }
    }
  }
}

void moveSnake() {
  int newX = snakeX[0];
  int newY = snakeY[0];

  if (dir == UP) newY--;
  else if (dir == DOWN) newY++;
  else if (dir == LEFT) newX--;
  else if (dir == RIGHT) newX++;

  // Wall collision
  if (newX < 0 || newX > 7 || newY < 0 || newY > 7) {
    gameOver();
    return;
  }

  // Self collision
  for (int i = 0; i < snakeLength; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
      gameOver();
      return;
    }
  }

  // Move body
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // Move head
  snakeX[0] = newX;
  snakeY[0] = newY;

  // Apple collision
  if (newX == appleX && newY == appleY) {
    if (snakeLength < maxLength) {
      snakeLength++;
      //snakeSpeed = max(100, snakeSpeed - 30);
      playTone(880, 100);  // Bite sound
      if (snakeLength == maxLength) {
        winGame();
        return;
      }
    }
    spawnApple();
  }
}

void readJoystick() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  if (xVal < 400 && dir != RIGHT) dir = LEFT;
  else if (xVal > 600 && dir != LEFT) 
  {
    dir = RIGHT;
  }
  else if (yVal < 400 && dir != DOWN) 
  {
    dir = UP;
  }
  else if (yVal > 600 && dir != UP) 
  {
    dir = DOWN;
  }
}

void playMusic() {
  if (millis() - lastNoteTime > 250) {
    tone(BUZZER, music[musicIndex], 100);
    musicIndex = (musicIndex + 1) % (sizeof(music) / sizeof(int));
    lastNoteTime = millis();
  }
}
