#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- UPDATE THESE TWO ---
#define WIFI_STA_NAME "WIFI_NAME"
#define WIFI_STA_PASS  "WIFI_PASSWORD"

// IMPORTANT: Run 'ipconfig' on your PC. 
// Use the IPv4 address that looks like 192.168.1.XXX
#define MQTT_SERVER   "192.168.1.33" 
#define MQTT_PORT     1883
#define MQTT_USERNAME "root"
#define MQTT_PASSWORD "root"
#define MQTT_NAME     "smart-control-esp8266"

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastMsg = 0;
unsigned long lastReconnectAttempt = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Clear any previous saved connection
  delay(100);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 40) { // Try for 20 seconds
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi failed. Check your SSID/Password or Router.");
  }
}

boolean reconnect() {
  Serial.print("Attempting MQTT connection...");
  if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("connected");
    mqtt.publish("TEST/STATUS", "ESP8266 Online");
    mqtt.subscribe("TEST/COMMAND");
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqtt.state());
    Serial.println(" try again in 5 seconds");
  }
  return mqtt.connected();
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void loop() {
  if (!mqtt.connected()) {
    unsigned long now = millis();
    // Try to reconnect every 5 seconds without "freezing" the code
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // MQTT loop only runs if connected
    mqtt.loop();

    unsigned long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;
      Serial.println("Publishing heartbeat...");
      mqtt.publish("light/control", "30");
      Serial.println(mqtt.subscribe("TEST/MQTT"));
    }
  }
}