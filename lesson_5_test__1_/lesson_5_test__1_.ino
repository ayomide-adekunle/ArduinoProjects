// Define pin connections
#define BUZZER_PIN 9
int LED_PINS[7] = {2, 3, 4, 5, 6, 7, 8}; // LEDs for each note

// Frequencies for solfa notes (in Hz)
 // TODO 1





// Note and LED mapping
// TODO 2





// Define note durations
#define WHOLE 1000
#define HALF  500
#define QUARTER 250
#define EIGHTH 125

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  for (int i = 0; i < 7; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
}

// Function to play a note with an LED
void playNote(int noteIndex, int duration) {
  tone(BUZZER_PIN, NOTES[noteIndex]);
  digitalWrite(LED_PINS[noteIndex], HIGH);
  delay(duration);
  noTone(BUZZER_PIN);
  digitalWrite(LED_PINS[noteIndex], LOW);
  delay(50);
}

// Full Twinkle Twinkle Little Star
// Function to play full Twinkle Twinkle melody
void playTwinkleTwinkle() {
  int melody[] = {
    0, 0, 4, 4, 5, 5, 4,  // Twinkle, twinkle, little star
    3, 3, 2, 2, 1, 1, 0,  // How I wonder what you are
    4, 4, 3, 3, 2, 2, 1,  // Up above the world so high
    4, 4, 3, 3, 2, 2, 1,  // Like a diamond in the sky
    0, 0, 4, 4, 5, 5, 4,  // Twinkle, twinkle, little star
    3, 3, 2, 2, 1, 1, 0   // How I wonder what you are
  };

  int durations[] = {
    HALF, HALF, HALF, HALF, HALF, HALF, WHOLE, 
    HALF, HALF, HALF, HALF, HALF, HALF, WHOLE,
    HALF, HALF, HALF, HALF, HALF, HALF, WHOLE,
    HALF, HALF, HALF, HALF, HALF, HALF, WHOLE,
    HALF, HALF, HALF, HALF, HALF, HALF, WHOLE,
    HALF, HALF, HALF, HALF, HALF, HALF, WHOLE
  };

  for (int i = 0; i < 42; i++) {
    playNote(melody[i], durations[i]);
  }
  delay(500);
}


// Full Mary Had a Little Lamb Melody
void playMaryHadALittleLamb() {
  int melody[] = {
    2, 1, 0, 1, 2, 2, 2,  // Mary had a little lamb
    1, 1, 1, 2, 4, 4,     // Little lamb, little lamb
    2, 1, 0, 1, 2, 2, 2,  // Mary had a little lamb
    1, 1, 1, 2, 1, 0 ,     // Its fleece was white as snow

    2, 1, 0, 1, 2, 2, 2,  // Everywhere that Mary went
    1, 1, 1, 2, 4, 4,     // Mary went, Mary went
    2, 1, 0, 1, 2, 2, 2,  // Everywhere that Mary went
    1, 1, 1, 2, 1, 0     // The lamb was sure to go
  };

  int durations[] = {
    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF, 
    QUARTER, QUARTER, HALF, QUARTER, QUARTER, HALF,
    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF, 
    QUARTER, QUARTER, HALF, QUARTER, QUARTER, WHOLE,

    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF, 
    QUARTER, QUARTER, HALF, QUARTER, QUARTER, HALF,
    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF, 
    QUARTER, QUARTER, HALF, QUARTER, QUARTER, WHOLE
  };

  for (int i = 0; i < 52; i++) {
    playNote(melody[i], durations[i]);
  }
  delay(500);
}


// Full London Bridge Melody
void playLondonBridge() {
  int melody[] = {


    4, 5, 4, 3, 2, 3, 4,  // London Bridge is falling down
    1, 2, 3, 2, 3, 4,     // Falling down, falling down
    4, 5, 4, 3, 2, 3, 4,  // London Bridge is falling down
    1, 4, 2, 0,

    4, 5, 4, 3, 2, 3, 4,  // London Bridge is falling down
    1, 2, 3, 2, 3, 4,     // Falling down, falling down
    4, 5, 4, 3, 2, 3, 4,  // London Bridge is falling down
    1, 4, 2, 0 
  };

  int durations[] = {
    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF,
    QUARTER, QUARTER, HALF, QUARTER, QUARTER, HALF,
    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF,
    HALF, HALF, QUARTER,  HALF,

    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF,
    QUARTER, QUARTER, HALF, QUARTER, QUARTER, HALF,
    QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, QUARTER, HALF,
    HALF, HALF, QUARTER,  HALF,
  };

  for (int i = 0; i < 48; i++) {
    playNote(melody[i], durations[i]);
  }
  delay(500);
}


void loop() {

  // TODO 3
  


}
