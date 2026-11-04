/*
  Smart Agriculture DSS - ESP32 web server (no MQTT)
  The ESP32 reads DHT11 + soil + MQ7 and serves them as JSON at:
      http://<ESP32-IP>/data
  The dashboard page fetches that address every few seconds.

  Libraries (Library Manager): DHT sensor library (Adafruit), Adafruit Unified Sensor
  (WiFi and WebServer come with the ESP32 board package)
*/

#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// ---------- EDIT THESE ----------
const char* WIFI_SSID = "Drek";
const char* WIFI_PASS = "12345678";

const char* WEATHER_API_KEY = "b079fb63f4c13adff648116c69f30842";
const char* WEATHER_CITY = "Mumbai";
// --------------------------------

#define DHTPIN   4
#define DHTTYPE  DHT11
#define SOIL_PIN 34   // ADC1, input only
#define MQ7_PIN  35   // ADC1, input only (use a voltage divider if the module runs at 5 V)

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperature = 0, humidity = 0;
int soilRaw = 0, gasRaw = 0;
bool dhtOk = false;
unsigned long lastRead = 0;

int readAnalogAvg(int pin) {
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(pin); delay(5); }
  return sum / 10;
}

void readSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  dhtOk = !(isnan(h) || isnan(t));
  if (dhtOk) { humidity = h; temperature = t; }
  else Serial.println("DHT11 read failed - check wiring");
  soilRaw = readAnalogAvg(SOIL_PIN);   // higher = drier
  gasRaw  = readAnalogAvg(MQ7_PIN);    // higher = more gas
}

void sendCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");   // lets the dashboard page read this data
  server.sendHeader("Cache-Control", "no-store");
}

void handleData() {
  sendCORS();
  char json[160];
  snprintf(json, sizeof(json),
           "{\"temperature\":%.1f,\"humidity\":%.1f,\"soil\":%d,\"gas\":%d,\"dhtOk\":%s}",
           temperature, humidity, soilRaw, gasRaw, dhtOk ? "true" : "false");
  server.send(200, "application/json", json);
}

void handleRoot() {
  sendCORS();
  server.send(200, "text/plain", "ESP32 smart agriculture node is running. Data at /data");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(MQ7_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Connected. Open this in the dashboard: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  readSensors();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {      // simple auto-reconnect
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(3000);
  }
  server.handleClient();

  if (millis() - lastRead >= 2000) {        // DHT11 needs at least 1 s between reads
    lastRead = millis();
    readSensors();
    Serial.printf("T=%.1f C  H=%.1f %%  Soil=%d  Gas=%d\n", temperature, humidity, soilRaw, gasRaw);
  }
}
