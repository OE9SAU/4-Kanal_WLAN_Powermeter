#include <Wire.h>
#include <Adafruit_INA226.h>

Adafruit_INA226 ina = Adafruit_INA226();

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);  // SDA = D2, SCL = D1

  if (!ina.begin()) {
    Serial.println("INA226 nicht gefunden. Check Verbindung!");
    while (1);
  }

  // Kalibrierung für 2mΩ Shunt, max 20A
  // Current_LSB = 0.000625 A (625 µA)
  // Calibration = 4096 (aus Berechnung)
  ina.setCalibration_32V_2A();  // default, aber wir überschreiben gleich

  // Eigene Kalibrierung setzen:
  // Leider setzt Adafruit keine direkte Kalibrierung, 
  // aber man kann den Shunt-Widerstand einstellen (Ohm)
  ina.setShuntResistor(0.002);  // 2 mΩ

  // Wähle Current_LSB manuell über setCurrentLSB (kann man anpassen)
  ina.setCurrentLSB(0.000625);  // 625 µA LSB

  Serial.println("INA226 bereit für Messungen bis 20A.");
}

void loop() {
  float current = ina.readCurrent();     // Strom in Ampere
  float busVoltage = ina.readBusVoltage(); // Bus-Spannung in Volt
  float power = ina.readPower();           // Leistung in Watt
  float shuntVoltage = ina.readShuntVoltage(); // Shuntspannung in Volt

  Serial.print("Strom: "); Serial.print(current, 3); Serial.print(" A, ");
  Serial.print("Bus-Spannung: "); Serial.print(busVoltage, 3); Serial.print(" V, ");
  Serial.print("Shunt-Spannung: "); Serial.print(shuntVoltage * 1000, 2); Serial.print(" mV, ");
  Serial.print("Leistung: "); Serial.print(power, 3); Serial.println(" W");

  delay(1000);
}
