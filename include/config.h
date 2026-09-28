#ifndef CONFIG_H
#define CONFIG_H

// --- Network Settings ---
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define MQTT_BROKER   "broker.hivemq.com"
#define MQTT_PORT     1883

// --- MQTT Topics bridging Silver Parakeet & Glorious Winner ---
#define TOPIC_ALERT       "healthbot/silver-parakeet/alert"
#define TOPIC_TELEMETRY   "healthbot/glorious-winner/telemetry"
#define TOPIC_COMMANDS    "healthbot/glorious-winner/cmd"

// --- Pin Mappings ---
#define PIR_PIN          33  // PIR Motion Sensor
#define CSI_MOCK_PIN     14  // Slide switch for CSI / Anomaly trigger
#define MQTT_MOCK_PIN    13  // Slide switch for real vs mock MQTT

// --- I2C Pins (Default ESP32 SDA / SCL) ---
#define I2C_SDA          21
#define I2C_SCL          22

// --- RGB Status LED ---
#define LED_RED          25
#define LED_GREEN        26
#define LED_BLUE         27

#endif