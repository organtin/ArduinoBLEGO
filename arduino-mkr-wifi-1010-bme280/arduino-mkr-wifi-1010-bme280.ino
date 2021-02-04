/*
    arduino-mkr-wifi-1010-bme280 v 2.0
    Copyright (C) 2020  Giovanni Organtini giovanni.organtini@uniroma1.it

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    The code for this example file has been partially stolen from the Arduino Science
    Journal firmware distributed with the examples in the Arduino IDE after 
    the installation of the Science Journal Library
 */

#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 sensor; 

#define SCIENCE_KIT_UUID(val) ("555a0002-" val "-467a-9538-01f0652c74e8")
#define VERSION 2

BLEService                     service                    (SCIENCE_KIT_UUID("0000"));
BLEUnsignedIntCharacteristic   versionCharacteristic      (SCIENCE_KIT_UUID("0001"), BLERead);
// the following are standard Arduino Science Journal characteristics
BLEFloatCharacteristic         temperatureCharacteristic  (SCIENCE_KIT_UUID("0014"), BLENotify);
BLEFloatCharacteristic         pressureCharacteristic     (SCIENCE_KIT_UUID("0015"), BLENotify);
// composite characteristic
BLECharacteristic              bme280Data                 (SCIENCE_KIT_UUID("0020"), BLENotify, 3 * sizeof(float));

void setup() {
  Serial.begin(9600);
  // the builtin led is used to check if a client is connected
  pinMode(LED_BUILTIN, OUTPUT); 
  // initialise BLE
  BLE.begin();
  Serial.println("BLE ok");
  // initialise the sensor
  sensor.begin(0x76);
  Serial.println("sensor ok");

  delay(2000);

  BLE.setLocalName("ArduinoGO");
  BLE.setDeviceName("ArduinoGO");
  BLE.setAdvertisedService(service); 
  service.addCharacteristic(versionCharacteristic);
  service.addCharacteristic(temperatureCharacteristic);
  service.addCharacteristic(pressureCharacteristic);

  service.addCharacteristic(bme280Data);

  versionCharacteristic.setValue(VERSION);
  
  // add the service and advertise it
  BLE.addService(service); 
  BLE.advertise();  
}

void publishData() {
  unsigned long t0 = micros();
  float T = sensor.readTemperature();
  float p = sensor.readPressure()/1.0e4;
  unsigned long t1 = micros();
  
  // publish to BLE
  temperatureCharacteristic.writeValue(T);
  pressureCharacteristic.writeValue(p);

  float pTdata[3];
  pTdata[0] = 0.5*(t0+t1);
  pTdata[1] = p;
  pTdata[2] = T;
  bme280Data.writeValue((byte*)pTdata, sizeof(pTdata));
} 

bool firstConnection = true;

void loop() {
  while (BLE.connected()) {
    // once a client connects stay in the loop and publish data
    if (firstConnection) {
      // as soon as the client connect write a message to the serial port and swicth on the 
      // built in LED
      BLEDevice central = BLE.central();
      Serial.print("# Connected to ");
      Serial.println(central.address());
      firstConnection = false;
      digitalWrite(LED_BUILTIN, HIGH);
    }
    Serial.print("*** ");
    publishData();
    delay(1000);
  }
  // if no connection, keep the builtin LED off though publish data 
  firstConnection = true;
  digitalWrite(LED_BUILTIN, LOW);
  publishData();
  delay(1000);
}
