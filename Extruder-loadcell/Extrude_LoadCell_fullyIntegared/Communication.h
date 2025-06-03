#include "Arduino.h"

void initArd(const HardwareSerial& port, const String id, const String exprespons) {
  unsigned long toc = millis();
  unsigned long tic = 0;

  while (true) {

    if (millis() - tic > 500) {
      Serial.println(id);
      tic = millis();
    }

    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim(); // Remove any whitespace or newline chars

      if (response == exprespons) {
        Serial.println("INIT SUCCESS");
        break;
      }
    }

  }



}
