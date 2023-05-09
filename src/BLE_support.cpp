#include "BLE_support.h"


void BLEstop() {
  //Stop Advertisings and release memory only if other device is not connected, but only if no other priority event occurs
  if (pServer==nullptr) return;
  
  BLEDevice::stopAdvertising();
  //Release memory - Callback will be created agin next BLE_PERIOD loop
  delete(pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID))->getCharacteristic(BLEUUID(BLE_CHARACT_CO2_UUID)));
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted characteristic");}
  delete(pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID)));
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted service");}
  delete ((BLE2902*) BLE2902Object);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted BLE2902");}
  delete ((MyCallbacks*) myCallbacksObject);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted MyCallbacks");}
  delete ((MyServerCallbacks*) myServerCallbacksObject);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted MyServerCallbacks");}
  delete (pBeacon);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted pBeacon");}
  delete (pAdvertisementData);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted pAdvertisementData");}
  /*BLEDevice::removeAdvertising();pAdvertising=nullptr;
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted pAdvertising");}
  */
  delete(pServer);pServer=nullptr;
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - Deleted pServer");}
  BLEDevice::deinit(false);

  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEstop] - BLEstop exit");}
}


uint32_t BLEinit() {
  BLECharacteristic *pCharacteristic;
  BLEService* pService=nullptr;

  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - BLEinit in");}

  BLEDevice::init(std::string(String("se"+device.substring(12)).c_str()));
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - BLEDevice initiated");}
  
  if (pServer==nullptr) {
    //Usually after power up or reset. Need to create BLEServer, BLEService and BLECharacteristics
    pServer=BLEDevice::createServer();
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - BLEServer created");}

    if (pServer==nullptr) { 
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Creating BLEServer failed");}
      return ERROR_BLE_SETUP;
    }

    //Declare callbacks
    myServerCallbacksObject=new MyServerCallbacks();
    myCallbacksObject=new MyCallbacks();
    BLE2902Object=new BLE2902();
    
    //pServer->setCallbacks(new MyServerCallbacks());
    pServer->setCallbacks((MyServerCallbacks*) myServerCallbacksObject);
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pServer->setCallbacks configured");

    // Create the BLE Service as it was removed when switched BLE off
    pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID));
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Service created");
    
    if (pService==nullptr) { 
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pService is nullptr");}
      return ERROR_BLE_SETUP;
    }

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                      BLE_CHARACT_CO2_UUID,
                      //BLECharacteristic::PROPERTY_READ   
                      BLECharacteristic::PROPERTY_READ   |
                      //BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
    //pCharacteristic->setCallbacks(new MyCallbacks());
    //pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks((MyCallbacks*) myCallbacksObject);
    pCharacteristic->addDescriptor((BLE2902* )BLE2902Object);
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pCharacteristic configured");

    if (pAdvertising==nullptr) {
      pAdvertising = pServer->getAdvertising();
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pAdvertising is got");}
    }

    if (pAdvertising==nullptr) { 
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pAdvertising is nullptr");}
      return ERROR_BLE_SETUP;
    }

    pAdvertising->addServiceUUID(BLEUUID(BLE_SERVICE_UUID));
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Service added to the Advertising");

    // Start the service
    pService->start();
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Service started");
  }
  else {
    //Declare callbacks
    myServerCallbacksObject=new MyServerCallbacks();
    myCallbacksObject=new MyCallbacks();
    BLE2902Object=new BLE2902();
    //pCharacteristic=pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID))->getCharacteristic(BLEUUID(BLE_CHARACT_CO2_UUID));
    pCharacteristic = pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID))->createCharacteristic(
                      BLE_CHARACT_CO2_UUID,
                      //BLECharacteristic::PROPERTY_READ   
                      BLECharacteristic::PROPERTY_READ   |
                      //BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
    
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Declared callbacks");

    pServer->setCallbacks((MyServerCallbacks*) myServerCallbacksObject);
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pServer->setCallbacks configured");

    pCharacteristic->setCallbacks((MyCallbacks*) myCallbacksObject);
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pCharacteristic->setCallbacks configured");

    pCharacteristic->addDescriptor((BLE2902* )BLE2902Object);
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pCharacteristic configured");
  }

  //Add Beacon to the Advertising - Every time due to BLEBeacon & BLEAdvertisementData object memory is realeasd
  if (pAdvertising==nullptr) pAdvertising = pServer->getAdvertising();

  if (pAdvertising==nullptr) { 
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - pAdvertising is nullptr");}
    return ERROR_BLE_SETUP;
  }

  /*BLEBeacon myBeacon;
  myBeacon.setManufacturerId(BEACON_MANUFACTURER);
  myBeacon.setMajor(5);
  myBeacon.setMinor(88);
  myBeacon.setSignalPower(BEACON_RSSI);
  myBeacon.setProximityUUID(BLEUUID(BEACON_UUID_REV));
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Beacon configured");

  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x1A);
  advertisementData.setManufacturerData(myBeacon.getData());
  pAdvertising->setAdvertisementData(advertisementData);
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Beacon added to the Advertising");
  */

  pBeacon=new BLEBeacon();
  pBeacon->setManufacturerId(BEACON_MANUFACTURER);

  uint16_t aux=(mac[2]<<8)|mac[3];
  pBeacon->setMajor(aux);
  aux=(mac[4]<<8)|mac[5];
  pBeacon->setMinor(aux);
  pBeacon->setSignalPower(BEACON_RSSI);
  pBeacon->setProximityUUID(BLEUUID(BEACON_UUID_REV));
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Beacon configured");

  pAdvertisementData=new BLEAdvertisementData();
  pAdvertisementData->setFlags(0x1A);
  pAdvertisementData->setManufacturerData(pBeacon->getData());
  pAdvertising->setAdvertisementData(*pAdvertisementData);
  pAdvertising->setDeviceAddress(mac,BLE_ADDR_TYPE_PUBLIC);

  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Beacon added to the Advertising");


  // Start Advertising
  pAdvertising->start();
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [BLEinit] - Service+iBeacon Advertising started");

  if (deviceConnected) {
    BLECharacteristic* pCharacteristic;
    pCharacteristic=pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID))->getCharacteristic(BLEUUID(BLE_CHARACT_CO2_UUID));
    pCharacteristic->setValue(valueT);
    //pCharacteristic->notify();
  }

  //Stop and realease memory
  //BLEDevice::stopAdvertising();
  //BLEDevice::deinit(false);

  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [BLEinit] - BLEinit exit");}
  return 0;
}

