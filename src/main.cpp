#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include "config.h"

static const uint8_t LEFT_FRONT = LEFT_FRONT_BLINKER;
static const uint8_t LEFT_REAR = LEFT_REAR_BLINKER;
static const uint8_t RIGHT_FRONT = RIGHT_FRONT_BLINKER;
static const uint8_t RIGHT_REAR = RIGHT_REAR_BLINKER;

static const uint8_t ALL_LIGHTS[] = { LEFT_FRONT, LEFT_REAR, RIGHT_FRONT, RIGHT_REAR };

WebServer webServer(80);

bool webLeftSwitchOn = false;
bool webRightSwitchOn = false;
bool webBrakeSwitchOn = false;

const char WEB_PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html lang="en"><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Hondavidson lights</title><style>
body{font-family:Arial,sans-serif;margin:24px;max-width:680px;color:#1b1b1b}h1{margin-bottom:4px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px;margin:20px 0}.item{border:1px solid #bbb;padding:14px;border-radius:6px}.state{font-weight:bold}.on{color:#087f23}.off{color:#aa1e1e}button{width:100%;margin-top:10px;padding:10px;font-size:16px;cursor:pointer}
</style></head><body><h1>Hondavidson lights</h1><p>Physical switches and web controls are combined.</p>
<div id="switches" class="grid"></div><h2>Outputs</h2><div id="outputs" class="grid"></div>
<script>
const switches=[['left','Left blinker'],['right','Right blinker'],['brake','Brake']];
const outputs=[['leftFront','Left front'],['rightFront','Right front'],['leftRear','Left rear'],['rightRear','Right rear']];
function state(value){return `<span class="state ${value?'on':'off'}">${value?'ON':'OFF'}</span>`}
function render(data){document.getElementById('switches').innerHTML=switches.map(([key,label])=>`<div class="item"><b>${label}</b><p>Effective: ${state(data.switches[key])}<br>Physical: ${state(data.physical[key])}<br>Web: ${state(data.web[key])}</p><button onclick="toggle('${key}',${!data.web[key]})">Turn web ${data.web[key]?'off':'on'}</button></div>`).join('');document.getElementById('outputs').innerHTML=outputs.map(([key,label])=>`<div class="item"><b>${label}</b><p>${state(data.outputs[key])}</p></div>`).join('')}
function update(){fetch('/status').then(response=>response.json()).then(render)}
function toggle(name,enabled){fetch(`/switch?name=${name}&state=${enabled?1:0}`).then(update)}
update();setInterval(update,500);
</script></body></html>
)rawliteral";

void sendStatus() {
  const bool physicalLeft = digitalRead(LEFT_BLINKER_SWITCH_PIN) == LOW;
  const bool physicalRight = digitalRead(RIGHT_BLINKER_SWITCH_PIN) == LOW;
  const bool physicalBrake = digitalRead(BRAKE_SWITCH_PIN) == LOW;

  String status = "{\"physical\":{\"left\":" + String(physicalLeft ? "true" : "false") +
                  ",\"right\":" + String(physicalRight ? "true" : "false") +
                  ",\"brake\":" + String(physicalBrake ? "true" : "false") +
                  "},\"web\":{\"left\":" + String(webLeftSwitchOn ? "true" : "false") +
                  ",\"right\":" + String(webRightSwitchOn ? "true" : "false") +
                  ",\"brake\":" + String(webBrakeSwitchOn ? "true" : "false") +
                  "},\"switches\":{\"left\":" + String((physicalLeft || webLeftSwitchOn) ? "true" : "false") +
                  ",\"right\":" + String((physicalRight || webRightSwitchOn) ? "true" : "false") +
                  ",\"brake\":" + String((physicalBrake || webBrakeSwitchOn) ? "true" : "false") +
                  "},\"outputs\":{\"leftFront\":" + String(digitalRead(LEFT_FRONT) == HIGH ? "true" : "false") +
                  ",\"rightFront\":" + String(digitalRead(RIGHT_FRONT) == HIGH ? "true" : "false") +
                  ",\"leftRear\":" + String(digitalRead(LEFT_REAR) == HIGH ? "true" : "false") +
                  ",\"rightRear\":" + String(digitalRead(RIGHT_REAR) == HIGH ? "true" : "false") + "}}";
  webServer.send(200, "application/json", status);
}

void setWebSwitch() {
  const String name = webServer.arg("name");
  const bool enabled = webServer.arg("state") == "1";

  if (name == "left") {
    webLeftSwitchOn = enabled;
  } else if (name == "right") {
    webRightSwitchOn = enabled;
  } else if (name == "brake") {
    webBrakeSwitchOn = enabled;
  } else {
    webServer.send(400, "text/plain", "Unknown switch");
    return;
  }

  sendStatus();
}

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

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  webServer.on("/", HTTP_GET, []() { webServer.send_P(200, "text/html", WEB_PAGE); });
  webServer.on("/status", HTTP_GET, sendStatus);
  webServer.on("/switch", HTTP_GET, setWebSwitch);
  webServer.begin();

  delay(1000);
  Serial.println("ESP32-C3 blinker controller ready");
  Serial.print("Web server: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  webServer.handleClient();

  const bool physicalLeftSwitchOn = (digitalRead(LEFT_BLINKER_SWITCH_PIN) == LOW);
  const bool physicalRightSwitchOn = (digitalRead(RIGHT_BLINKER_SWITCH_PIN) == LOW);
  const bool physicalBrakeSwitchOn = (digitalRead(BRAKE_SWITCH_PIN) == LOW);
  const bool leftSwitchOn = physicalLeftSwitchOn || webLeftSwitchOn;
  const bool rightSwitchOn = physicalRightSwitchOn || webRightSwitchOn;
  const bool brakeSwitchOn = physicalBrakeSwitchOn || webBrakeSwitchOn;

  static bool previousLeftSwitchOn = false;
  static bool previousRightSwitchOn = false;
  static bool previousBrakeSwitchOn = false;

  if (physicalLeftSwitchOn != previousLeftSwitchOn) {
    Serial.println(physicalLeftSwitchOn ? "Left blinker switch ON" : "Left blinker switch OFF");
    previousLeftSwitchOn = physicalLeftSwitchOn;
  }

  if (physicalRightSwitchOn != previousRightSwitchOn) {
    Serial.println(physicalRightSwitchOn ? "Right blinker switch ON" : "Right blinker switch OFF");
    previousRightSwitchOn = physicalRightSwitchOn;
  }

  if (physicalBrakeSwitchOn != previousBrakeSwitchOn) {
    Serial.println(physicalBrakeSwitchOn ? "Brake switch ON" : "Brake switch OFF");
    previousBrakeSwitchOn = physicalBrakeSwitchOn;
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
