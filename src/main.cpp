#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <Wire.h>

#define SLEEP_TIME_MINUTES 15 // Time to sleep between measurements
#define uS_TO_MIN_FACTOR                                                       \
  60000000ULL // Conversion factor for micro seconds to minutes

// XIAO ESP32C3 built-in LED (active LOW)
#define LED_PIN D10
#define FLASH_FULL_THRESHOLD 512 // Minimum free bytes before stopping writes

Adafruit_BME280 bme;

// --- Helper: Persist bootCount via LittleFS (RTC_DATA_ATTR not supported on
// ESP32-C3) ---
int readBootCount() {
  File f = LittleFS.open("/bootcount.txt", FILE_READ);
  if (!f)
    return 0;
  int count = f.parseInt();
  f.close();
  return count;
}

void writeBootCount(int count) {
  File f = LittleFS.open("/bootcount.txt", FILE_WRITE);
  if (f) {
    f.println(count);
    f.close();
  }
}

// --- Helper: Blink LED to signal an error ---
void blinkError(int times) {
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW); // Active LOW
    delay(150);
    digitalWrite(LED_PIN, HIGH);
    delay(150);
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C with explicit pin assignment
  Wire.begin(D4, D5); // SDA, SCL

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    // If mounting fails, it will be formatted (true)
    Serial.println("LittleFS Mount Failed");
  }

  // --- Read sensor ---
  float temp = 0.0, hum = 0.0, pres = 0.0;
  bool sensorFound = false;

  // Try primary address first; only try secondary if the first fails
  if (!bme.begin(0x76)) {
    bme.begin(0x77);
  }
  sensorFound = (bme.sensorID() != 0);

  if (sensorFound) {
    temp = bme.readTemperature();
    hum = bme.readHumidity();
    pres = bme.readPressure() / 100.0F;
  } else {
    blinkError(5); // 5 blinks = sensor error
  }

  // --- Check free flash space ---
  bool flashFull =
      (LittleFS.totalBytes() - LittleFS.usedBytes()) < FLASH_FULL_THRESHOLD;
  if (flashFull) {
    blinkError(10); // 10 blinks = flash full
  }

  // --- Save to LittleFS ---
  if (!flashFull) {
    int bootCount = readBootCount();
    bootCount++;
    writeBootCount(bootCount);

    File dataFile = LittleFS.open("/data.csv", FILE_APPEND);
    if (dataFile) {
      // Write header if file is new / empty
      if (dataFile.size() == 0) {
        dataFile.println("BootCount,Temp(*C),Humidity(%),Pressure(hPa)");
      }

      dataFile.print(bootCount);
      dataFile.print(",");
      if (sensorFound) {
        dataFile.print(temp);
        dataFile.print(",");
        dataFile.print(hum);
        dataFile.print(",");
        dataFile.println(pres);
      } else {
        dataFile.println("ERROR,ERROR,ERROR");
      }
      dataFile.close();
    }
  }

  // --- Interactive serial menu (only when USB is connected) ---
  // Wait max 5 seconds to see if Serial (USB) connects
  unsigned long startWait = millis();
  bool usbConnected = false;
  while (millis() - startWait < 5000) {
    if (Serial) {
      usbConnected = true;
      break;
    }
    delay(10);
  }

  if (usbConnected) {
    Serial.println("\n--- ESP32 TempLogger ---");
    Serial.println("USB connected. Interactive mode active.");
    Serial.println("Commands:");
    Serial.println("  d - Dump data");
    Serial.println("  c - Clear data");
    Serial.println("  s - Show status");
    Serial.println("  x - Exit and go to Deep Sleep");
    Serial.print("Measuring every ");
    Serial.print(SLEEP_TIME_MINUTES);
    Serial.println(" minutes.");
    Serial.println("Waiting 10 seconds for user input...");

    unsigned long interactiveStart = millis();
    bool stayInteractive = true;

    while (stayInteractive && millis() - interactiveStart < 10000) {
      if (Serial.available()) {
        char c = Serial.read();

        // Filter newline / carriage return
        if (c == '\n' || c == '\r')
          continue;

        if (c == 'd') {
          Serial.println("\n--- Data Dump ---");
          File file = LittleFS.open("/data.csv", FILE_READ);
          if (!file) {
            Serial.println("Failed to open file for reading");
          } else {
            while (file.available()) {
              Serial.write(file.read());
            }
            file.close();
          }
          Serial.println("--- End of Data ---");
          interactiveStart = millis(); // Reset timeout
        } else if (c == 'c') {
          LittleFS.remove("/data.csv");
          writeBootCount(0);
          Serial.println("Data and boot counter cleared.");
          interactiveStart = millis(); // Reset timeout
        } else if (c == 's') {
          Serial.print("Flash total: ");
          Serial.print(LittleFS.totalBytes());
          Serial.print(" bytes, used: ");
          Serial.print(LittleFS.usedBytes());
          Serial.print(" bytes, free: ");
          Serial.print(LittleFS.totalBytes() - LittleFS.usedBytes());
          Serial.println(" bytes");
          Serial.print("Boot count: ");
          Serial.println(readBootCount());
          interactiveStart = millis(); // Reset timeout
        } else if (c == 'x') {
          stayInteractive = false;
          Serial.println("Going to sleep immediately...");
        }
      }
      delay(10);
    }
  }

  // --- Go to deep sleep ---
  Serial.flush();
  Wire.end();
  LittleFS.end();
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_MINUTES * uS_TO_MIN_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Not used during deep sleep operation
}
