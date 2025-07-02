// INA226 Libary
// https://github.com/RobTillaart/INA226
//

#include "INA226.h"

INA226 INA0(0x40);
INA226 INA1(0x41);
INA226 INA2(0x44);
INA226 INA3(0x45);

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("INA226_LIB_VERSION: ");
  Serial.println(INA226_LIB_VERSION);

  Wire.begin();

  if (!INA0.begin()) Serial.println("INA0 (0x40) could not connect.");
  if (!INA1.begin()) Serial.println("INA1 (0x41) could not connect.");
  if (!INA2.begin()) Serial.println("INA2 (0x44) could not connect.");
  if (!INA3.begin()) Serial.println("INA3 (0x45) could not connect.");

  // Optional: Set max current and shunt values
  INA0.setMaxCurrentShunt(2, 0.002);  // 2A, 2 mΩ
  INA1.setMaxCurrentShunt(2, 0.002);
  INA2.setMaxCurrentShunt(2, 0.002);
  INA3.setMaxCurrentShunt(2, 0.002);
}

void loop()
{
  Serial.println("\nBUS\tSHUNT\tCURRENT\tPOWER\t||\tBUS\tSHUNT\tCURRENT\tPOWER");
  for (int i = 0; i < 20; i++)
  {
    printSensor(INA0);
    Serial.print("\t||\t");
    printSensor(INA1);
    Serial.println();

    printSensor(INA2);
    Serial.print("\t||\t");
    printSensor(INA3);
    Serial.println();

    Serial.println("------------------------------------------------------------");
    delay(1000);
  }
}

void printSensor(INA226& sensor)
{
  Serial.print(sensor.getBusVoltage(), 3);
  Serial.print("\t");
  Serial.print(sensor.getShuntVoltage_mV(), 3);
  Serial.print("\t");
  Serial.print(sensor.getCurrent_mA(), 3);
  Serial.print("\t");
  Serial.print(sensor.getPower_mW(), 3);
}
