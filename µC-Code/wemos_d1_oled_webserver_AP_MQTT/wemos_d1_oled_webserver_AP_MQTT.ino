// ========== Bibliotheken ==========
// OLED Display (SH1106/SSD1306):
// https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <SH1106Wire.h>

// INA226 Sensor:
// https://github.com/RobTillaart/INA226
#include <INA226.h>

// ESP8266 WiFi:
// Installiert über ESP8266 Arduino Core
#include <ESP8266WiFi.h>

// Webserver:
// Teil von ESP8266 Arduino Core
#include <ESP8266WebServer.h>

// mDNS (optional):
// Teil von ESP8266 Arduino Core
#include <ESP8266mDNS.h>

// OTA-Update Support:
// Teil von ESP8266 Arduino Core
#include <ArduinoOTA.h>

// WiFiManager für WLAN Setup:
// https://github.com/tzapu/WiFiManager
#include <WiFiManager.h>

// EEPROM-Speicher:
// Standard Arduino-Bibliothek
#include <EEPROM.h>

// MQTT-Client:
// https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

// ========== Versionsinfo ==========
const char* Release = "V3.1 MQTT by OE9SAU";

// ========== OLED Display ==========
SH1106Wire display(0x3C, D2, D1);  // SDA = D2, SCL = D1

// ========== INA226 Sensoren ==========
INA226 INA0(0x40);
INA226 INA1(0x41);
INA226 INA2(0x44);
INA226 INA3(0x45);

// ========== Webserver ==========
ESP8266WebServer server(2210);

// ========== Sensorwerte ==========
float c00, v0, c11, v1, c22, v2, c33, v3;
bool wlanOk = false;

// ========== MQTT ==========
WiFiClient espClient;
PubSubClient mqttClient(espClient);

struct MqttConfig {
  char broker[40] = "";
  int port = 1883;
  char username[32] = "";
  char password[32] = "";
  char topic[64] = "psu/oe9xvi";
} mqttConfig;

bool mqttConnected = false;

// ========== Web Root ==========
void handleRoot() {
  int rssi = WiFi.RSSI();
  int level = 0;
  if (rssi > -50) level = 4;
  else if (rssi > -60) level = 3;
  else if (rssi > -70) level = 2;
  else if (rssi > -80) level = 1;

  String html = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<title>OE9XVI PSU-Monitor</title><meta http-equiv='refresh' content='5'>"
                "<style>body{background:#121212;color:#e0e0e0;font-family:sans-serif;padding:20px;}h1{text-align:center;}table{margin:0 auto;border-collapse:collapse;}td,th{padding:10px;border-bottom:1px solid #333;}footer{text-align:center;margin-top:20px;color:#888;}</style></head><body>"
                "<h1>OE9XVI PSU Monitor</h1>"
                "<table><tr><th>Sensor</th><th>Strom (A)</th><th>Spannung (V)</th></tr>"
                "<tr><td>BAT</td><td>" + String(c00, 2) + "</td><td>" + String(v0, 2) + "</td></tr>"
                "<tr><td>NT1</td><td>" + String(c11, 2) + "</td><td>" + String(v1, 2) + "</td></tr>"
                "<tr><td>NT2</td><td>" + String(c22, 2) + "</td><td>" + String(v2, 2) + "</td></tr>"
                "<tr><td>NT3</td><td>" + String(c33, 2) + "</td><td>" + String(v3, 2) + "</td></tr></table>"
                "<footer>IP: " + WiFi.localIP().toString() + " | MQTT: " + (mqttConnected ? "✅" : "❌") + "</footer>"
                "</body></html>";
  server.send(200, "text/html", html);
}

// ========== WLAN-Balken ==========
void drawWifiSignal(int strength) {
  int x = 110, y = 0, barWidth = 3, gap = 2, level = 0;
  if (strength > -50) level = 4;
  else if (strength > -60) level = 3;
  else if (strength > -70) level = 2;
  else if (strength > -80) level = 1;

  for (int i = 0; i < 4; i++) {
    int barHeight = (i + 1) * 2;
    if (i < level)
      display.fillRect(x + i * (barWidth + gap), y + (8 - barHeight), barWidth, barHeight);
    else
      display.drawRect(x + i * (barWidth + gap), y + (8 - barHeight), barWidth, barHeight);
  }
}

