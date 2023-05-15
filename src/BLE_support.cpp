#include "BLE_support.h"

/* ****************************************************************************************************************

  BLE_support.c version 2.0

  This module deal with three tasks:
  - Manage BLE stuff
  - Fix heap leak constraints due to the big size of the BLE stack
  - Fix the error "Task watchdog got triggered" when serving HTTP Requests

  BLE stratgy 
  ===========

    BLE stack is really big and prevents from working with other modules, i.e. Webserver.

    To fix this issue, the BLE statck is inited (loaded) to advertise the beacon and then deinitiated (removed)
    to release memory and allow other modules to work with no heap leak.

    Functions:

    * setupBLE() - It initiates (BLEDevice::init()) the BLE stack (free 95000 B in memory required), creates all the 
      required elements at setup() time and then deinitiates (BLEDevice::deinit())the BLE stack to realease memory.
      The BLEDevice::deinit() functions only releases memory related to the esp_ble_* low level API (ESP ble framework),
      i.e. GATT table, etc. but all the BLE Classes-related stuff is kept in memory, which is far less than the 
      low level BLE api stack.

      Elements created and kept in memory:
        - BLEServer - Static
        - BLEAdvertising - Static
        - BLEService(s) - Dynamic
        - BLECharacteristic(s) - Dynamic
        - BLEDescriptor(s)  - Dynamic
        - BLEBeacon - Dynamic
        - BLEAdvertisementData - Dynamic
        - Callbacks (server and characteristic) - Dynamic

    * startAdvertising() - It recreates the GATT table (esp_ble_* low level API stuff) and then starts the Advertising.
      No more actions related to the BLE classes are required as they are kept in memory since setupBLE().
      This function is called in the BLE_PERIOD cycle from loop().

      It starts only if there is enough heap memory (at least 95 KB) to init the esp_ble_* low level API stack.

      It modifies the Watch Dog Timer (WDT) from 9000 milliseconds to 20000 milliseconds to avoid 
      the error "Task watchdog got triggered" when the ESPAsyncWebServer webServer needs to server an HTPP Request.
      It happens randomly especially when serving big files (i.e. The_IoT_Factory.png file), probably because the CPU
      time is shared between sending BLE Advertising and serving HTTP Request, which might take longer than the WDT.

    * stopBLE() - It stops the Advertisings and the releases memory (BLEDevice::deinit()). All the BLE classes related
      elements are kept in memory ready for the next cycle to Adverise beacons and servicies.
      This function is called in the BLE_ON_TIMEOUT cycle from loop().

      It also turns the WDT back to automatic with the timeout value (9000 milliseconds).

      In the BLE_ON_TIMEOUT cycle in the loop(), WDT is manually feed to avoid the error "Task watchdog got triggered".

    Required modifications in the BLE classes

    * BLEServer
      - void recreateGATTTable();
    * BLEService
      - void recreateGATTTable(BLEServer* pServer);
    * BLECharacteristic
      - void recreateGATTTable(BLEService* pService);
    * BLEDescriptor
      - void recreateGATTTable(BLECharacteristic* pCharacteristic);

  Alterantive strategy (version 1.0) 
  =================================

    In the BLE_ON_TIMEOUT cicle:
      - stop advertising
      - BLEDevice::deinit()
      - remove service: pServer->removeService();
      - remove all the objects and memory
        - BLEServer - Static
        - BLEAdvertising - Static
        - BLEService(s) - Dynamic
        - BLECharacteristic(s) - Dynamic
        - BLEDescriptor(s)  - Dynamic
        - BLEBeacon - Dynamic
        - BLEAdvertisementData - Dynamic
        - Callbacks (server and characteristic) - Dynamic
    
    In the BLE_PERIOD cycle:
      - create everything again

    Problem doing this is that is not 100% than everything is removed, so heap leak issue might occur.

   ****************************************************************************************************************** */


