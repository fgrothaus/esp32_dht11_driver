#include "Arduino.h"

const int dhtpin = 4;

void setup() {
  Serial.begin(115200);
  Serial.println("DHT11 Werte auslesen startet...");
}

void loop() {

  pinMode(dhtpin, OUTPUT);
  digitalWrite(dhtpin, LOW); // Pin auf 0V ziehen
  delay(20); // Lt. Datenblatt mindestens 18ms halten, damit der DHT11 weiß, das er antworten soll
  digitalWrite(dhtpin, HIGH); // Flanke hochreißen
  pinMode(dhtpin, INPUT_PULLUP); // Erst, wenn ich die Leitung wieder loslasse, sage ich dem DHT11, dass er antworten soll
  
  delayMicroseconds(40); // Der Sensor braucht 20-40 Microseconds zum Antworten. Deswegen muss der ESP32 so lange mindestens warten auf seine Antwort.

  if (digitalRead(dhtpin) == LOW) {

    while(digitalRead(dhtpin) == LOW); // Kurzes hochpushen auf HIGH vom Sensor, um zu Signalisieren, dass die Daten JETZT kommen
    while(digitalRead(dhtpin) == HIGH);

    uint8_t daten[5] = {0,0,0,0,0}; // Array für die Sensorwerte -> Array von Bytes (uint8_t)

    for (int i = 0; i < 40; i++) {

        while(digitalRead(dhtpin) == LOW); // Jedes Bit startet mit 50µs LOW
        unsigned long start = micros(); // Nach dem LOW ist der Pin auf HIGH => Stoppuhr starten
        while(digitalRead(dhtpin) == HIGH); // Messen der Dauer von HIGH, um zu ermitteln, ob es als 0 oder 1 interpretiert werden muss

        uint8_t index = i / 8; // i ist immer ein Integer. In C++ wird bei der division durch einen Integer immer der Rest abgeschnitten => 0-7 landen in Index 0, 8-15 in Index 1 etc.
        daten[index] <<= 1;    // Schiebt vorhandene Bits im Byte nach links in der Schrittweite
        
        if ((micros() - start) > 50) {
          daten[index] |= 1;   // Setzt das unterste Bit auf 1, falls Puls lang war
        }
      }

    // Checksumme prüfen
    if (daten[4] == ((daten[0] + daten[1] + daten[2] + daten[3]) & 0xFF)) {
      Serial.println("Daten korrekt!");
      Serial.printf("Messung: %d.%d%% Feuchte, %d.%d°C Temperatur\n", 
                    daten[0], daten[1], daten[2], daten[3]);

    } else {
        Serial.println("Checksummen-Fehler! Daten verworfen.");
    }
  }
  delay(5000);
}