#include <Wire.h>
#include <SH1106Wire.h>   	  	// https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <INA226.h>	          	// https://github.com/RobTillaart/INA226
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>  		// https://github.com/tzapu/WiFiManager

const char* Release = "V2.0 by OE9SAU";

// OLED Display
SH1106Wire display(0x3C, D2, D1);  // SDA = D2, SCL = D1

// INA226 Sensoren
INA226 INA0(0x40);
INA226 INA1(0x41);
INA226 INA2(0x44);
INA226 INA3(0x45);

// Webserver
ESP8266WebServer server(5050);

// Sensorwerte
float c00, v0, c11, v1, c22, v2, c33, v3;

bool wlanOk = false;

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>OE9XVI - PsMon</title>"
                "<meta http-equiv=\"refresh\" content=\"5\">"
                "<style>body {background-color: black; color: white; font-family: monospace;}"
                "h1 {text-align:center; font-size: 36px; margin-bottom: 5px;}"
                ".release {text-align:center; font-size: 16px; margin-bottom: 20px; color: gray;}"
                "pre {font-size: 24px; line-height: 1.5;}"
                "footer {font-size: 14px; color: gray; text-align:center; margin-top: 20px;}"
                "</style></head><body>"
                "<h1>OE9XVI Powersupply Monitor</h1>"
                "<div class='release'>" + String(Release) + "</div><pre>";

  html += "Sensor 0: " + String(c00, 2) + " A   " + String(v0, 2) + " V\n";
  html += "Sensor 1: " + String(c11, 2) + " A   " + String(v1, 2) + " V\n";
  html += "Sensor 2: " + String(c22, 2) + " A   " + String(v2, 2) + " V\n";
  html += "Sensor 3: " + String(c33, 2) + " A   " + String(v3, 2) + " V\n";
  html += "</pre><footer>";
  html += "IP: " + WiFi.localIP().toString();
  html += " | MAC: " + WiFi.macAddress();
  html += " | Gateway: " + WiFi.gatewayIP().toString();
  html += " | Subnet: " + WiFi.subnetMask().toString();
  html += " | DNS: " + WiFi.dnsIP().toString();
  html += "</footer></body></html>";

  server.send(200, "text/html", html);
}

// Signalstärke anzeigen (WLAN Balken rechts oben)
void drawWifiSignal(int strength) {
  int x = 110;
  int y = 0;
  int barWidth = 3;
  int gap = 2;

  int level = 0;
  if (strength > -50) level = 4;
  else if (strength > -60) level = 3;
  else if (strength > -70) level = 2;
  else if (strength > -80) level = 1;
  else level = 0;

  for (int i = 0; i < 4; i++) {
    int barHeight = (i + 1) * 2;
    if (i < level) {
      display.fillRect(x + i * (barWidth + gap), y + (8 - barHeight), barWidth, barHeight);
    } else {
      display.drawRect(x + i * (barWidth + gap), y + (8 - barHeight), barWidth, barHeight);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);

  display.init();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.drawString(0, 0, "OE9XVI - PsMon");
  display.drawString(0, 12, Release);
  display.display();
  delay(1500);

  // INA226 Initialisierung
  if (!INA0.begin()) Serial.println("INA0 Fehler");
  if (!INA1.begin()) Serial.println("INA1 Fehler");
  if (!INA2.begin()) Serial.println("INA2 Fehler");
  if (!INA3.begin()) Serial.println("INA3 Fehler");

  INA0.setMaxCurrentShunt(20, 0.002);
  INA1.setMaxCurrentShunt(20, 0.002);
  INA2.setMaxCurrentShunt(20, 0.002);
  INA3.setMaxCurrentShunt(20, 0.002);

  // WLAN mit WiFiManager
  WiFiManager wifiManager;
  display.drawString(0, 36, "Starte WiFi-Setup...");
  display.display();

  if (!wifiManager.autoConnect("OE9XVI-PsMon-AP")) {
    Serial.println("WLAN-Verbindung fehlgeschlagen. Neustart...");
    display.drawString(0, 48, "WLAN fehlgeschlagen");
    display.display();
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  // WLAN verbunden
  wlanOk = true;
  Serial.println("WLAN verbunden, IP: " + WiFi.localIP().toString());
  display.clear();
  display.drawString(0, 0, "WLAN verbunden");
  display.drawString(0, 12, WiFi.SSID());
  display.drawString(0, 24, WiFi.localIP().toString());
  display.display();
  delay(3000);  // 3 Sekunden anzeigen

  // OTA Setup
  ArduinoOTA.onStart([]() { Serial.println("OTA Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nOTA Ende"); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Fehler [%u]\n", error);
  });
  ArduinoOTA.begin();

  // Webserver
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP Server gestartet");
}

void loop() {
  if (wlanOk) {
    ArduinoOTA.handle();
    server.handleClient();
  }

  // Sensorwerte lesen
  c00 = INA0.getCurrent_mA() / 1000.0;
  v0 = INA0.getBusVoltage();
  c11 = INA1.getCurrent_mA() / 1000.0;
  v1 = INA1.getBusVoltage();
  c22 = INA2.getCurrent_mA() / 1000.0;
  v2 = INA2.getBusVoltage();
  c33 = INA3.getCurrent_mA() / 1000.0;
  v3 = INA3.getBusVoltage();

  // OLED aktualisieren
  display.clear();
  drawWifiSignal(WiFi.RSSI());  // Signalstärke-Symbol oben rechts
  display.drawString(0, 0, "OE9XVI - PsMon");
  display.drawString(0, 12, "0: " + String(c00, 2) + "A " + String(v0, 2) + "V");
  display.drawString(0, 24, "1: " + String(c11, 2) + "A " + String(v1, 2) + "V");
  display.drawString(0, 36, "2: " + String(c22, 2) + "A " + String(v2, 2) + "V");
  display.drawString(0, 48, "3: " + String(c33, 2) + "A " + String(v3, 2) + "V");
  display.display();

  delay(1000);
}