uint32_t setupBLE() {
  //BLEDevice::init() needed to create Server, Advertising, etc.
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - Init and heap="+String(esp_get_free_heap_size()));}
  
  BLEDevice::init(std::string(String("se"+device.substring(12)).c_str()));
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - BLEDevice initiated and heap="+String(esp_get_free_heap_size()));}
  if (pServer!=nullptr) {
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - BLEServer already created. Exit");}
    return ERROR_BLE_SETUP;
  }

  //Usually after power up or reset. Need to create BLEServer, BLEService and BLECharacteristics
  pServer=BLEDevice::createServer();
  if (pServer==nullptr) { 
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - Creating BLEServer failed");}
    return ERROR_BLE_SETUP;
  }

  pServer->setCallbacks(pMyServerCallbacks);

  // Create the BLE Service as it was removed when switched BLE off
  pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID));
  if (pService==nullptr) { 
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - pService is nullptr");}
    return ERROR_BLE_SETUP;
  }

  // Create a BLE Characteristics
  pCharacteristicCO2 = pService->createCharacteristic(
                    BLE_CHARACT_CO2_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    //BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pCharacteristicCO2->setCallbacks(pMyCallbacks);pCharacteristicCO2->addDescriptor(pBLE2902CO2);
  pCharacteristicT = pService->createCharacteristic(
                    BLE_CHARACT_TEM_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    //BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pCharacteristicT->setCallbacks(pMyCallbacks);pCharacteristicT->addDescriptor(pBLE2902T);
  pCharacteristicHum = pService->createCharacteristic(
                    BLE_CHARACT_HUM_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    //BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pCharacteristicHum->setCallbacks(pMyCallbacks);pCharacteristicHum->addDescriptor(pBLE2902Hum);
                
  pAdvertising = pServer->getAdvertising();
  if (pAdvertising==nullptr) { 
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - pAdvertising is nullptr");}
    return ERROR_BLE_SETUP;
  }

  //Add service to the Advertising
  pAdvertising->addServiceUUID(BLEUUID(BLE_SERVICE_UUID));

  // Start the service
  pService->start();
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setupBLE] - Service started");

  //Add Beacon to the Advertising - Every time due to BLEBeacon & BLEAdvertisementData object memory is realeasd
  pBeacon->setManufacturerId(BEACON_MANUFACTURER);
  //iBeacon Data
  byte mac[6];WiFi.macAddress(mac);
  uint16_t aux=(mac[2]<<8)|mac[3];
  pBeacon->setMajor(aux);
  aux=(mac[4]<<8)|mac[5];
  pBeacon->setMinor(aux);
  pBeacon->setSignalPower(BEACON_RSSI);
  pBeacon->setProximityUUID(BLEUUID(BEACON_UUID_REV));

  //Set Flags and Add Beacon to the Advertising
  pAdvertisementData->setFlags(0x1A);
  pAdvertisementData->setManufacturerData(pBeacon->getData());
  pAdvertising->setAdvertisementData(*pAdvertisementData);

  //pAdvertising->start();
  //if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [setupBLE] - Advertising started");
  
  //Release memory
  BLEDevice::deinit(false);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [setupBLE] - BLEDevice deinitiated and heap="+String(esp_get_free_heap_size())+". Exit");}

  return 0;
}

uint32_t startBLEAdvertising() {
  //FreeRTOS::Semaphore m_semaphoreCreateEvt 		= FreeRTOS::Semaphore("CreateEvt");
  unsigned int wdt_timeout=0;
  rtc_wdt_get_timeout(RTC_WDT_STAGE0, &wdt_timeout);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - Init and heap="+String(esp_get_free_heap_size())+", rtc_wdt_get_timeout="+String(wdt_timeout)+", rtc_wdt_get_protect_status="+String(rtc_wdt_get_protect_status())+", rtc_wdt_is_on="+String(rtc_wdt_is_on()));}
  //Configure manual WDT with longer timeout to avoid WDT Error Event when serving HTTP Request while Advertising BLE Services 
  rtc_wdt_protect_off();    // Turns off the automatic wdt service
  rtc_wdt_disable();         //Turns it off manually
  rtc_wdt_set_time(RTC_WDT_STAGE0, WD_TIMEOUT);  // Define how long (ms) you desire to let dog wait. 9000 by default
  rtc_wdt_enable();         // Turn it on manually
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - WDT manually configured with timeout="+String(WD_TIMEOUT)+" ms");}

  // Init BLEDevice
  BLEDevice::init(std::string(String("se"+device.substring(12)).c_str()));
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - BLEDevice initiated and heap="+String(esp_get_free_heap_size()));}
  
  if (pAdvertising==nullptr) { 
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - pAdvertising is nullptr");}
    BLEClurrentStatus=BLEOffStatus;
    return ERROR_BLE_SETUP;
  }

  //Recreate the GATT table with all the elments in memory (services, characteristics and descriptors) and start the services 
  pServer->recreateServerGATTTable();
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - GATT table recreated with all services, characteristics and descriptors");

  //Set Adverstisement Data as BLEDevice::deinit released memorry and then Start Advertising
  pAdvertising->setAdvertisementData(*pAdvertisementData);
  pAdvertising->start();
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - Advertising started");

  if (BLEDevice::getInitialized()) {
    rtc_wdt_get_timeout(RTC_WDT_STAGE0, &wdt_timeout);
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [startBLEAdvertising] - BLEDevice initiated and heap="+String(esp_get_free_heap_size())+", rtc_wdt_get_timeout="+String(wdt_timeout)+", rtc_wdt_get_protect_status="+String(rtc_wdt_get_protect_status())+", rtc_wdt_is_on="+String(rtc_wdt_is_on())+". Exit");}
    return 0;
  }
  else {
    BLEClurrentStatus=BLEOffStatus;
    return ERROR_BLE_SETUP;
  }
}

void stopBLE(uint8_t caller) {
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [stopBLE] - Init and caller="+String(caller)+" (0=loop)");}

  //Exit if BLE Advertising is off
  //if( !bluetoothEnabled || (bluetoothEnabled && !isBeaconAdvertising && !BLEDevice::getInitialized()) ) {
  if( !bluetoothEnabled || (bluetoothEnabled && !isBeaconAdvertising) ) {
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [stopBLE] - Either !isBeaconAdvertising ("+String(!isBeaconAdvertising)+") or !bluetoothEnabled ("+String(!bluetoothEnabled)+") or !BLEDevice::getInitialized("+String(!BLEDevice::getInitialized())+") - Nothing to stop. Exit");}
    return;
  }

  unsigned int wdt_timeout=0;
  rtc_wdt_get_timeout(RTC_WDT_STAGE0, &wdt_timeout);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [stopBLE] - Init and heap="+String(esp_get_free_heap_size())+", rtc_wdt_get_timeout="+String(wdt_timeout)+", rtc_wdt_get_protect_status="+String(rtc_wdt_get_protect_status())+", rtc_wdt_is_on="+String(rtc_wdt_is_on()));}
  
  //Overflow protection mechanism
  if (pServer==nullptr) return;
  
  //Stop Advertisings and release memory
  BLEDevice::stopAdvertising();
  BLEDevice::deinit(false);

  //Turn the WDT back to automatic with default timeout
  rtc_wdt_disable();         //Turns it off manually
  rtc_wdt_set_time(RTC_WDT_STAGE0, 9000);  //Define default dog wait.
  rtc_wdt_protect_on();  //Turns on the automatic wdt service again
  rtc_wdt_get_timeout(RTC_WDT_STAGE0, &wdt_timeout);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [stopBLE] - WDT turned back to automatic with timeout=9000 ms");}

  //Control variables update
  //nowTimeGlobal=loopStartTime+millis();
  //isBeaconAdvertising=false;
  //lastTimeBLEOnCheck=nowTimeGlobal;
  heapSizeNow=esp_get_free_heap_size();

  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  [stopBLE] - BLEDevice deinitiated and heap="+String(heapSizeNow)+" B, minHeap="+String(minHeapSeen)+" B, rtc_wdt_get_timeout="+String(wdt_timeout)+", rtc_wdt_get_protect_status="+String(rtc_wdt_get_protect_status())+", rtc_wdt_is_on="+String(rtc_wdt_is_on())+". Exit");}
}