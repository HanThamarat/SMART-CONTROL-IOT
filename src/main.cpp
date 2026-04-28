#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string>
#include <ArduinoJson.h>
#include "Dimmer.h"

#define WIFI_STA_NAME "HanThamarat"
#define WIFI_STA_PASS  "88888888"

// IMPORTANT: Run 'ipconfig' on your PC. 
// Use the IPv4 address that looks like 192.168.1.XXX
#define MQTT_SERVER   "mqtt.lextago.site" 
#define MQTT_PORT     11883
#define MQTT_USERNAME "root"
#define MQTT_PASSWORD "root"
#define MQTT_NAME     "smart-control-esp8266"

uint8_t D1 = 5; // PSM Pin
uint8_t D2 = 4; // zc

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastMsg = 0;
unsigned long lastReconnectAttempt = 0;

Dimmer dimmer(D1, D2);

int value = 0;
int dir = 1;

int ledPin = LED_BUILTIN;
int InfrareddigitalPin = 2; // D4
int infraredVal = 0;

// dimmer
int max_value = 100;
int min_value = 0;
int current_value = 0;

int brightnessValue = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Received topic: [" );
  Serial.print(topic);
  Serial.print("]");
  Serial.print("| Message | ");
  Serial.print(message);
  Serial.println();

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("JSON parse failed");
    return;
  }

  if (!error && doc.containsKey("current_value")) {
      max_value = doc["max_value"];
      min_value = doc["min_value"];
      current_value = doc["current_value"];
  }
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
  pinMode(ledPin, OUTPUT); // sets the pin as output
  pinMode(InfrareddigitalPin, INPUT); // sets the pin as input

  Serial.begin(115200);
  // Serial.begin(9600);
  setup_wifi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
  Serial.println("Dimmer Program Started");
  dimmer.begin();
}

void InfraredFunc() {
    infraredVal = digitalRead(InfrareddigitalPin);
    Serial.print("infrared value : "); 
    Serial.println(infraredVal); 

    infraredVal == 1 ?  brightnessValue = 100 : brightnessValue = current_value;

}

void loop() {
  dimmer.handle();

  InfraredFunc();
 
  dimmer.setBrightness(brightnessValue);

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
      char buffer[10];
      snprintf(buffer, sizeof(buffer), "%d", infraredVal);
      Serial.println("Publishing heartbeat...");
      mqtt.publish("Test", buffer);
      mqtt.publish("Control", buffer);
      Serial.println(mqtt.subscribe("TEST/MQTT"));
      Serial.println(mqtt.subscribe("Control"));
      Serial.println(mqtt.subscribe("Test"));
    }
  }
}