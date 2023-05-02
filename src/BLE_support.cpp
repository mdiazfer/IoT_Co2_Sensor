#include "BLE_support.h"


void setServiceAdvertising(BLEAdvertisementData* ptrAdvertisementData, BLEService *pService) {
  //Setting Service Advertising
  
  
}

void setiBeacon(BLEBeacon* ptriBeacon, BLEAdvertisementData* ptrAdvertisementData) {
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