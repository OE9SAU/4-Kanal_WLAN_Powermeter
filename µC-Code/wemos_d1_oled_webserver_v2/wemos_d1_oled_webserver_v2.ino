#include <Wire.h>
#include <SH1106Wire.h>       // Von ESP8266-OLED Bibliothek https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "INA226.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>  // Für den HTTP-Webserver
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* Release = "V2.0 by OE9SAU";

// OLED Display
SH1106Wire display(0x3C, D2, D1);  // SDA = D2, SCL = D1

// INA226 Sensoren
INA226 INA0(0x40);
INA226 INA1(0x41);
INA226 INA2(0x44);
INA226 INA3(0x45);

// WLAN
const char* ssid = "VCOM";
const char* password = "Ms2210ems#";
IPAddress ip(192,168,1,220);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);

// Webserver
ESP8266WebServer server(5050);

// Messwerte
float c00, v0, c11, v1, c22, v2, c33, v3;

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                "<title>OE9XVI - PsMon</title>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<meta name='theme-color' content='#000000'>"
                "<meta http-equiv='refresh' content='5'>"
                "<style>"
                "body {background-color:black; color:white; font-family:monospace; margin:0; padding:10px;}"
                "h1 {text-align:center; font-size:28px; margin-bottom:5px;}"
                ".release {text-align:center; font-size:14px; margin-bottom:20px; color:gray;}"
                ".sensor-container {display:flex; flex-wrap:wrap; justify-content:center; gap:10px;}"
                ".sensor-card {background-color:#111; border:1px solid #333; border-radius:10px; padding:10px; width:90%; max-width:220px; text-align:center;}"
                ".sensor-card span {display:block; font-size:18px;}"
                "footer {font-size:12px; color:gray; text-align:center; margin-top:20px;}"
                "</style></head><body>";

  html += "<h1>OE9XVI Powersupply Monitor</h1>";
  html += "<div class='release'>" + String(Release) + "</div>";

  html += "<div class='sensor-container'>";
  html += "<div class='sensor-card'><span><strong>Sensor 0</strong></span><span>" + String(c00, 2) + " A</span><span>" + String(v0, 2) + " V</span></div>";
  html += "<div class='sensor-card'><span><strong>Sensor 1</strong></span><span>" + String(c11, 2) + " A</span><span>" + String(v1, 2) + " V</span></div>";
  html += "<div class='sensor-card'><span><strong>Sensor 2</strong></span><span>" + String(c22, 2) + " A</span><span>" + String(v2, 2) + " V</span></div>";
  html += "<div class='sensor-card'><span><strong>Sensor 3</strong></span><span>" + String(c33, 2) + " A</span><span>" + String(v3, 2) + " V</span></div>";
  html += "</div>";

  html += "<footer>IP: " + WiFi.localIP().toString() + " | MAC: " + WiFi.macAddress() + "</footer>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);

  // Display Setup
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

  // WLAN Setup
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden mit IP: " + WiFi.localIP().toString());

  // OTA optional
  ArduinoOTA.begin();

  // Webserver starten
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP Server gestartet");
}

void loop() {
  ArduinoOTA.handle();

  // Sensorwerte erfassen
  c00 = INA0.getCurrent_mA() / 1000.0; v0 = INA0.getBusVoltage();
  c11 = INA1.getCurrent_mA() / 1000.0; v1 = INA1.getBusVoltage();
  c22 = INA2.getCurrent_mA() / 1000.0; v2 = INA2.getBusVoltage();
  c33 = INA3.getCurrent_mA() / 1000.0; v3 = INA3.getBusVoltage();

  // OLED Ausgabe
  display.clear();
  display.drawString(0, 0, "OE9XVI - PsMon");
  display.drawString(0, 12, "0: " + String(c00, 2) + "A  " + String(v0, 2) + "V");
  display.drawString(0, 24, "1: " + String(c11, 2) + "A  " + String(v1, 2) + "V");
  display.drawString(0, 36, "2: " + String(c22, 2) + "A  " + String(v2, 2) + "V");
  display.drawString(0, 48, "3: " + String(c33, 2) + "A  " + String(v3, 2) + "V");
  display.display();

  // Webserver Anfragen bearbeiten
  server.handleClient();

  delay(1000);
}