// ========== MQTT-Verbindung ==========
void setupMQTT() {
  mqttClient.setServer(mqttConfig.broker, mqttConfig.port);
  if (strlen(mqttConfig.broker) > 0) {
    if (mqttClient.connect("PSUClient", mqttConfig.username, mqttConfig.password)) {
      mqttConnected = true;
      mqttClient.publish(mqttConfig.topic, "MQTT verbunden");
      Serial.println("MQTT verbunden");
    } else {
      Serial.println("MQTT-Verbindung fehlgeschlagen");
    }
  }
}

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);
  EEPROM.begin(512);
  EEPROM.get(0, mqttConfig);

  display.init();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.drawString(0, 0, "OE9XVI PSU-Monitor");
  display.drawString(0, 12, Release);
  display.display();
  delay(1500);

  if (!INA0.begin()) Serial.println("INA0 Fehler");
  if (!INA1.begin()) Serial.println("INA1 Fehler");
  if (!INA2.begin()) Serial.println("INA2 Fehler");
  if (!INA3.begin()) Serial.println("INA3 Fehler");

  INA0.setMaxCurrentShunt(20, 0.002);
  INA1.setMaxCurrentShunt(20, 0.002);
  INA2.setMaxCurrentShunt(20, 0.002);
  INA3.setMaxCurrentShunt(20, 0.002);

  WiFiManager wifiManager;
  display.drawString(0, 36, "Starte WiFi-Setup...");
  display.display();
  if (!wifiManager.autoConnect("OE9XVI-PSU-Monitor-AP")) {
    Serial.println("WLAN fehlgeschlagen. Neustart...");
    display.drawString(0, 48, "WLAN fehlgeschlagen");
    display.display();
    delay(3000);
    ESP.restart();
  }

  wlanOk = true;
  Serial.println("WLAN verbunden: " + WiFi.localIP().toString());
  display.clear();
  display.drawString(0, 0, "WLAN verbunden");
  display.drawString(0, 12, WiFi.SSID());
  display.drawString(0, 24, WiFi.localIP().toString());
  display.display();
  delay(3000);

  ArduinoOTA.begin();

  server.on("/", handleRoot);

  // MQTT Setup-Seite
  server.on("/mqtt", HTTP_GET, []() {
    String form = "<form method='POST'>"
                  "Broker: <input name='broker' value='" + String(mqttConfig.broker) + "'><br>"
                  "Port: <input name='port' value='" + String(mqttConfig.port) + "'><br>"
                  "Username: <input name='username' value='" + String(mqttConfig.username) + "'><br>"
                  "Password: <input type='password' name='password' value='" + String(mqttConfig.password) + "'><br>"
                  "Topic: <input name='topic' value='" + String(mqttConfig.topic) + "'><br>"
                  "<input type='submit' value='Speichern & Neustart'>"
                  "</form>";
    server.send(200, "text/html", "<h1>MQTT Setup</h1>" + form);
  });

  server.on("/mqtt", HTTP_POST, []() {
    strncpy(mqttConfig.broker, server.arg("broker").c_str(), sizeof(mqttConfig.broker));
    mqttConfig.port = server.arg("port").toInt();
    strncpy(mqttConfig.username, server.arg("username").c_str(), sizeof(mqttConfig.username));
    strncpy(mqttConfig.password, server.arg("password").c_str(), sizeof(mqttConfig.password));
    strncpy(mqttConfig.topic, server.arg("topic").c_str(), sizeof(mqttConfig.topic));
    EEPROM.put(0, mqttConfig);
    EEPROM.commit();
    server.send(200, "text/html", "Gespeichert. Neustart in 3s...");
    delay(3000);
    ESP.restart();
  });

  server.begin();
  Serial.println("HTTP Server gestartet");

  setupMQTT();
}

// ========== Hauptloop ==========
void loop() {
  if (wlanOk) {
    ArduinoOTA.handle();
    server.handleClient();
  }

  mqttClient.loop();

  // Sensorwerte lesen
  c00 = INA0.getCurrent_mA() / 1000.0;
  v0 = INA0.getBusVoltage();
  c11 = INA1.getCurrent_mA() / 1000.0;
  v1 = INA1.getBusVoltage();
  c22 = INA2.getCurrent_mA() / 1000.0;
  v2 = INA2.getBusVoltage();
  c33 = INA3.getCurrent_mA() / 1000.0;
  v3 = INA3.getBusVoltage();

  // OLED anzeigen
  display.clear();
  drawWifiSignal(WiFi.RSSI());
  display.drawString(0, 0, "OE9XVI PSU-Monitor");
  display.drawString(0, 12, "BAT: " + String(c00, 2) + "A " + String(v0, 2) + "V");
  display.drawString(0, 24, "NT1: " + String(c11, 2) + "A " + String(v1, 2) + "V");
  display.drawString(0, 36, "NT2: " + String(c22, 2) + "A " + String(v2, 2) + "V");
  display.drawString(0, 48, "NT3: " + String(c33, 2) + "A " + String(v3, 2) + "V");
  display.display();

  // MQTT Publish
  if (mqttConnected) {
    String payload = "{";
    payload += "\"bat\":{\"c\":" + String(c00, 2) + ",\"v\":" + String(v0, 2) + "},";
    payload += "\"nt1\":{\"c\":" + String(c11, 2) + ",\"v\":" + String(v1, 2) + "},";
    payload += "\"nt2\":{\"c\":" + String(c22, 2) + ",\"v\":" + String(v2, 2) + "},";
    payload += "\"nt3\":{\"c\":" + String(c33, 2) + ",\"v\":" + String(v3, 2) + "}";
    payload += "}";
    mqttClient.publish(mqttConfig.topic, payload.c_str());
  }

  delay(1000);
}
