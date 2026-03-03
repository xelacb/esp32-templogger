# 🌡️ ESP32-C3 Langzeit-Temperaturlogger

Ein ultra-stromsparender Temperatur-, Luftfeuchtigkeits- und Luftdruck-Logger, der für den monatelangen bis jahrelangen autarken Betrieb mit einem einzigen 18650-Akku ausgelegt ist. Das gesamte Projekt ist so kompakt konzipiert, dass es in ein wasserdichtes Geocaching-Röhrchen (PETling) passt.

## 🌟 Features
* **Extreme Akkulaufzeit:** Durch die konsequente Nutzung des "Deep Sleep"-Modus des ESP32-C3 verbraucht das Gerät im Ruhezustand fast keinen Strom (Laufzeit > 1 Jahr möglich).
* **Interner Speicher:** Daten werden sicher im internen Flash-Speicher (LittleFS) des Mikrocontrollers abgelegt.
* **Offline-Auslesen per USB:** Kein stromfressendes WLAN nötig. Der Datenabruf erfolgt einfach per USB-C Kabel über ein serielles Menü.
* **Wasserdicht & Robust:** Perfekt für den Outdoor-Einsatz durch das PETling-Gehäuse.

---

## 🛠️ Hardware & Stückliste



Folgende Bauteile werden für den Aufbau benötigt:

1. **Mikrocontroller:** `Seeed Studio XIAO ESP32C3`
   * *Funktion:* Das Gehirn des Projekts. Steuert den Sensor, speichert die Daten und wickelt die USB-Kommunikation ab.
2. **Sensor:** `BME280 Breakout Board (3.3V Version)`
   * *Funktion:* Misst hochpräzise Temperatur, Luftfeuchtigkeit und Luftdruck. Wichtig: Es muss die 3.3V Variante sein (oft lila Platine).
3. **Akku:** `18650 Li-Ion Zelle (Geschützt / mit PCB) mit Z-Lötfahnen`
   * *Funktion:* Die Energiequelle (ca. 3.7V, z.B. 3500mAh). Die Lötfahnen sind wichtig, damit der Akku ohne klobigen Batteriehalter direkt in das Rohr passt.
4. **Gehäuse:** `PETling (PET-Rohling) ca. 13-15cm Länge`
   * *Funktion:* Absolut wasserdichtes und bruchsicheres Gehäuse.

---

## 🔌 Anschlussbelegung & Schaltplan

Der Aufbau ist sehr simpel und erfordert keine zusätzlichen Widerstände.



### Sensor (BME280) an Mikrocontroller (XIAO C3)
Die Kommunikation erfolgt über den I2C-Bus.

| BME280 Pin | XIAO ESP32C3 Pin | Kabelfarbe (Empfehlung) |
| :--- | :--- | :--- |
| **VCC** (Strom) | **3V3** | Rot |
| **GND** (Masse) | **GND** | Schwarz |
| **SCL** (Clock) | **D5** (SCL) | Gelb |
| **SDA** (Daten) | **D4** (SDA) | Blau / Grün |

### Akku an Mikrocontroller
Der XIAO C3 hat auf der **Rückseite** zwei kleine Lötpads für den direkten Batterieanschluss.
* **Akku Plus (+)** ➔ Pad **BAT+** auf der Rückseite des XIAO
* **Akku Minus (-)** ➔ Pad **BAT-** auf der Rückseite des XIAO

---

## 🏗️ Zusammenbau-Anleitung

1. **Akku vorbereiten:** Löte ein rotes Kabel an die Plus-Lötfahne und ein schwarzes an die Minus-Lötfahne des 18650-Akkus. **Achtung:** Wickle danach Isolierband um die Pole, um Kurzschlüsse mit der Elektronik zu vermeiden!
2. **Akku verbinden:** Löte die beiden Kabel an die `BAT+` und `BAT-` Pads auf der Unterseite des XIAO ESP32C3.
3. **Sensor anlöten:** Verbinde die 4 Pins des BME280 über kurze, flexible Kabel (Litze) mit den entsprechenden Pins auf der Oberseite des XIAO (siehe Tabelle).
4. **Einbau:** Schiebe den Akku mit dem Minuspol voran in den PETling. Der Mikrocontroller und der Sensor finden oben am Flaschenhals Platz. Achte darauf, dass der Sensor nicht direkt den Akku oder Chip berührt (Wärmeentwicklung verfälscht die Messung).

---

## 💻 Software & Flashen (PlatformIO)

Das Projekt wird mit **Visual Studio Code** und der Erweiterung **PlatformIO** kompiliert.

1. Erstelle ein neues Projekt für das Board `Seeed Studio XIAO ESP32C3` und dem Framework `Arduino`.
2. Ersetze den Inhalt der `platformio.ini` mit folgendem Code:
   ```ini
   [env:seeed_xiao_esp32c3]
   platform = espressif32
   board = seeed_xiao_esp32c3
   framework = arduino
   monitor_speed = 115200
   lib_deps = 
       adafruit/Adafruit BME280 Library
       adafruit/Adafruit Unified Sensor
