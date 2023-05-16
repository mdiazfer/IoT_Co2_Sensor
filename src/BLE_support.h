#include <Arduino.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEBeacon.h>
#include "global_setup.h"
#include "misc.h"
#include "soc/rtc_wdt.h"

#ifndef _DISPLAYSUPPORTINFO_
  enum BLEStatus {BLEOnStatus,BLEConnectedStatus,BLEStandbyStatus,BLEOffStatus};
#endif

extern RTC_DATA_ATTR byte mac[6];
extern RTC_DATA_ATTR boolean bluetoothEnabled,forceDisplayRefresh;
extern RTC_DATA_ATTR BLEStatus BLECurrentStatus;
extern RTC_DATA_ATTR float_t valueCO2, valueT, valueHum;
extern RTC_DATA_ATTR uint64_t lastTimeBLEOnCheck;
extern RTC_DATA_ATTR uint32_t minHeapSeen;
extern RTC_DATA_ATTR enum displayModes displayMode;

extern uint32_t heapSizeNow;
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
extern bool isBeaconAdvertising,deviceConnected,webServerResponding;
extern BLECharacteristic* pCharacteristicCO2;
extern BLECharacteristic* pCharacteristicT;
extern BLECharacteristic* pCharacteristicHum;
extern BLEService* pService;
extern BLEBeacon* pBeacon;
extern BLEAdvertisementData* pAdvertisementData;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      //pServer->getServiceByUUID(BLEUUID(BLE_SERVICE_UUID))->getCharacteristic(BLEUUID(BLE_CHARACT_CO2_UUID))->setValue(valueCO2);
      pCharacteristicCO2->setValue(valueCO2);  //pCharacteristicCO2 is global pointer
      pCharacteristicT->setValue(valueT);  //pCharacteristicT is global pointer
      pCharacteristicHum->setValue(valueHum);  //pCharacteristicHum is global pointer

      deviceConnected = true;
      BLECurrentStatus=BLEConnectedStatus;
      if (displayMode==sampleValue) forceDisplayRefresh=true; //Refresh Icons in the next loop cycle
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onConnect] - deviceConnected = true");
    };

    void onDisconnect(BLEServer* pServer) {
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onDisconnect] - deviceConnected = false");

      // Restart advertising to be visible and connectable again
      //pServer->getAdvertising()->start();
      pAdvertising->start(); //pAdvertising is global pointer
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onDisconnect] - Advertising restarted");
      deviceConnected = false;
      BLECurrentStatus=BLEStandbyStatus;
      if (displayMode==sampleValue) forceDisplayRefresh=true; //Refresh Icons in the next loop cycle
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
      BLEUUID Co2BLEUUID;

      Co2BLEUUID=pCharacteristic->getUUID();

      if (Co2BLEUUID.equals(BLEUUID(BLE_CHARACT_CO2_UUID))) {
          pCharacteristic->setValue(valueCO2);
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onRead] - Sent CO2 value");
      }

      if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [onRead] - Ends");
    }
};

extern MyServerCallbacks* pMyServerCallbacks;
extern MyCallbacks* pMyCallbacks;
extern BLE2902* pBLE2902CO2;
extern BLE2902* pBLE2902T;
extern BLE2902* pBLE2902Hum;

uint32_t setupBLE();
uint32_t startBLEAdvertising();
void stopBLE(uint8_t caller=0);
