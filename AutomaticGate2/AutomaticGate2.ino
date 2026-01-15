#include <Servo.h>

// Pin Definitions
#define TRIG_PIN 9
#define ECHO_PIN 8
#define SERVO_PIN 3
#define RED_LED 5
#define GREEN_LED 6
#define BUZZER_PIN 4

Servo gateServo;
long duration;
int distance;

// Todo 1


void loop() {
  distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance < 15) {
    // Car approaching
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    openGateSmoothlyWithBeep(); // Gate opens with beep

    // Car passes
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    delay(3000); // Allow time for car to pass

    // Gate closes with beep
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    closeGateSmoothlyWithBeep();

    // Gate is now closed
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  delay(200);
}

// Todo 2


void openGateSmoothlyWithBeep() {
  unsigned long prevServoTime = 0;
  unsigned long prevBeepTime = 0;
  int pos = 0;
  bool beeping = false;

  unsigned long startTime = millis();

  while (pos <= 90 || millis() - startTime <= 3500) {
    unsigned long currentTime = millis();

    // Move servo smoothly
    if (pos <= 90 && currentTime - prevServoTime >= 40) {
      gateServo.write(pos);
      pos++;
      prevServoTime = currentTime;
    }

    // Buzzer beeping pattern
    if (currentTime - prevBeepTime >= 300) {
      if (beeping) {
        noTone(BUZZER_PIN);
        beeping = false;
      } else {
        tone(BUZZER_PIN, 1000);
        beeping = true;
      }
      prevBeepTime = currentTime;
    }
  }

  noTone(BUZZER_PIN);
}

void closeGateSmoothlyWithBeep() {
  unsigned long prevServoTime = 0;
  unsigned long prevBeepTime = 0;
  int pos = 90;
  bool beeping = false;

  unsigned long startTime = millis();

  while (pos >= 0 || millis() - startTime <= 3500) {
    unsigned long currentTime = millis();

    // Move servo smoothly
    if (pos >= 0 && currentTime - prevServoTime >= 40) {
      gateServo.write(pos);
      pos--;
      prevServoTime = currentTime;
    }

    // Buzzer beeping pattern
    if (currentTime - prevBeepTime >= 300) {
      if (beeping) {
        noTone(BUZZER_PIN);
        beeping = false;
      } else {
        tone(BUZZER_PIN, 1000);
        beeping = true;
      }
      prevBeepTime = currentTime;
    }
  }

  noTone(BUZZER_PIN);
}
