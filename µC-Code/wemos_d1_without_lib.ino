#include <Wire.h>

// INA226 Registeradresse und Funktionen (ohne extra Library, direkt über I2C)
// Alternativ kannst du die INA226-Bibliothek von Adafruit oder ähnliche nutzen.
// Hier einfacher Direktzugriff für Grundfunktionalität.

#define INA226_ADDR 0x40 // Standard-I2C-Adresse

// INA226 Register
#define REG_CONFIG        0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE   0x02
#define REG_POWER         0x03
#define REG_CURRENT       0x04
#define REG_CALIBRATION   0x05

// Kalibrierwerte für 20A max mit 2mΩ Shunt
#define CALIBRATION_VALUE 4096

// ILSB = 0.000625 A (625uA)
#define CURRENT_LSB 0.000625f

void writeRegister(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(INA226_ADDR);
  Wire.write(reg);
  Wire.write(value >> 8);
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

uint16_t readRegister(uint8_t reg) {
  Wire.beginTransmission(INA226_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom(INA226_ADDR, (uint8_t)2);
  uint16_t val = (Wire.read() << 8) | Wire.read();
  return val;
}

void setupINA226() {
  // Konfiguration:
  // AVG=16, Bus Conversion Time=1100us, Shunt Conversion Time=1100us, Mode=Shunt + Bus continuous
  uint16_t config = 0x4127; // Werte nach Datenblatt Beispiel
  writeRegister(REG_CONFIG, config);
  
  // Kalibrierregister setzen
  writeRegister(REG_CALIBRATION, CALIBRATION_VALUE);
}

float readShuntVoltage() {
  int16_t val = (int16_t)readRegister(REG_SHUNT_VOLTAGE);
  // Shunt Spannung in Volt: LSB = 2.5uV
  return val * 0.0000025f;
}

float readBusVoltage() {
  uint16_t val = readRegister(REG_BUS_VOLTAGE);
  // Bus Spannung in Volt: LSB = 1.25mV, Bit 0 und 1 sind Statusbits
  return (val >> 3) * 0.00125f;
}

float readCurrent() {
  int16_t val = (int16_t)readRegister(REG_CURRENT);
  // Current Register LSB = Current_LSB (hier 0.625mA)
  return val * CURRENT_LSB;
}

float readPower() {
  uint16_t val = readRegister(REG_POWER);
  // Power Register LSB = 25 × Current_LSB
  return val * (25.0f * CURRENT_LSB);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); // SDA = D2, SCL = D1 auf Wemos D1
  delay(100);
  
  setupINA226();
  Serial.println("INA226 mit 2mOhm Shunt, max 20A Kalibrierung gestartet.");
}

void loop() {
  float shuntV = readShuntVoltage();
  float busV = readBusVoltage();
  float current = readCurrent();
  float power = readPower();
  
  Serial.print("Shunt Voltage: "); Serial.print(shuntV * 1000, 2); Serial.print(" mV, ");
  Serial.print("Bus Voltage: "); Serial.print(busV, 3); Serial.print(" V, ");
  Serial.print("Current: "); Serial.print(current, 3); Serial.print(" A, ");
  Serial.print("Power: "); Serial.print(power, 3); Serial.println(" W");
  
  delay(1000);
}
