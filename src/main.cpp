#include <Arduino.h>
#include "config.h"

static const uint8_t LEFT_FRONT = LEFT_FRONT_BLINKER;
static const uint8_t LEFT_REAR = LEFT_REAR_BLINKER;
static const uint8_t RIGHT_FRONT = RIGHT_FRONT_BLINKER;
static const uint8_t RIGHT_REAR = RIGHT_REAR_BLINKER;

static const uint8_t ALL_LIGHTS[] = { LEFT_FRONT, LEFT_REAR, RIGHT_FRONT, RIGHT_REAR };

void setGroupState(const uint8_t pins[], size_t count, bool enabled) {
  for (size_t i = 0; i < count; ++i) {
    digitalWrite(pins[i], enabled ? HIGH : LOW);
  }
}

void setAllLights(bool enabled) {
  setGroupState(ALL_LIGHTS, 4, enabled);
}

void setLeftTurn(bool enabled) {
  digitalWrite(LEFT_FRONT, enabled ? HIGH : LOW);
  digitalWrite(RIGHT_FRONT, LOW);
}

void setRightTurn(bool enabled) {
  digitalWrite(RIGHT_FRONT, enabled ? HIGH : LOW);
  digitalWrite(LEFT_FRONT, LOW);
}

void applyLeftTurn() {
  digitalWrite(LEFT_FRONT, HIGH);
  digitalWrite(RIGHT_FRONT, LOW);
  digitalWrite(LEFT_REAR, HIGH);
  delay(500);
  digitalWrite(LEFT_REAR, LOW);
  delay(500);
}

void applyRightTurn() {
  digitalWrite(RIGHT_FRONT, HIGH);
  digitalWrite(LEFT_FRONT, LOW);
  digitalWrite(RIGHT_REAR, HIGH);
  delay(500);
  digitalWrite(RIGHT_REAR, LOW);
  delay(500);
}

void blinkRearLamp(uint8_t rearPin) {
  digitalWrite(rearPin, HIGH);
  delay(500);
  digitalWrite(rearPin, LOW);
  delay(500);
}

void applyBrakeState(bool leftOn, bool rightOn) {
  const bool brakeOnly = !leftOn && !rightOn;
  const bool leftTurn = leftOn && !rightOn;
  const bool rightTurn = rightOn && !leftOn;
  const bool allTurn = leftOn && rightOn;

  if (brakeOnly) {
    digitalWrite(LEFT_FRONT, LOW);
    digitalWrite(RIGHT_FRONT, LOW);
    digitalWrite(LEFT_REAR, HIGH);
    digitalWrite(RIGHT_REAR, HIGH);
    delay(50);
    return;
  }

  if (leftTurn) {
    digitalWrite(LEFT_FRONT, HIGH);
    digitalWrite(RIGHT_FRONT, LOW);
    digitalWrite(RIGHT_REAR, HIGH);
    digitalWrite(LEFT_REAR, (millis() % 1000) < 500 ? HIGH : LOW);
    delay(50);
    return;
  }

  if (rightTurn) {
    digitalWrite(RIGHT_FRONT, HIGH);
    digitalWrite(LEFT_FRONT, LOW);
    digitalWrite(LEFT_REAR, HIGH);
    digitalWrite(RIGHT_REAR, (millis() % 1000) < 500 ? HIGH : LOW);
    delay(50);
    return;
  }

  if (allTurn) {
    digitalWrite(LEFT_FRONT, HIGH);
    digitalWrite(RIGHT_FRONT, HIGH);
    digitalWrite(LEFT_REAR, HIGH);
    digitalWrite(RIGHT_REAR, HIGH);
    delay(50);
    return;
  }

  digitalWrite(LEFT_FRONT, LOW);
  digitalWrite(RIGHT_FRONT, LOW);
  digitalWrite(LEFT_REAR, HIGH);
  digitalWrite(RIGHT_REAR, HIGH);
  delay(50);
}

