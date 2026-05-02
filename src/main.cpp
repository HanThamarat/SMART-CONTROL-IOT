#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Dimmer.h"

// ===== WiFi =====
#define WIFI_STA_NAME "HanThamarat"
#define WIFI_STA_PASS "88888888"

// ===== MQTT =====
#define MQTT_SERVER   "mqtt.lextago.site"
#define MQTT_PORT     11883
#define MQTT_USERNAME "root"
#define MQTT_PASSWORD "root"
#define MQTT_NAME     "smart-control-esp8266"

// ===== Dimmer Pins =====
#define DIM_PIN 5   // D1 / GPIO5
#define ZC_PIN  4   // D2 / GPIO4

// ===== Ultrasonic Pins =====
#define TRIG_PIN 14  // D5 
#define ECHO_PIN 12  // D6

WiFiClient espClient;
PubSubClient mqtt(espClient);

Dimmer dimmer(DIM_PIN, ZC_PIN);

unsigned long lastMsg = 0;
unsigned long lastReconnectAttempt = 0;
unsigned long lastUltrasonic = 0;
unsigned long holdBrightUntil = 0;

int max_value = 100;
int min_value = 0;
int current_value = 0;
int brightnessValue = 0;
int infraredVal = 0;

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print(" | Message: ");
  Serial.println(message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("JSON parse failed");
    return;
  }

  if (doc.containsKey("current_value")) {
    max_value = doc["max_value"] | 100;
    min_value = doc["min_value"] | 0;
    current_value = doc["current_value"] | 0;

    current_value = constrain(current_value, min_value, max_value);

    Serial.print("Set current_value = ");
    Serial.println(current_value);
  }
}

void setup_wifi() {
  delay(100);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED && timeout < 40) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi failed");
  }
}

boolean reconnect() {
  Serial.print("Attempting MQTT connection... ");

  if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("connected");

    mqtt.publish("TEST/STATUS", "ESP8266 Online");

    mqtt.subscribe("TEST/COMMAND");
    mqtt.subscribe("TEST/MQTT");
    mqtt.subscribe("Control");
    mqtt.subscribe("Test");

    return true;
  }

  Serial.print("failed, rc=");
  Serial.println(mqtt.state());

  return false;
}

void ultrasonicFunc() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 8000);
  unsigned long now = millis();

  if (duration != 0) {
    long cm = microsecondsToCentimeters(duration);

    Serial.print("Distance: ");
    Serial.print(cm);
    Serial.println(" cm");

    if (cm <= 10) {
      holdBrightUntil = now + 10000;
    }
  }

  if (now < holdBrightUntil) {
    brightnessValue = current_value;
  } else {
    brightnessValue = 0;
  }

  brightnessValue = constrain(brightnessValue, 0, 100);
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("Starting...");

  setup_wifi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

  dimmer.begin();
  dimmer.setBrightness(0);

  Serial.println("Dimmer Program Started");
}

void loop() {
  // Initial Dimmer Lib.
  dimmer.handle();

  unsigned long now = millis();

  if (now - lastUltrasonic >= 200) {
    lastUltrasonic = now;

    ultrasonicFunc();
    dimmer.setBrightness(brightnessValue);
  }

  if (!mqtt.connected()) {
    if (now - lastReconnectAttempt >= 5000) {
      lastReconnectAttempt = now;

      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqtt.loop();

    if (now - lastMsg >= 5000) {
      lastMsg = now;

      char buffer[10];
      snprintf(buffer, sizeof(buffer), "%d", infraredVal);

      mqtt.publish("Test", buffer);
      mqtt.publish("Control", buffer);

      Serial.println("Published heartbeat");
    }
  }
}