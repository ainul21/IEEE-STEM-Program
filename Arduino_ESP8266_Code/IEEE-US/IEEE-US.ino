#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Ainul";
const char* password = "asdf1234";
// const char* ssid = "dlinkap1360";
// const char* password = "abcd1234";

const char* mqtt_server = "174.138.28.115";
const int mqtt_port = 1883;
const char* mqtt_user = ""; // Add user if authentication is required
const char* mqtt_password = ""; // Add password if authentication is required

// MQTT Topics
const char* distance_topic = "sensor/distance";

// Ultrasonic Sensor Pins
const int trigPin = D7;
const int echoPin = D8;
float preDistance = 0;

// Built-in LED
const int builtInLed = LED_BUILTIN;

WiFiClient espClient;
PubSubClient client(espClient);

void setupWifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ensureConnections() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
  if (!client.connected()) {
    reconnect();
  }
}

void setup() {
  Serial.begin(9600);
  setupWifi();
  client.setServer(mqtt_server, mqtt_port);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(builtInLed, OUTPUT);

  digitalWrite(builtInLed, LOW);
}

void loop() {
  ensureConnections();
  client.loop();

  // Read distance from ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  if (distance < 10) {
    digitalWrite(builtInLed, HIGH);
    Serial.print(distance);
    Serial.println(": LED ON");
    preDistance = distance;
  } else {
    digitalWrite(builtInLed, LOW);
    Serial.print(distance);
    Serial.println(": LED OFF");
    preDistance = distance;
  }

  delay(10);

  // Check if any reads failed and exit early (to try again).
  if (isnan(distance)) {
    Serial.println("Failed to read from sensors!");
    return;
  }

  // Publish Sensor Data
  client.publish(distance_topic, String(distance).c_str(), true);

  delay(100); // Delay between measurements
}
