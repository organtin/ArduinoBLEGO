/*
    arduino-imu-1010 v 2.0
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

    The code for this example file is partially stolen from the Arduino Science
    Journal firmware distributed with the examples in the Arduino IDE after 
    the installation of the Science Journal Library
 */

#include <ArduinoBLE.h>
#include <MKRIMU.h> 

#define SCIENCE_KIT_UUID(val) ("555a0002-" val "-467a-9538-01f0652c74e8")
#define VERSION 2

BLEService                     service                    (SCIENCE_KIT_UUID("0000"));
BLEUnsignedIntCharacteristic   versionCharacteristic      (SCIENCE_KIT_UUID("0001"), BLERead);
// the following are standard Arduino Science Journal characteristics
BLECharacteristic              accelerationCharacteristic (SCIENCE_KIT_UUID("0011"), BLENotify, 3 * sizeof(float));
BLECharacteristic              gyroscopeCharacteristic    (SCIENCE_KIT_UUID("0012"), BLENotify, 3 * sizeof(float));
BLECharacteristic              magneticFieldCharacteristic(SCIENCE_KIT_UUID("0013"), BLENotify, 3 * sizeof(float));
BLEFloatCharacteristic         temperatureCharacteristic  (SCIENCE_KIT_UUID("0014"), BLENotify);
BLEFloatCharacteristic         pressureCharacteristic     (SCIENCE_KIT_UUID("0015"), BLENotify);
BLEFloatCharacteristic         humidityCharacteristic     (SCIENCE_KIT_UUID("0016"), BLENotify);
BLEUnsignedIntCharacteristic   proximityCharacteristic    (SCIENCE_KIT_UUID("0017"), BLENotify);
BLECharacteristic              colorCharacteristic        (SCIENCE_KIT_UUID("0018"), BLENotify, 4 * sizeof(int));
BLEUnsignedShortCharacteristic soundPressureCharacteristic(SCIENCE_KIT_UUID("0019"), BLENotify);
// this is a custom characteristic holding the whole set of data packed into a bytearray
BLECharacteristic              imuData                    (SCIENCE_KIT_UUID("0020"), BLENotify, 7 * sizeof(float));

void setup() {
  Serial.begin(9600);
  // the builtin led is used to check if a client is connected
  pinMode(LED_BUILTIN, OUTPUT); 
  // initialise BLE
  BLE.begin();
  // initialise the IMU or any other device, if needed
  IMU.begin();

  delay(2000);

  BLE.setLocalName("ArduinoGO");
  BLE.setDeviceName("ArduinoGO");
  BLE.setAdvertisedService(service); 
  service.addCharacteristic(versionCharacteristic);
  // example: add standard characteristics to the service
  service.addCharacteristic(accelerationCharacteristic);
  service.addCharacteristic(gyroscopeCharacteristic);
  // example: add custom characteristic to the service
  service.addCharacteristic(imuData);

  versionCharacteristic.setValue(VERSION);
  
  // add the service and advertise it
  BLE.addService(service); 
  BLE.advertise();  
}

void toSerial(float *data, int n) {
  // helper function to write arrays to the serial connection
  for (int i = 0; i < n; i++) {
    Serial.print(data[i]);
    Serial.print(", ");
  }
  Serial.println();
}

void publishData() {
  // function to actually do the measurements and publish data both on the serial port
  // and BLE
  float gyroscope[3];
  float acceleration[3];

  // get data
  unsigned long t0 = micros();
  IMU.readEulerAngles(gyroscope[0], gyroscope[1], gyroscope[2]); 
  IMU.readAcceleration(acceleration[0], acceleration[1], acceleration[2]);
  unsigned long t1 = micros();
  
  // publish to BLE
  accelerationCharacteristic.writeValue((byte*)acceleration, sizeof(acceleration));
  gyroscopeCharacteristic.writeValue((byte*)gyroscope, sizeof(gyroscope));

  // build a custom characteristic with all the data packed together and publish it
  float imudata[7];
  imudata[0] = 0.5 * (t0 + t1);
  for (int i = 1; i <= 3; i++) {
     imudata[i] = acceleration[i];
     imudata[i + 3] = gyroscope[i];
  }

  // write to serial port
  toSerial(imudata, 7);

  imuData.writeValue((byte*)imudata, sizeof(imudata));
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
  }
  // if no connection, keep the builtin LED off though publish data 
  firstConnection = true;
  digitalWrite(LED_BUILTIN, LOW);
  publishData();
}
