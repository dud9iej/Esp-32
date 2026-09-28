#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- Pin Mappings (Matched directly to diagram.json) ---
#define PIR_PIN          33  // pir:OUT  -> esp:D33
#define POT_PIN          32  // pot:SIG  -> esp:D32
#define CSI_MOCK_PIN     14  // swCsi:2  -> esp:D14
#define MQTT_MOCK_PIN    13  // swMqtt:2 -> esp:D13

#define LED_RED          25  // r1 -> esp:D25
#define LED_GREEN        26  // r2 -> esp:D26
#define LED_BLUE         27  // r3 -> esp:D27

// --- Network Settings ---
const char* SSID         = "Wokwi-GUEST";
const char* PASSWORD     = "";
const char* MQTT_BROKER   = "broker.hivemq.com";
const int   MQTT_PORT     = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- State Machine ---
enum SystemState {
  STATE_IDLE,
  STATE_ACTIVITY,
  STATE_WAIT_FOR_RESET
};

SystemState currentState = STATE_IDLE;

// --- Timers ---
unsigned long activityStartTime = 0;
const unsigned long ACTIVITY_DURATION = 10000; // 10s test window

unsigned long lastRawDataTime = 0;
const unsigned long RAW_DATA_INTERVAL = 1000;  // 1s stream interval

unsigned long lastMqttRetry = 0;
const unsigned long MQTT_RETRY_INTERVAL = 5000; // 5s retry cooldown

unsigned long lastActivityPrint = 0;

// --- Helper Functions ---
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? HIGH : LOW);
  digitalWrite(LED_GREEN, g ? HIGH : LOW);
  digitalWrite(LED_BLUE, b ? HIGH : LOW);
}

void ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(SSID, PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(250);
    attempts++;
  }
}

void ensureMQTTConnected() {
  if (mqttClient.connected()) return;

  if (millis() - lastMqttRetry < MQTT_RETRY_INTERVAL) return;
  lastMqttRetry = millis();

  ensureWiFiConnected();
  if (WiFi.status() == WL_CONNECTED) {
    String clientId = "ESP32-Wokwi-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("[MQTT] Connected successfully.");
    } else {
      Serial.print("[MQTT] Connection failed, state=");
      Serial.println(mqttClient.state());
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  
  pinMode(CSI_MOCK_PIN, INPUT);
  pinMode(MQTT_MOCK_PIN, INPUT);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  setRGB(false, false, false);

  WiFi.mode(WIFI_STA);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  Serial.println("System Ready. Wiring confirmed.");
}

void loop() {
  bool isRealMqtt = (digitalRead(MQTT_MOCK_PIN) == HIGH);

  if (isRealMqtt) {
    ensureMQTTConnected();
    if (mqttClient.connected()) {
      mqttClient.loop();
    }
  }

  switch (currentState) {

    case STATE_IDLE: {
      bool unconsciousDetected = (digitalRead(CSI_MOCK_PIN) == HIGH);

      if (unconsciousDetected) {
        Serial.println("\n[ALERT] CSI Unconscious state triggered!");

        if (isRealMqtt) {
          if (mqttClient.connected()) {
            mqttClient.publish("rpi/emergency", "WAKE_UP");
            Serial.println("[MQTT REAL] Published emergency WAKE_UP");
          }
          setRGB(false, false, false);
        } else {
          setRGB(false, false, true);
          delay(300);
          setRGB(false, false, false);
          Serial.println("[MQTT MOCK] Flashed Blue LED (Pi Alert Sent)");
        }

        activityStartTime = millis();
        lastActivityPrint = millis();
        currentState = STATE_ACTIVITY;
        break;
      }

      if (millis() - lastRawDataTime >= RAW_DATA_INTERVAL) {
        lastRawDataTime = millis();
        int pirVal = digitalRead(PIR_PIN);
        int potVal = analogRead(POT_PIN);

        if (isRealMqtt && mqttClient.connected()) {
          char payload[64];
          snprintf(payload, sizeof(payload), "{\"pir\":%d, \"i2c_mock\":%d}", pirVal, potVal);
          mqttClient.publish("rpi/raw_data", payload);
          Serial.printf("[MQTT REAL] Sent -> PIR: %d | Pot: %d\n", pirVal, potVal);
        } else {
          Serial.printf("[IDLE MOCK] Stream -> PIR: %d | Pot: %d\n", pirVal, potVal);
        }
      }
      break;
    }

    case STATE_ACTIVITY: {
      unsigned long elapsed = millis() - activityStartTime;

      if (elapsed < ACTIVITY_DURATION) {
        if (millis() - lastActivityPrint >= 1000) {
          lastActivityPrint = millis();
          Serial.printf("[ACTIVITY MODE] Discarding CSI. Analyzing Vitals... %lu s remaining\n", (ACTIVITY_DURATION - elapsed) / 1000);
        }
      } else {
        int finalPot = analogRead(POT_PIN);
        Serial.println("\n[ANALYSIS RESULT]");

        if (finalPot > 3000) {
          Serial.println("Outcome: DATA_INCORRECT");
          if (isRealMqtt && mqttClient.connected()) {
            mqttClient.publish("rpi/vital_status", "DATA_INCORRECT");
          } else {
            setRGB(false, false, true);
          }
        } else if (finalPot < 1000) {
          Serial.println("Outcome: VITAL_DISORDER");
          if (isRealMqtt && mqttClient.connected()) {
            mqttClient.publish("rpi/vital_status", "VITAL_DISORDER");
          } else {
            setRGB(true, false, false);
          }
        } else {
          Serial.println("Outcome: VITAL_OK");
          if (isRealMqtt && mqttClient.connected()) {
            mqttClient.publish("rpi/vital_status", "VITAL_OK");
          } else {
            setRGB(false, true, false);
          }
        }

        Serial.println("--> Toggle CSI switch (GPIO 14) to OFF to clear state.");
        currentState = STATE_WAIT_FOR_RESET;
      }
      break;
    }

    case STATE_WAIT_FOR_RESET: {
      if (digitalRead(CSI_MOCK_PIN) == LOW) {
        setRGB(false, false, false);
        currentState = STATE_IDLE;
        Serial.println("[RESET] Returned to IDLE state.\n");
      }
      break;
    }
  }

  delay(10);
}
```[cite: 9]