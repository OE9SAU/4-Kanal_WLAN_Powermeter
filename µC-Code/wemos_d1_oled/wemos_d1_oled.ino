#include <Wire.h>
#include <SH1106Wire.h>       // Von ESP8266-OLED Bibliothek https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <INA226.h>       	  // https://github.com/RobTillaart/INA226

// Display: SH1106 128x64, I2C, Adresse 0x3C
SH1106Wire display(0x3C, D2, D1);  // SDA = D2, SCL = D1

// INA226 Sensoren
INA226 INA0(0x40);
INA226 INA1(0x41);
INA226 INA2(0x44);
INA226 INA3(0x45);

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);  // Sicherstellen, dass I2C korrekt gesetzt ist

  // Display initialisieren
  display.init();
  //display.flipScreenVertically();   // 180° drehen
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.clear();
  display.drawString(0, 0, "INA226 Strom & Spannung");
  display.display();
  delay(1500);

  // INA226 initialisieren
  if (!INA0.begin()) Serial.println("INA0 Fehler");
  if (!INA1.begin()) Serial.println("INA1 Fehler");
  if (!INA2.begin()) Serial.println("INA2 Fehler");
  if (!INA3.begin()) Serial.println("INA3 Fehler");

  INA0.setMaxCurrentShunt(20, 0.002);
  INA1.setMaxCurrentShunt(20, 0.002);
  INA2.setMaxCurrentShunt(20, 0.002);
  INA3.setMaxCurrentShunt(20, 0.002);
}

void loop() {
  float c0 = INA0.getCurrent_mA();
  float c00 = c0 / 1000.0;          // Umrechnung in A
  float v0 = INA0.getBusVoltage();
  float c1 = INA1.getCurrent_mA();
  float c11 = c1 / 1000.0;          // Umrechnung in A
  float v1 = INA1.getBusVoltage();
  float c2 = INA2.getCurrent_mA();
  float c22 = c2 / 1000.0;          // Umrechnung in A
  float v2 = INA2.getBusVoltage();
  float c3 = INA3.getCurrent_mA();
  float c33 = c3 / 1000.0;          // Umrechnung in A
  float v3 = INA3.getBusVoltage();

  display.clear();

  display.drawString(0, 0,  "0: " + String(c00, 2) + "A  " + String(v0, 1) + "V");
  display.drawString(0, 12, "1: " + String(c11, 2) + "A  " + String(v1, 1) + "V");
  display.drawString(0, 24, "2: " + String(c22, 2) + "A  " + String(v2, 1) + "V");
  display.drawString(0, 36, "3: " + String(c33, 2) + "A  " + String(v3, 1) + "V");

  display.display();
  delay(1000);
}
