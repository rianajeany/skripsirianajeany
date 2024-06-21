#include <Adafruit_BME680.h>
#include <DFRobot_MICS.h>
#include "MHZ19.h"
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

Adafruit_BME680 bme;
DFRobot_MICS_I2C mics(&Wire, 0x75);
MHZ19 mhz19;                           
HardwareSerial mhSerial(2);  // RX:16, TX:17

void setup() {
  Serial.begin(115200);
   WiFi.begin("TP-Link_BF14", "14315619");
  while (!WiFi.isConnected()) delay(500);
  while (!bme.begin()) { Serial.println("BME: could not find sensor."); delay(1000); }
  mics.begin();
  if (mics.getPowerState() == SLEEP_MODE) mics.wakeUpMode();
  Serial.println("warming up 3 mins...");
  // while (!mics.warmUpTime(3)) delay(5000);  // default: 3 mins
  mhSerial.begin(9600);
  mhz19.begin(mhSerial);
  mhz19.autoCalibration();
}

void loop() {
  if (!bme.performReading()) { Serial.println("BME: failed to read."); return; }

  int bme680_gas = bme.gas_resistance;
  int mhz19_co2 = mhz19.getCO2();
  float mics4514_nh3 = mics.getGasData(NH3);
  float mics4514_co = mics.getGasData(CO);
  float mics4514_no2 = mics.getGasData(NO2);
  int mics6814_nh3 = analogRead(35);
  int mics6814_co  = analogRead(34);
  int mics6814_no2 = analogRead(32);
 
  
  Serial.print(bme680_gas);       Serial.print(", ");   // ohm
  Serial.println(mhz19_co2);
  Serial.print(mics4514_nh3);     Serial.print(", ");   // CO, CH4, C2H5OH, H2, NH3, NO2
  Serial.print(mics4514_co);      Serial.print(", ");
  Serial.print(mics4514_no2);     Serial.println();
  Serial.print(mics6814_nh3);     Serial.print(", ");
  Serial.print(mics6814_co);      Serial.print(", ");
  Serial.print(mics6814_no2);     Serial.println();

  WiFiClient wifi;
  HTTPClient http;
    
  http.begin(wifi, "http://api.thingspeak.com/update");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST("api_key=5OWTV7IS7Z736H3X&field1=" + String(bme680_gas) + "&field2=" + mhz19_co2 + "&field3=" + mics4514_nh3 + "&field4=" + mics6814_nh3 + 
  "&field5=" + mics4514_co + "&field6=" + mics6814_co + "&field7=" + mics4514_no2 + "&field8=" + mics6814_no2);
  http.end();
 
  delay(15000);
}
