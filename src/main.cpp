#include <Arduino.h>
#include "config.h"

static const uint8_t LEFT_BLINKERS[] = { LEFT_FRONT_BLINKER, LEFT_REAR_BLINKER };
static const uint8_t RIGHT_BLINKERS[] = { RIGHT_FRONT_BLINKER, RIGHT_REAR_BLINKER };
static const uint8_t ALL_BLINKERS[] = {
  LEFT_FRONT_BLINKER,
  LEFT_REAR_BLINKER,
  RIGHT_FRONT_BLINKER,
  RIGHT_REAR_BLINKER
};

void setGroupState(const uint8_t pins[], size_t count, bool enabled) {
  for (size_t i = 0; i < count; ++i) {
    digitalWrite(pins[i], enabled ? HIGH : LOW);
  }
}

void blinkGroup(const uint8_t pins[], size_t count) {
    setGroupState(pins, count, true);
    delay(500);
    setGroupState(pins, count, false);
    delay(500);
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  pinMode(LEFT_FRONT_BLINKER, OUTPUT);
  pinMode(LEFT_REAR_BLINKER, OUTPUT);
  pinMode(RIGHT_FRONT_BLINKER, OUTPUT);
  pinMode(RIGHT_REAR_BLINKER, OUTPUT);

  pinMode(LEFT_BLINKER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BLINKER_SWITCH_PIN, INPUT_PULLUP);

  setGroupState(ALL_BLINKERS, 4, false);

  delay(1000);
  Serial.println("ESP32-C3 blinker controller ready");
  Serial.print("Left switch pin: ");
  Serial.println(LEFT_BLINKER_SWITCH_PIN);
  Serial.print("Right switch pin: ");
  Serial.println(RIGHT_BLINKER_SWITCH_PIN);
}

void loop() {
  const bool leftSwitchOn = (digitalRead(LEFT_BLINKER_SWITCH_PIN) == LOW);
  const bool rightSwitchOn = (digitalRead(RIGHT_BLINKER_SWITCH_PIN) == LOW);

  if (leftSwitchOn && rightSwitchOn) {
    Serial.println("All blinkers ON");
    blinkGroup(ALL_BLINKERS, 4);
  } else if (leftSwitchOn) {
    Serial.println("Left blinkers ON");
    blinkGroup(LEFT_BLINKERS, 2);
  } else if (rightSwitchOn) {
    Serial.println("Right blinkers ON");
    blinkGroup(RIGHT_BLINKERS, 2);
  } else {
    setGroupState(ALL_BLINKERS, 4, false);
    delay(50);
  }
}
