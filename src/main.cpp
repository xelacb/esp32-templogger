#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <Wire.h>

#define SLEEP_TIME_MINUTES 15 // Time to sleep between measurements
#define uS_TO_MIN_FACTOR                                                       \
  60000000ULL // Conversion factor for micro seconds to minutes

Adafruit_BME280 bme;
RTC_DATA_ATTR int bootCount = 0;

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    // If mounting fails, it will be formatted (true)
    Serial.println("LittleFS Mount Failed");
  }

  // Measure data every boot
  float temp = 0.0, hum = 0.0, pres = 0.0;
  bool sensorFound = false;

  if (bme.begin(0x76) || bme.begin(0x77)) {
    sensorFound = true;
    temp = bme.readTemperature();
    hum = bme.readHumidity();
    pres = bme.readPressure() / 100.0F;
  }

  // Save to LittleFS
  File dataFile = LittleFS.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    if (bootCount == 0) {
      // Write header if first boot (or file was just created)
      if (dataFile.size() == 0) {
        dataFile.println("BootCount,Temp(*C),Humidity(%),Pressure(hPa)");
      }
    }

    bootCount++;
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

  // Wait max 5 seconds to see if Serial (USB) connects
  // For ESP32-C3 native USB, Serial evaluates to true when connected to a host
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
    Serial.println("  x - Exit and go to Deep Sleep");
    Serial.print("Waiting ");
    Serial.print(SLEEP_TIME_MINUTES);
    Serial.println(" minutes between measurements.");
    Serial.println("Waiting 10 seconds for user input...");

    unsigned long interactiveStart = millis();
    bool stayInteractive = true;

    while (stayInteractive && millis() - interactiveStart < 10000) {
      if (Serial.available()) {
        char c = Serial.read();
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
          // Clear data
          LittleFS.remove("/data.csv");
          Serial.println("Data cleared.");
          bootCount = 0;
          interactiveStart = millis(); // Reset timeout
        } else if (c == 'x') {
          stayInteractive = false;
          Serial.println("Going to sleep immediately...");
        }
      }
      delay(10);
    }
  }

  // Go to deep sleep
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_MINUTES * uS_TO_MIN_FACTOR);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
  // Not used during deep sleep operation
}
