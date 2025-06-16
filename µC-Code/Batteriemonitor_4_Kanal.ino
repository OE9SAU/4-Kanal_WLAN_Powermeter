/* Release Notes
V1.0: First step, starting the project
V2.0: angepasst von INA 219 auf INA226 Adresse 0X40
V3.0: erweitert auf 4 INA226 Sensoren mit 2mΩ Shunt / 20A
*/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <INA226_WE.h>  // https://wolles-elektronikkiste.de/ina226

#define NUM_INAS 4
const uint8_t inaAddresses[NUM_INAS] = { 0x40, 0x41, 0x42, 0x43 };
INA226_WE ina226[NUM_INAS] = {
  INA226_WE(inaAddresses[0]),
  INA226_WE(inaAddresses[1]),
  INA226_WE(inaAddresses[2]),
  INA226_WE(inaAddresses[3])
};

float shuntVoltage_mV[NUM_INAS];
float loadVoltage_V[NUM_INAS];
float busVoltage_V[NUM_INAS];
float current_mA[NUM_INAS];
float power_mW[NUM_INAS];

const char* ssid = "VCOM";
const char* password = "Ms2210ems#";
const char* Release = "V3.0";

IPAddress ip(192,168,1,220);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);
WiFiServer server(5050);

#define DEBUG_MODE false

void setup() {
  Serial.begin(115200);
  delay(10);
  Wire.begin();

  // Initialisierung aller INA226
  for (int i = 0; i < NUM_INAS; i++) {
    if (!ina226[i].init()) {
      Serial.print("INA226 Sensor #");
      Serial.print(i + 1);
      Serial.println(" nicht gefunden!");
      while (1);
    }
    ina226[i].setResistorRange(0.002, 20.0); // 2 mΩ Shunt, 20 A max
    ina226[i].waitUntilConversionCompleted();
  }

  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet, dns);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden");

  // OTA Setup
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("Solarladeregler@OE9XVI");
  ArduinoOTA.setPassword("OTA");
  ArduinoOTA.begin();

  server.begin();
  delay(500);
  Serial.println("Server gestartet");
  Serial.print("Zugriff über: http://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(server.port());
}

void loop() {
  ArduinoOTA.handle();

  for (int i = 0; i < NUM_INAS; i++) {
    shuntVoltage_mV[i] = ina226[i].getShuntVoltage_mV();
    busVoltage_V[i] = ina226[i].getBusVoltage_V();
    current_mA[i] = ina226[i].getCurrent_mA();
    power_mW[i] = ina226[i].getBusPower();
    loadVoltage_V[i] = busVoltage_V[i] + (shuntVoltage_mV[i] / 1000.0);
  }

  if (DEBUG_MODE) {
    for (int i = 0; i < NUM_INAS; i++) {
      Serial.print("INA226 #"); Serial.print(i + 1); Serial.println(":");
      Serial.print("  Load Voltage [V]: "); Serial.println(loadVoltage_V[i]);
      Serial.print("  Current [mA]: "); Serial.println(current_mA[i]);
      Serial.print("  Power [mW]: "); Serial.println(power_mW[i]);
      Serial.println(ina226[i].overflow ? "  Overflow!" : "  OK");
    }
    delay(3000);
  }

  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Neuer Client");
  while (!client.available()) delay(100);
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  String macAddress = WiFi.macAddress();
  String gatewayIP = WiFi.gatewayIP().toString();
  String subnetMask = WiFi.subnetMask().toString();
  String dnsIP = WiFi.dnsIP().toString();

  // HTML Ausgabe
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML><html lang='de' style='background: linear-gradient(to right, #ff7e5f, #feb47b); color:white;'>");
  client.println("<head><title>Solarladeregler @ OE9XVI</title>");
  client.println("<meta http-equiv='refresh' content='30'>");
  client.println("<style>");
  client.println("body { font-family: 'Roboto', sans-serif; margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; background: #2e3b4e; }");
  client.println("h1 { font-size: 36px; color: #fff; text-align: center; margin: 10px; }");
  client.println("div { background-color: rgba(0, 0, 0, 0.7); border-radius: 10px; padding: 20px; margin: 10px; width: 300px; text-align: center; font-size: 20px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.5); }");
  client.println("footer { font-size: 12px; color: #bbb; text-align: center; margin-top: 30px; }");
  client.println("span { font-weight: bold; }");
  client.println("</style></head><body>");
  client.println("<h1>Solarladeregler @ OE9XVI</h1>");
  client.println("<p>Release: " + String(Release) + " by OE9SAU</p>");

  for (int i = 0; i < NUM_INAS; i++) {
    client.println("<div>");
    client.println("<h2>Sensor #" + String(i + 1) + "</h2>");
    client.println("Spannung: <span>" + String(loadVoltage_V[i], 2) + " V</span><br>");
    client.println("Strom: <span>" + String(current_mA[i], 2) + " mA</span><br>");
    client.println("Leistung: <span>" + String(power_mW[i], 2) + " mW</span>");
    client.println("</div>");
  }

  unsigned int freeMemory = ESP.getFreeSketchSpace();
  unsigned int flashSize = ESP.getFlashChipSize();
  float freeMemoryPercentage = (float(freeMemory) / float(flashSize)) * 100;

  client.println("<footer>");
  client.println("<p>Freier Speicherplatz: " + String(freeMemoryPercentage, 0) + "% | ");
  client.println("IP: " + WiFi.localIP().toString() + " | MAC: " + macAddress + " | Gateway: " + gatewayIP + " | Subnetz: " + subnetMask + " | DNS: " + dnsIP + "</p>");
  client.println("</footer>");
  client.println("</body></html>");

  delay(1000);
}
