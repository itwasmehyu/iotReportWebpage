#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <ArduinoJson.h>
#include "DHT.h"

// network definition
const char* ssid = "iPhone của hyu"; // wifi name
const char* password = "zxczcc101203"; // wifi Password
const char* mqtt_server = "172.20.10.4"; // broker IP address

WiFiClient espClient;
PubSubClient client(espClient);

#define lightSub "light_sub"
#define airConditionerSub "air_conditioner_sub"
#define fanSub "fan_sub"
#define lightPub "light_pub"
#define airConditionerPub "air_conditioner_pub"
#define fanPub "fan_pub"
// #define warningSub "warning_sub"
// #define warningPub "warning_pub"
#define newLightSub1 "new_led1_sub"
#define newLightSub2 "new_led2_sub"
#define newLightSub3 "new_led3_sub"

#define newLightPub1 "new_led1_pub"
#define newLightPub2 "new_led2_pub"
#define newLightPub3 "new_led3_pub"



#define sensorsDataPub "sensorsDataPub"
#define sensorsDataSub "sensorsDataSub"

#define DPIN 4
#define DTYPE DHT11

DHT dht(DPIN, DTYPE);
unsigned long lastPublish = 0; // Timer for publishing
const long publishInterval = 2000; // Publish every 2 seconds

#define LED_PIN1 16 // Pin for the LED d1 -> light
#define LED_PIN2 5  // d6 -> air conditioner
#define LED_PIN3 12  // d0 -> fan
#define LED_PIN4 2  // led moi 1
#define LED_PIN5 14 // led moi 2
#define LED_PIN6 13 // led moi 3

#define ldr_pin A0

const char *mqtt_username = "nnqhuy";
const char *mqtt_password = "b21dccn434";

void setup_wifi() {
  delay(10);
  // wifi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Da ket noi WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// connect to mqtt broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a client ID
    String clientId = "ESP8266Client-01";
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      client.subscribe(lightSub);
      client.subscribe(airConditionerSub);
      client.subscribe(fanSub);
      client.subscribe(newLightSub1);
      client.subscribe(newLightSub2);
      client.subscribe(newLightSub3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

char* changeDeviceStatus(int pin, byte* payload) {
  static char status[4];
  char* strPayload = reinterpret_cast<char*>(payload);

  Serial.print("strPayload: ");
  Serial.println(strPayload);

  if (strcmp(strPayload, "on") == 0) {
    digitalWrite(pin, HIGH);
    strcpy(status, "on");
  } 
  else if (strcmp(strPayload, "off") == 0) {
    digitalWrite(pin, LOW);
    strcpy(status, "off");
  } 
  else {
    strcpy(status, "err");
  }

  return status;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic[");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  byte strPayload[length + 1];
  memcpy(strPayload, payload, length);
  strPayload[length] = '\0';

  if (strcmp(topic, lightSub) == 0) {
    char* status = changeDeviceStatus(LED_PIN1, strPayload);
    client.publish(lightPub, status);
  } else if (strcmp(topic, airConditionerSub) == 0) {
    char* status = changeDeviceStatus(LED_PIN2, strPayload);
    client.publish(airConditionerPub, status);
  } else if (strcmp(topic, fanSub) == 0) {
    char* status = changeDeviceStatus(LED_PIN3, strPayload);
    client.publish(fanPub, status);
  } else if (strcmp(topic, newLightSub1) == 0) {
    char* status = changeDeviceStatus(LED_PIN4, strPayload);
    client.publish(newLightPub1, status);
  } else if (strcmp(topic, newLightSub2) == 0) {
    char* status = changeDeviceStatus(LED_PIN5, strPayload);
    client.publish(newLightPub2, status);
  } else if (strcmp(topic, newLightSub3) == 0) {
    char* status = changeDeviceStatus(LED_PIN6, strPayload);
    client.publish(newLightPub3, status);
  } 
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  pinMode(LED_PIN1, OUTPUT); // Set the LED pin as output
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(LED_PIN4, OUTPUT);
  pinMode(LED_PIN4, OUTPUT);
  pinMode(LED_PIN5, OUTPUT);
  pinMode(LED_PIN6, OUTPUT);

  pinMode(ldr_pin, INPUT);

  digitalWrite(LED_PIN1, LOW);
  digitalWrite(LED_PIN2, LOW);
  digitalWrite(LED_PIN3, LOW);
  digitalWrite(LED_PIN4, LOW);
  digitalWrite(LED_PIN5, LOW);
  digitalWrite(LED_PIN6, LOW);

  client.setServer(mqtt_server, 2024);
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastPublish >= publishInterval) {
    lastPublish = currentMillis;

    // Read temperature, humidity
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    int brightness = 1023 - analogRead(ldr_pin);
    // long wind  = random(0, 100);
    // long sound = random(0, 100);
    // long ground = random(0, 100);
    // Format the payload as a JSON string
    String payload;

    // LED nhấp nháy khi wind >= 60
    // if (wind >= 60) {
    //   for(int i=0; i<10; i++){
    //   digitalWrite(LED_PIN4, HIGH); 
    //   delay(50);                  
    //   digitalWrite(LED_PIN4, LOW);
    //   delay(50);                  
    //   }
    // }

    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["brightness"] = brightness;
    // doc["wind"] = wind;
    // doc["sound"] = sound;
    // doc["ground"] = ground;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C ");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" % ");

    Serial.print("Light: ");
    Serial.print(brightness);
    Serial.print(" units ");

    Serial.print("Wind: ");
    // Serial.print(wind);
    Serial.println(" m/s");

    serializeJson(doc, payload);
    // Publish to the sensorsDataPub topic
    client.publish(sensorsDataPub, payload.c_str());
  }
}
