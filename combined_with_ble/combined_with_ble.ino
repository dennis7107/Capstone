#include "Wire.h"
#include "DFRobot_MAX17043.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID                  "44d55770-bd07-4ce7-8ff9-c564c9c9b24a"
#define BATTERY_CHARACTERISTIC_UUID   "62391be9-a26b-4a82-8a13-e4db229df11e"
#define SPEED_CHARACTERISTIC_UUID     "a56f6a0e-164d-4d7c-b4cd-74afbdfeaf1f"

//flow sensor
const int FLOWSENSOR_INT                        = 2;
const int FLOWSENSOR_POWER                      = 4;
const float FLOWSENSOR_CALIBRATION              = 4.5;
volatile byte pulseCount                        = 0;
float flowRate                                  = 0;            // in L/min
int bikeSpeed                                   = 0;            // cm/s
unsigned long sensorOldTime                     = 0;

//solid-state relay
const int RELAY_PIN                             = 25;
const long RELAY_PRE_ON                         = 2000;
unsigned long relayOldTime                      = 0;

//battery fuel gauge ic
DFRobot_MAX17043 gauge;
const int GAUGE_POWER                           = 5;
const long GAUGE_INTERVAL                       = 10000;
float batteryVoltage                            = 10;
unsigned long gaugeOldTime                      = 0;
int SOC                                         = 0;

//upload data
const unsigned long UPLOAD_INTERVAL             = 1000;
unsigned long uploadOldTime                     = 0;
bool gaugeMeasure = false;
bool dataUpload = false;

//ble
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLEDescriptor pDescriptor(BLEUUID((uint16_t)0x2901));
BLEDescriptor pDescriptor2(BLEUUID((uint16_t)0x2901));
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


void setup() {
  Serial.begin(115200);

  pinMode(FLOWSENSOR_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(FLOWSENSOR_INT), pulseCounter, FALLING);
  pinMode(FLOWSENSOR_POWER, OUTPUT);
  digitalWrite(FLOWSENSOR_POWER, HIGH);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(GAUGE_POWER, OUTPUT);
  digitalWrite(GAUGE_POWER, HIGH);
//  while(gauge.begin() != 0) {
//    Serial.println("gauge begin failed!");
//    delay(2000);
//  }
//  delay(2);
//  Serial.println("gauge begin successful!");

  // ble
  BLEDevice::init("WaterBike");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
                                         BATTERY_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setValue(SOC);
  pDescriptor.setValue("Bike battery level");
  pCharacteristic->addDescriptor(&pDescriptor);

  pCharacteristic2 = pService->createCharacteristic(
                                         SPEED_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic2->setValue(flowRate);
  pDescriptor2.setValue("Water flow sensor reading");
  pCharacteristic2->addDescriptor(&pDescriptor2);

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  //pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x00);
  BLEDevice::startAdvertising();
  pAdvertising->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if ((millis() - sensorOldTime) > 1000)
  {
    flowSensorUpdate();
    sensorOldTime = millis();
  }
  if ((millis() - gaugeOldTime) > GAUGE_INTERVAL)
  {
    gaugeMeasure = true;
    digitalWrite(RELAY_PIN, HIGH);
    relayOldTime = millis();
    gaugeOldTime = millis();
  }
  if ((millis() - uploadOldTime) > UPLOAD_INTERVAL)
  {
    dataUpload = true;
    uploadOldTime = millis();
  }

  if (gaugeMeasure == true)
  {
    if ((millis() - relayOldTime) > RELAY_PRE_ON)
    {
    batteryVoltage = gauge.readVoltage();
    batteryVoltage = (float)batteryVoltage/3750*12.00;
    gaugeMeasure = false;
    digitalWrite(RELAY_PIN, LOW);
    }
  }

  if (deviceConnected) {
        if (dataUpload == true) {
          getStateOfCharge();
          pCharacteristic->setValue(SOC);
          pCharacteristic->notify();
          pCharacteristic2->setValue(bikeSpeed);
          pCharacteristic2->notify();
          Serial.println(bikeSpeed);
          dataUpload = false;
        }      
  }
  
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }

  
}


void pulseCounter()
{
  pulseCount++;
}

void flowSensorUpdate()
{
  detachInterrupt(digitalPinToInterrupt(FLOWSENSOR_INT));
  flowRate = ((1000.0 / (millis() - sensorOldTime)) * pulseCount) / FLOWSENSOR_CALIBRATION;
  bikeSpeed = flowRate * 1000 / 60 / 0.9503;
  pulseCount = 0;
  attachInterrupt(digitalPinToInterrupt(FLOWSENSOR_INT), pulseCounter, FALLING);
}

void getStateOfCharge()
{
  if (batteryVoltage>12.4) {
    SOC = 100;
  }
  else if (batteryVoltage>=11.2) {
    SOC = 100 - 25*(12.40-batteryVoltage);
  } 
  else if (batteryVoltage>=11.0) {
    SOC = 70 - 250*(11.20-batteryVoltage);
  }
  else if (batteryVoltage>=10.0) {
    SOC = 20 - 20*(11.00-batteryVoltage);
  }
  else {
    SOC = 0;
  }
  //Serial.println("reading battery soc");
}
