#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// define the DHT pin
#define DHTPIN D7
// Define the pin connected to the flame sensor
#define FLAME_SENSOR_PIN D1
// Define the pin connected to the motion sensor
#define MOTION_SENSOR_PIN D0
// Define the pin connected to the gas sensor
#define GAS_SENSOR_PIN A0
// define the DHT type
#define DHTTYPE DHT11

// initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);
// replace with your network credentials
const char* ssid = "TP-Link_6C72";
const char* password = "56759860";

// replace with your MQTT broker address
const char* mqtt_server = "test.mosquitto.org";

// initialize the WiFi client and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// define callback function for when a message is received
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received message: ");
  Serial.println((char*)payload);
}

void setup() {
  // initialize serial communication
  Serial.begin(9600);
  dht.begin();
  // connect to WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // set MQTT broker and callback function
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // connect to MQTT broker
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT broker!");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // subscribe to a topic
  client.subscribe("test/topic");
  // Set the flame sensor pin as an input
  pinMode(FLAME_SENSOR_PIN, INPUT);
  // Set the motion sensor pin as an input
  pinMode(MOTION_SENSOR_PIN, INPUT);
}

void loop() {
  // check for incoming MQTT messages
  client.loop();

  // DH11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  String payload_temperature = String(temperature);
  String payload_humidity = String(humidity);
  
//  if (isnan(temperature) || isnan(humidity)) {
//    Serial.println("Failed to read from DHT sensor!");
//    delay(2000);
//    return;
//  }

  // Read the value from the flame sensor pin
  int flameValue = digitalRead(FLAME_SENSOR_PIN);
  String payload_flameValue = String(flameValue);

  int gasValue = analogRead(GAS_SENSOR_PIN);

  // Calculate the gas concentration based on the sensor value
  float gasConcentration = (5.0 * gasValue / 1023.0) / 0.1;
  String payload_gasConcentration = String(gasConcentration);

 // Read the value from the motion sensor pin
  int motionValue = digitalRead(MOTION_SENSOR_PIN);
  String payload_motionValue = String(motionValue);
  
  // publish a message to a topic
  client.publish("home/temp", payload_temperature.c_str());
  client.publish("home/Humidity", payload_humidity.c_str());
  client.publish("home/flame", payload_flameValue.c_str());
  client.publish("home/MQ6", payload_gasConcentration.c_str());
  client.publish("home/motion", payload_motionValue.c_str());
  
  delay(1000);
}