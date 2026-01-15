#include <dht_nonblocking.h>
#include <LiquidCrystal.h>

// LCD Pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// DHT11 setup
#define DHT_SENSOR_TYPE DHT_TYPE_11
const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Timing
unsigned long previousDisplayMillis = 0;
bool showCelsius = true;

// Reading state
float lastTempC = NAN;
float lastHum = NAN;
bool hasValidReading = false;

bool measure_environment(float *temperature, float *humidity) {
  static unsigned long measurement_timestamp = millis();

  if (millis() - measurement_timestamp > 3000ul) {
    if (dht_sensor.measure(temperature, humidity)) {
      measurement_timestamp = millis();
      return true;
    }
  }

  return false;
}

void setup() {
  // TODO: 1
}

void loop() {
  float tempC, hum;

  // Try reading sensor
  if (measure_environment(&tempC, &hum)) {
    lastTempC = tempC;
    lastHum = hum;
    hasValidReading = true;
  }

  // Display logic every 4 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousDisplayMillis >= 4000) {
    previousDisplayMillis = currentMillis;
    lcd.clear();

    if (!hasValidReading) {
      lcd.setCursor(0, 0);
      lcd.print("Reading...");
    } else {
      showCelsius = !showCelsius;

      float tempToDisplay = showCelsius ? lastTempC : (lastTempC * 9.0 / 5.0 + 32.0);
      char unit = showCelsius ? 'C' : 'F';

      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(tempToDisplay, 1);
      lcd.print((char)223);
      lcd.print(unit);

      lcd.setCursor(0, 1);
      lcd.print("Hum: ");
      lcd.print(lastHum, 1);
      lcd.print("%");
    }
  }

  delay(100);  // reduce flicker
}