void applyBlinkerState(bool leftOn, bool rightOn) {
  if (leftOn && rightOn) {
    digitalWrite(LEFT_FRONT, HIGH);
    digitalWrite(RIGHT_FRONT, HIGH);
    digitalWrite(LEFT_REAR, HIGH);
    digitalWrite(RIGHT_REAR, HIGH);
    delay(500);
    digitalWrite(LEFT_REAR, LOW);
    digitalWrite(RIGHT_REAR, LOW);
    delay(500);
    return;
  }

  if (leftOn) {
    applyLeftTurn();
    return;
  }

  if (rightOn) {
    applyRightTurn();
    return;
  }

  setAllLights(false);
  delay(50);
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  pinMode(LEFT_FRONT, OUTPUT);
  pinMode(LEFT_REAR, OUTPUT);
  pinMode(RIGHT_FRONT, OUTPUT);
  pinMode(RIGHT_REAR, OUTPUT);

  pinMode(LEFT_BLINKER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BLINKER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(BRAKE_SWITCH_PIN, INPUT_PULLUP);

  setAllLights(false);

  delay(1000);
  Serial.println("ESP32-C3 blinker controller ready");
}

void loop() {
  const bool leftSwitchOn = (digitalRead(LEFT_BLINKER_SWITCH_PIN) == LOW);
  const bool rightSwitchOn = (digitalRead(RIGHT_BLINKER_SWITCH_PIN) == LOW);
  const bool brakeSwitchOn = (digitalRead(BRAKE_SWITCH_PIN) == LOW);

  static bool previousLeftSwitchOn = false;
  static bool previousRightSwitchOn = false;
  static bool previousBrakeSwitchOn = false;

  if (leftSwitchOn != previousLeftSwitchOn) {
    Serial.println(leftSwitchOn ? "Left blinker switch ON" : "Left blinker switch OFF");
    previousLeftSwitchOn = leftSwitchOn;
  }

  if (rightSwitchOn != previousRightSwitchOn) {
    Serial.println(rightSwitchOn ? "Right blinker switch ON" : "Right blinker switch OFF");
    previousRightSwitchOn = rightSwitchOn;
  }

  if (brakeSwitchOn != previousBrakeSwitchOn) {
    Serial.println(brakeSwitchOn ? "Brake switch ON" : "Brake switch OFF");
    previousBrakeSwitchOn = brakeSwitchOn;
  }

  static bool brakeWasActive = false;
  static bool brakeLatch = false;

  if (brakeSwitchOn) {
    brakeWasActive = true;
    brakeLatch = false;
    applyBrakeState(leftSwitchOn, rightSwitchOn);
    return;
  }

  if (brakeWasActive && (leftSwitchOn || rightSwitchOn)) {
    brakeLatch = true;
  }

  if (brakeLatch) {
    if (!leftSwitchOn && !rightSwitchOn) {
      brakeLatch = false;
      brakeWasActive = false;
      setAllLights(false);
      delay(50);
      return;
    }

    if (leftSwitchOn && !rightSwitchOn) {
      digitalWrite(LEFT_FRONT, HIGH);
      digitalWrite(RIGHT_FRONT, LOW);
      digitalWrite(RIGHT_REAR, HIGH);
      digitalWrite(LEFT_REAR, (millis() % 1000) < 500 ? HIGH : LOW);
      delay(50);
      return;
    }

    if (rightSwitchOn && !leftSwitchOn) {
      digitalWrite(RIGHT_FRONT, HIGH);
      digitalWrite(LEFT_FRONT, LOW);
      digitalWrite(LEFT_REAR, HIGH);
      digitalWrite(RIGHT_REAR, (millis() % 1000) < 500 ? HIGH : LOW);
      delay(50);
      return;
    }

    digitalWrite(LEFT_FRONT, HIGH);
    digitalWrite(RIGHT_FRONT, HIGH);
    digitalWrite(LEFT_REAR, HIGH);
    digitalWrite(RIGHT_REAR, HIGH);
    delay(50);
    return;
  }

  brakeWasActive = false;
  applyBlinkerState(leftSwitchOn, rightSwitchOn);
}
