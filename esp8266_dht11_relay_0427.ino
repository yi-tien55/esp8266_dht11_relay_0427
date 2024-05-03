#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN  4
#define RELAY_PIN_1 2  // relay1連接到GPIO2引腳
#define RELAY_PIN_2 5  // relay2連接到GPIO5引脚

// Set your WiFi credentials
const char* ssid = "IOT123";
const char* password = "IOT123456";

// Set your ThingSpeak API key
const char* apiKey = "Q2JC1RGQMACVHL5F";
const char* server = "api.thingspeak.com";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(RELAY_PIN_1, OUTPUT);  // 將GPIO2引脚設置為输出模式
  pinMode(RELAY_PIN_2, OUTPUT);  // 將GPIO5引脚設置為输出模式

  // Connect to Wi-Fi
  Serial.begin(9600);
  delay(10);
  Serial.println("");
  Serial.println("Connecting To: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
}

void loop() {
  float humidity, temperature_C;

  humidity = dht.readHumidity();
  temperature_C = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature_C)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature_C);
  Serial.println(" °C");

  // Upload data to ThingSpeak
  WiFiClient client;
  if (!client.connect(server, 80)) {
    Serial.println("Connection to ThingSpeak failed!");
    delay(2000);
    return;
  }

  String postStr = String("/update?api_key=") + apiKey + "&field1=" + String(temperature_C) + "&field2=" + String(humidity);
  client.print("GET " + postStr + " HTTP/1.1\r\n");
  client.print("Host: " + String(server) + "\r\n");
  client.print("Connection: close\r\n\r\n");

  Serial.println("Waiting for response...");
  while (!client.available()) {
    delay(500);
  }

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println("Closing connection");
  client.stop();

  // Control relays based on sensor readings
  if (temperature_C > 28) {
    digitalWrite(RELAY_PIN_1, LOW); // 打開relay1
    Serial.println("Relay 1: ON");
  } else {
    digitalWrite(RELAY_PIN_1, HIGH); //關閉relay1
    Serial.println("Relay 1: OFF");
  }

  if (humidity > 65) {
    digitalWrite(RELAY_PIN_2, LOW); // 打開relay2
    Serial.println("Relay 2: ON");
  } else {
    digitalWrite(RELAY_PIN_2, HIGH); // 關閉relay2
    Serial.println("Relay 2: OFF");
  }
  delay(180000); // Send data every 3 minutes
}
