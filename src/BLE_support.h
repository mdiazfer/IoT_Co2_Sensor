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

/*void iBeaconStop();
void iBeaconInit();
uint32_t BLEPeripheralInit(bool fromSetup, bool debugModeOn, bool TFTDisplayLogs);
*/
uint32_t  sendiBeacon();