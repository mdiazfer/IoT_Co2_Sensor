#include <Arduino.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEBeacon.h>
#include "global_setup.h"
#include "misc.h"

#ifndef _DISPLAYSUPPORTINFO_
  enum BLEStatus {BLEOnStatus,BLEConnectedStatus,BLEOffStatus};
#endif

extern RTC_DATA_ATTR byte mac[6];
extern RTC_DATA_ATTR boolean bluetoothEnabled;
extern RTC_DATA_ATTR BLEStatus BLEClurrentStatus;
extern RTC_DATA_ATTR float_t valueCO2, valueT;
extern RTC_DATA_ATTR uint64_t lastTimeBLEOnCheck;

extern TFT_eSprite stext1;
extern uint8_t pixelsPerLine,
    spL,            //Number of Lines in the Sprite
    scL,            //Number of Lines in the Scroll
    pFL,            //Pointer First Line
    pLL,            //pointer Last Line written
    spFL,           //Sprite First Line Window
    spLL,           //Sprite Last Line Window
    scFL,           //Scroll First Line Window
    scLL;           //Scroll Last Line Window

//extern uint32_t heapSizeNow,heapSizeLast;
extern BLEServer *pServer;
extern BLEAdvertising* pAdvertising;
extern bool isBeaconAdvertising;
extern bool deviceConnected;
extern BLEService* ptrService;
extern BLEBeacon* pBeacon;
extern BLEAdvertisementData* pAdvertisementData;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BLECharacteristic* pCharacteristic;
      pCharacteristic=pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID))->getCharacteristic(BLEUUID(BLE_CHARACT_CO2_UUID));
      pCharacteristic->setValue(valueT);
      
      deviceConnected = true;
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onConnect] - deviceConnected = true");
      BLEAdvertising* pAdvertising;
      pAdvertising = pServer->getAdvertising();
      pAdvertising->stop();
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onConnect] - Advertising stopped");
    };

    void onDisconnect(BLEServer* pServer) {
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onDisconnect] - deviceConnected = false");

      // Restart advertising to be visible and connectable again
      BLEAdvertising* pAdvertising;
      pAdvertising = pServer->getAdvertising();
      pAdvertising->start();
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onDisconnect] - Advertising restarted");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onWrite] - ************************************");
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onWrite] - Received Value");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onWrite] - ************************************");
      }
    }

    void onRead(BLECharacteristic *pCharacteristic) {

      if (pCharacteristic->getUUID().equals(BLEUUID(BLE_CHARACT_CO2_UUID))) {
          pCharacteristic->setValue(valueT);
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onRead] - Sent CO2 value");
      }

      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onRead] - Ends");
    }
};

extern void* myServerCallbacksObject;
extern void* myCallbacksObject;
extern void* BLE2902Object;

void BLEstop();
uint32_t BLEinit();
void setService(BLEAdvertising* pAdvertising);
void setiBeacon(BLEBeacon* ptriBeacon, BLEAdvertisementData* ptrAdvertisementData);
uint32_t  sendiBeacon();