void setService(BLEAdvertising* pAdvertising) {
  BLECharacteristic *pCharacteristic;

  //Creating Advertising the very first time, usually after power up
  if (pAdvertising==nullptr) 
  {
    pAdvertising=pServer->getAdvertising();

    pAdvertising->stop();
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setService] - Advertising stopped to setup Service");

    // Create the BLE Service
    BLEService *pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID));
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setService] - Service created");

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        BLE_CHARACT_CO2_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_WRITE  |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setService] - pCharacteristic configured");

    pAdvertising->addServiceUUID(BLEUUID(BLE_SERVICE_UUID));
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setService] - Service added to the Advertising");

    // Start the service
    pService->start();
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setService] - Service started");
  }

}

void setiBeacon(BLEBeacon* ptriBeacon, BLEAdvertisementData* ptrAdvertisementData) {
  pAdvertising->stop();
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setiBeacon] - Advertising stopped to setup iBeacon");
  
  //Setting iBeacon characteristics
  ptriBeacon->setManufacturerId(BEACON_MANUFACTURER);
  uint16_t aux=(mac[2]<<8)|mac[3];
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setiBeacon] - iBeacon Major= 0x"+String(aux,HEX));
  ptriBeacon->setMajor(aux);
  aux=(mac[4]<<8)|mac[5];
  ptriBeacon->setMinor(aux);
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setiBeacon] - iBeacon Minor= 0x"+String(aux,HEX));
  ptriBeacon->setSignalPower(BEACON_RSSI);
  ptriBeacon->setProximityUUID(BLEUUID(BEACON_UUID_REV));
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setiBeacon] - Proximity="+BEACON_UUID);

  //Seeting iBeacon Advertisement
  ptrAdvertisementData->setFlags(BEACON_FLAGS);
  ptrAdvertisementData->setManufacturerData(ptriBeacon->getData());
}

uint32_t sendiBeacon() {
  uint32_t error;
  BLEBeacon myBeacon;
  BLEAdvertisementData advertisementData;
  BLEService *pService;
  BLECharacteristic *pCharacteristic;

  isBeaconAdvertising=true;
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [sendiBeacon] - isBeaconAdvertising set");}
  
  //Create first the BLEServer and BLEAdvertisement
  BLEDevice::init(std::string(device.c_str()));
  if (pServer==nullptr) pServer=BLEDevice::createServer();

  if (pServer==nullptr) { 
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [sendiBeacon] - Creating BLEServer failed");}
    BLEClurrentStatus=BLEOffStatus;
    isBeaconAdvertising=false;
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [sendiBeacon] - isBeaconAdvertising unset");}
    return ERROR_BLE_SETUP;
  }

  BLEClurrentStatus=BLEOnStatus;
  //Creating Advertising the very first time, usually after power up
  if (pAdvertising==nullptr) pAdvertising=pServer->getAdvertising();

  //Configuring Service Adverising
  //Creating BLE Service and BLE CO2 Characteristic
  /*
  pService=pServer->createService(BLEUUID(BLE_SERVICE_UUID));
  pCharacteristic=pService->createCharacteristic(BLE_CHARACT_CO2_UUID,BLECharacteristic::PROPERTY_READ);
  pCharacteristic->addDescriptor(new BLE2902());
  pAdvertising->addServiceUUID(BLEUUID(BLE_SERVICE_UUID));
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [sendiBeacon] - starting Service, etc");}
  pService->start();
  */

  //Configuring the iBeacon Advertising
  setiBeacon(&myBeacon,&advertisementData);

  //Set Server Advertisement
  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  pAdvertising->setDeviceAddress(mac,BLE_ADDR_TYPE_RANDOM);

  //Sending Advertising for a few milliseconds
  pAdvertising->start();
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [sendiBeacon] - iBeacon configured. Advertisement started");

  
  //Stop sending iBeacon and releasing memory
  //pService->stop();
  //free(pService); free(pCharacteristic); 
  BLEDevice::stopAdvertising();
  BLEDevice::deinit(false);
  
  /*logRamStats("After BLEDevice deInit");
  heapSizeNow=esp_get_free_heap_size();
  Serial.printf("Heap=%d, consumed=%d\n",heapSizeNow,heapSizeLast-heapSizeNow);
  heapSizeLast=heapSizeNow;*/
  
  isBeaconAdvertising=false;
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [sendiBeacon] - isBeaconAdvertising unset");}
  return 0;
}