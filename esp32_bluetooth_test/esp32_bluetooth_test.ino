#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID                    "44d55770-bd07-4ce7-8ff9-c564c9c9b24a"
#define BATTERY_CHARACTERISTIC_UUID     "62391be9-a26b-4a82-8a13-e4db229df11e"
#define SPEED_CHARACTERISTIC_UUID       "a56f6a0e-164d-4d7c-b4cd-74afbdfeaf1f"


BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLEDescriptor pDescriptor(BLEUUID((uint16_t)0x2901));
BLEDescriptor pDescriptor2(BLEUUID((uint16_t)0x2901));
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint16_t value = 88;
float value2 = 10.52f;



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
  Serial.println("Starting BLE work!");

  BLEDevice::init("WaterBike");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
                                         BATTERY_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setValue(value);
  pDescriptor.setValue("Bike battery level");
  pCharacteristic->addDescriptor(&pDescriptor);
  pCharacteristic->addDescriptor(new BLE2902());

  pCharacteristic2 = pService->createCharacteristic(
                                         SPEED_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic2->setValue(value2);
  pDescriptor2.setValue("Water flow sensor reading");
  pCharacteristic2->addDescriptor(&pDescriptor2);
//  pCharacteristic2->addDescriptor(new BLE2902());
  
  pService->start();
  //BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  //pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x00);
  BLEDevice::startAdvertising();
  pAdvertising->start();
  Serial.println("Waiting a client connection to notify...");
  Serial.print("SERVICE_UUID: ");
  Serial.print(SERVICE_UUID);
  Serial.print("    BATTERY_CHARACTERISTIC_UUID:  ");
  Serial.println(BATTERY_CHARACTERISTIC_UUID);
}

void loop() {
  // notify changed value
    if (deviceConnected) {
        pCharacteristic->setValue(value);
        pCharacteristic->notify();
        pCharacteristic2->notify();
//        value++;
//        if (value == 101) {
//          value = 0;
//        }
        Serial.println("connected");
        delay(100); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
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
