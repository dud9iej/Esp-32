#ifndef CONFIG_H
#define CONFIG_H

// --- Network Settings ---
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// --- MQTT Broker Settings (HiveMQ public or local broker) ---
#define MQTT_BROKER   "broker.hivemq.com"
#define MQTT_PORT     1883

// --- MQTT Topics for Silver Parakeet & Glorious Winner ---
#define TOPIC_CSI_ALERT   "healthbot/silver-parakeet/csi_alert"
#define TOPIC_TELEMETRY   "healthbot/glorious-winner/telemetry"
#define TOPIC_COMMANDS    "healthbot/glorious-winner/cmd"

// --- Pin Mappings ---
#define PIR_PIN          33
#define POT_PIN          32
#define CSI_MOCK_PIN     14
#define MQTT_MOCK_PIN    13

#define LED_RED          25
#define LED_GREEN        26
#define LED_BLUE         27

#endif