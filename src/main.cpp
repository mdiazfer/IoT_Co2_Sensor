#ifndef WIFIFRAMEWORK
  #define WIFIFRAMEWORK
#endif

#ifndef _DECLAREGLOBALPARAMETERS_
  #define _DECLAREGLOBALPARAMETERS_
#endif

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <SoftwareSerial.h>
#include "time.h"
#include "esp_sntp.h"
#include <ESP32Time.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <AsyncMqttClient.h>
#include "global_setup.h"
#include "wifiConnection.h"
#include "display_support.h"
#include "MHZ19.h"
#include "Button.h"
#include "SHT2x.h"
#include "ButtonChecks.h"
#include "httpClient.h"
#include "icons.h"
#include "battery.h"
#include "misc.h"
#include "webServer.h"
#include "mqttClient.h"
#include "BLE_support.h"

#ifdef __MHZ19B__
  const ulong co2PreheatingTime=MH_Z19B_CO2_WARMING_TIME;
#endif

//Global variable definitions stored in RTC memory to be persistent during Sleep periods. 8 KB Max
// RTC memory Arrays:
//           3* (4*3600/60 = 240 B)     =  720 B
//           3* (4*24*3600/450 = 768 B) = 2304 B
// RTC memory variables in global_setup:    56 B
// RTC memory variables:                   1439 B
// ----------------------------------------------
// RTC memorty TOTAL:                     4519 B
// RTC memory left:     8000 B - 4519 B = 3481 B 
//
//EEPROM MAP
//Address 0-5: Stores the firmware version char []* (5B+null=6 B)
//Address 6-7: Stores checksum (2 B)
//Address 08: Stores Config Variable Valued (configVariables) (1 B)
//  - Bit 0: notFirstRun - 1=true, 0=false
//  - Bit 1: configSavingEnergyMode variable - 1=reduced, 0=lowest
//  - Bit 2: uploadSamplesEnabled - 1=true, 0=false
//  - Bit 3: bluetoothEnabled - 1=true, 0=false
//  - Bit 4: wifiEnabled - 1=true, 0=false
//  - Bit 5: webServerEnable - 1=true, 0=false
//  - Bit 6: mqttServerEnable - 1=true, 0=false
//  - Bit 7: secureMmqttEnable - 1=true, 0=false
//Address 09-0C: Stores float_t batCharge (4 B)
//Address 0D-2D:   SSID1 char []* (32 B+null=33 B)
//Address 2E-6D:   PSSW1 char []* (63 B+null=64 B)
//Address 6E-78:   SITE1 char []* (10 B+null=11 B)
//Address 79-99:   SSID2 char []* (32 B+null=33 B)
//Address 9A-D9:   PSSW2 char []* (63 B+null=64 B)
//Address DA-E4:   SITE2 char []* (10 B+null=11 B)
//Address E5-105:  SSID3 char []* (32 B+null=33 B)
//Address 106-145: PSSW3 char []* (63 B+null=64 B)
//Address 146-150: SITE3 char []* (10 B+null=11 B)
//Address 151-190: NTP1 char []* (63 B+null=64 B)
//Address 191-1D0: NTP2 char []* (63 B+null=64 B)
//Address 1D1-210: NTP3 char []* (63 B+null=64 B)
//Address 211-250: NTP4 char []* (63 B+null=64 B)
//Address 251-289: TZEnvVar char []* (56 B+null=57 B)
//Address 28A-2A7: TZName char []* (29 B+null=30 B)
//Address 2A8-2B2: Web User name char []* (10 B+null=11 B)
//Address 2B3-2BD: Web User passw char []* (10 B+null=11 B)
//Address 2BE: Stores Misc Variable Values (1 B)
//  - Bit 0: siteAllowToUploadSamples - 1=true, 0=false
//  - Bit 1: siteBk1AllowToUploadSamples - 1=true, 0=false
//  - Bit 2: siteBk2AllowToUploadSamples - 1=true, 0=false
//  - Bit 2: siteBk2AllowToUploadSamples - 1=true, 0=false
//Address 2BF-2FE: MQTT char []* (63 B+null=64 B)
//Address 2FF-309: MQTT User name char []* (10 B+null=11 B)
//Address 30A-314: MQTT User passw char []* (10 B+null=11 B)
//Address 315-3DD: MQTT Topic name char []* (200 B+null=201 B)
//Address 3DE: bootCount - Number of restarts since last upgrade
//Address 3DF: resetCount - Number of non controlled resets since last upgrade


RTC_DATA_ATTR float_t lastHourCo2Samples[3600/SAMPLE_T_LAST_HOUR];   //4*(3600/60)=240 B - Buffer to record last-hour C02 values
RTC_DATA_ATTR float_t lastHourTempSamples[3600/SAMPLE_T_LAST_HOUR];  //4*(3600/60)=240 B - Buffer to record last-hour Temp values
RTC_DATA_ATTR float_t lastHourHumSamples[3600/SAMPLE_T_LAST_HOUR];   //4*(3600/60)=240 B - Buffer to record last-hour Hum values
RTC_DATA_ATTR float_t lastDayCo2Samples[24*3600/SAMPLE_T_LAST_DAY];  //4*(24*3600/450)=768 B - Buffer to record last-day C02 values
RTC_DATA_ATTR float_t lastDayTempSamples[24*3600/SAMPLE_T_LAST_DAY]; //4*(24*3600/450)=768 B - Buffer to record last-day Temp values
RTC_DATA_ATTR float_t lastDayHumSamples[24*3600/SAMPLE_T_LAST_DAY];  //4*(24*3600/450)=768 B - Buffer to record last-day Hum values
RTC_DATA_ATTR boolean firstBoot=true;  //1B - First boot flag.
RTC_DATA_ATTR uint64_t nowTimeGlobal=0,timeUSBPowerGlobal=0,loopStartTime=0,loopEndTime=0,
                        lastTimeSampleCheck=0,previousLastTimeSampleCheck=0,
                        lastTimeBLECheck=0,previousLastTimeBLECheck=0,lastTimeBLEOnCheck=0,
                        lastTimeDisplayCheck=0,lastTimeDisplayModeCheck=0,lastTimeNTPCheck=0,lastTimeVOLTCheck=0,
                        lastTimeHourSampleCheck=0,lastTimeDaySampleCheck=0,lastTimeUploadSampleCheck=0,lastTimeIconStatusRefreshCheck=0,
                        lastTimeTurnOffBacklightCheck=0,lastTimeWifiReconnectionCheck=0; //16*8=128 B
RTC_DATA_ATTR ulong voltageCheckPeriod,samplePeriod,uploadSamplesPeriod,BLEPeriod,BLEOnTimeout; //4*4=16B
RTC_DATA_ATTR uint64_t sleepTimer=0; //8 B
RTC_DATA_ATTR enum displayModes displayMode=bootup,lastDisplayMode=bootup; //2*4=8 B
RTC_DATA_ATTR enum availableStates stateSelected=displayingSampleFixed,currentState=bootupScreen,lastState=currentState; //3*4=12 B
RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus; //4 B
RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus; // 4B
RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus; // 4B
RTC_DATA_ATTR boolean updateHourSample=true,updateDaySample=true,updateHourGraph=true,updateDayGraph=true,
              autoBackLightOff=true,button1Pressed=false,button2Pressed=false,reconnectWifiAndRestartWebServer=false,
              resyncNTPServer=false,deviceReset=false,factoryReset=false; //12*1=12 B 
RTC_DATA_ATTR enum powerModes powerState=off; //1*4=4 B
RTC_DATA_ATTR enum batteryChargingStatus batteryStatus=battery000; //1*4=4 B
RTC_DATA_ATTR enum energyModes energyCurrentMode,configSavingEnergyMode; //2*4=8 B
RTC_DATA_ATTR uint8_t bootCount=0,resetCount=0,loopCount=0; //3*1=3 B
RTC_DATA_ATTR const String co2SensorType=String(CO2_SENSOR_TYPE); //16 B
RTC_DATA_ATTR const String tempHumSensorType=String(TEMP_HUM_SENSOR_TYPE); //16 B
RTC_DATA_ATTR char co2SensorVersion[5]; //5 B
RTC_DATA_ATTR CircularGauge circularGauge=CircularGauge(0,0,CO2_GAUGE_RANGE,CO2_GAUGE_X,CO2_GAUGE_Y,CO2_GAUGE_R,
                                          CO2_GAUGE_WIDTH,CO2_GAUGE_SECTOR,TFT_DARKGREEN,
                                          CO2_GAUGE_TH1,TFT_YELLOW,CO2_GAUGE_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK); //212 B
RTC_DATA_ATTR HorizontalBar horizontalBar=HorizontalBar(0,TEMP_BAR_MIN,TEMP_BAR_MAX,TEMP_BAR_X,TEMP_BAR_Y,
                                          TEMP_BAR_LENGTH,TEMP_BAR_HEIGH,TFT_GREEN,TEMP_BAR_TH1,
                                          TFT_BLUE,TEMP_BAR_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);  //92 B
RTC_DATA_ATTR Button  button1(BUTTON1); //16 B
RTC_DATA_ATTR Button  button2(BUTTON2); //16 B
//Since v0.9.9, TZEnvVar might be removed (with a bit re-coding) as TZEnvVariable is got from EEPROM (permanent). It's kept to avoid re-coding
RTC_DATA_ATTR char TZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH]="\0"; //57 B Should be enough - To back Time Zone Variable up
RTC_DATA_ATTR struct tm startTimeInfo; //36 B
RTC_DATA_ATTR boolean firstWifiCheck=true,forceWifiReconnect=false,forceGetSample=false,forceGetVolt=false,
        forceDisplayRefresh=false,forceDisplayModeRefresh=false,forceNTPCheck=false,buttonWakeUp=false,
        forceWEBCheck=false,forceWEBTestCheck=false,forceWebServerInit=false; // 12*1=12 B
RTC_DATA_ATTR int uploadServerIPAddressOctectArray[4]; // 4*4B = 16B - To store upload server's @IP
RTC_DATA_ATTR byte mac[6]; //6*1=6B - To store WiFi MAC address
RTC_DATA_ATTR float_t valueCO2,valueT,valueHum=0,lastValueCO2=-1,tempMeasure; //5*4=20B
RTC_DATA_ATTR int errorsWiFiCnt=0,errorsSampleUpts=0,errorsNTPCnt=0,webServerError1=0,
                  webServerError2=0,webServerError3=0,SPIFFSErrors=0; //7*4=28B - Error stats
RTC_DATA_ATTR boolean wifiEnabled,bluetoothEnabled,uploadSamplesEnabled,webServerEnabled,mqttServerEnabled,secureMqttEnabled;//5*1=5B
RTC_DATA_ATTR boolean debugModeOn=DEBUG_MODE_ON,startTimeConfigure=false; //2*1=1B
RTC_DATA_ATTR enum BLEStatus BLEClurrentStatus=BLEOffStatus; //1*4=4B
RTC_DATA_ATTR AsyncWebServer webServer(WEBSERVER_PORT); //1*84=84B
RTC_DATA_ATTR AsyncEventSource webEvents(WEBSERVER_SAMPLES_EVENT); //1*104=104B
RTC_DATA_ATTR uint32_t error_setup=NO_ERROR,minHeapSeen=0xFFFFFFFF; //1*4=4B
RTC_DATA_ATTR AsyncMqttClient mqttClient;
RTC_DATA_ATTR boolean OTAUpgradeBinAllowed=false,SPIFFSUpgradeBinAllowed=false; //2*1=2B - v1.2.0 To block SPIFFS upgrade if there is something wrong with SPIFFS partition

//Global variable definitions stored in regular RAM. 520 KB Max
TFT_eSPI tft = TFT_eSPI();  // 292 B - Invoke library to manage the display
#ifdef __MHZ19B__
  MHZ19 co2Sensor;  //64 B
  extern const int MHZ19B;
#endif
SoftwareSerial co2SensorSerialPort(CO2_SENSOR_RX, CO2_SENSOR_TX); //136 B
#ifdef __SI7021__
  SHT2x tempHumSensor; //36 B
#endif
uint32_t remainingBootupSeconds=0;
int16_t cuX,cuY;
TFT_eSprite stext1 = TFT_eSprite(&tft); // Sprite object stext1
ulong previousTime=0,timePressButton1,timeReleaseButton1,timePressButton2,timeReleaseButton2,
      remainingBootupTime=BOOTUP_TIMEOUT*1000;
String valueString,TZEnvVariable,TZName;
String serverToUploadSamplesString(SERVER_UPLOAD_SAMPLES);
IPAddress serverToUploadSamplesIPAddress; //8 B
String device(DEVICE_NAME_PREFIX); //16 B
static const char hex_digits[] = "0123456789ABCDEF";
boolean waitingMessage=true,runningMessage=true,wifiResuming=false,NTPResuming=false,webResuming=false,startAPMode=false;
uint8_t pixelsPerLine,
    spL,            //Number of Lines in the Sprite
    scL,            //Number of Lines in the Scroll
    pFL,            //Pointer First Line
    pLL,            //pointer Last Line written
    spFL,           //Sprite First Line Window
    spLL,           //Sprite Last Line Window
    scFL,           //Scroll First Line Window
    scLL;           //Scroll Last Line Window
uint8_t auxLoopCounter=0,auxLoopCounter2=0,auxCounter=0;
uint64_t whileLoopTimeLeft=NTP_CHECK_TIMEOUT,whileWebLoopTimeLeft=HTTP_ANSWER_TIMEOUT;
uint8_t configVariables;
char firmwareVersion[VERSION_CHAR_LENGTH+1];
esp_sleep_wakeup_cause_t wakeup_reason;
uint8_t fileUpdateError=0,errorOnActiveCookie=0,errorOnWrongCookie=0;
size_t fileUpdateSize=0;
int updateCommand=-1;
uint32_t flashSize = ESP.getFlashChipSize();
uint32_t programSize = ESP.getSketchSize();
size_t OTAAvailableSize,SPIFFSAvailableSize;
uint32_t fileSystemSize=0;
uint32_t fileSystemUsed=0;
String fileUpdateName;
struct tm nowTimeInfo; //36 B
char activeCookie[COOKIE_SIZE];
char currentSetCookie[COOKIE_SIZE];
JSONVar samples;
String userName,userPssw,mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName;
uint32_t heapSizeNow=0;//,heapSizeLast=0;
bool webServerResponding=false,isBeaconAdvertising=false;
bool deviceConnected = false;
BLEServer* pServer=nullptr;
BLEAdvertising* pAdvertising=nullptr;
MyServerCallbacks* pMyServerCallbacks=new MyServerCallbacks();
MyCallbacks* pMyCallbacks=new MyCallbacks();
BLE2902* pBLE2902CO2=new BLE2902();
BLE2902* pBLE2902T=new BLE2902();
BLE2902* pBLE2902Hum=new BLE2902();
BLEBeacon* pBeacon=new BLEBeacon();
BLEAdvertisementData* pAdvertisementData=new BLEAdvertisementData();
BLEService* pService=nullptr;
BLECharacteristic* pCharacteristicCO2=nullptr;
BLECharacteristic* pCharacteristicT=nullptr;
BLECharacteristic* pCharacteristicHum=nullptr;

//Code

void initVariable() {
  //Global variables init. Needed as they have random values after wakeup from hiberte mode
  firstBoot=true;
  nowTimeGlobal=0;timeUSBPowerGlobal=0;loopStartTime=0;loopEndTime=0;
  lastTimeSampleCheck=0;previousLastTimeSampleCheck=0;lastTimeDisplayCheck=0;lastTimeDisplayModeCheck=0;
  lastTimeBLECheck=0;previousLastTimeBLECheck=0;lastTimeBLEOnCheck=0;
  lastTimeNTPCheck=0;lastTimeVOLTCheck=0;lastTimeHourSampleCheck=0;lastTimeDaySampleCheck=0;
  lastTimeUploadSampleCheck=0;lastTimeIconStatusRefreshCheck=0;lastTimeTurnOffBacklightCheck=0;
  lastTimeWifiReconnectionCheck=0;
  sleepTimer=0;
  displayMode=bootup;lastDisplayMode=bootup;
  stateSelected=displayingSampleFixed;currentState=bootupScreen;lastState=currentState;
  updateHourSample=true;updateDaySample=true;updateHourGraph=true;updateDayGraph=true;
  autoBackLightOff=true;button1Pressed=false;button2Pressed=false;
  powerState=off;
  batteryStatus=battery000;
  loopCount=0;
  circularGauge=CircularGauge(0,0,CO2_GAUGE_RANGE,CO2_GAUGE_X,CO2_GAUGE_Y,CO2_GAUGE_R,
                            CO2_GAUGE_WIDTH,CO2_GAUGE_SECTOR,TFT_DARKGREEN,
                            CO2_GAUGE_TH1,TFT_YELLOW,CO2_GAUGE_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);
  horizontalBar=HorizontalBar(0,TEMP_BAR_MIN,TEMP_BAR_MAX,TEMP_BAR_X,TEMP_BAR_Y,
                              TEMP_BAR_LENGTH,TEMP_BAR_HEIGH,TFT_GREEN,TEMP_BAR_TH1,
                              TFT_BLUE,TEMP_BAR_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);
  firstWifiCheck=true;forceWifiReconnect=false;forceGetSample=false;forceGetVolt=false;
  forceDisplayRefresh=false;forceDisplayModeRefresh=false;forceNTPCheck=false;buttonWakeUp=false;
  forceWEBCheck=false;forceWEBTestCheck=false;forceWebServerInit=false;
  valueCO2=0;valueT=0;valueHum=0;lastValueCO2=-1;tempMeasure=0;
  errorsWiFiCnt=0;errorsSampleUpts=0;errorsNTPCnt=0;webServerError1=0;webServerError2=0;webServerError3=0;SPIFFSErrors=0;
  error_setup=NO_ERROR;remainingBootupSeconds=0;
  previousTime=0;remainingBootupTime=BOOTUP_TIMEOUT*1000;
  static const char hex_digits[] = "0123456789ABCDEF";
  waitingMessage=true;runningMessage=true;
  auxLoopCounter=0;auxLoopCounter2=0;auxCounter=0;
  whileLoopTimeLeft=NTP_CHECK_TIMEOUT;whileWebLoopTimeLeft=HTTP_ANSWER_TIMEOUT;
  wifiResuming=false;NTPResuming=false;webResuming=false;
  debugModeOn=DEBUG_MODE_ON;
  BLEClurrentStatus=BLEOffStatus;
  reconnectWifiAndRestartWebServer=false;
  resyncNTPServer=false;
  deviceReset=false;
  factoryReset=false;
  error_setup=NO_ERROR;
  startAPMode=false;
  //webServer=AsyncWebServer(WEBSERVER_PORT);
  fileUpdateError=0;errorOnActiveCookie=0;errorOnWrongCookie=0;
  fileUpdateSize=0;
  updateCommand=-1;
  flashSize = ESP.getFlashChipSize();
  programSize = ESP.getSketchSize();
  OTAAvailableSize=getAppOTAPartitionSize(ESP_PARTITION_TYPE_APP,ESP_PARTITION_SUBTYPE_ANY);
  SPIFFSAvailableSize=getAppOTAPartitionSize(ESP_PARTITION_TYPE_DATA,0x82);
  memset(activeCookie,'\0',COOKIE_SIZE); //init variable
  memset(currentSetCookie,'\0',COOKIE_SIZE); //init variable
  //BLE variables and pointers
  webServerResponding=false;isBeaconAdvertising=false;
  deviceConnected = false;
  
  //Read from EEPROM the values to be stored in the Config Variables
  //
  //EEPROM MAP
  //Address 0-5: Stores the firmware version char []*
  //Address 6-7: Stores checksum
  //Address 08: Stores Config Variable Valued (configVariables)
  //  - Bit 0: notFirstRun - 1=true, 0=false
  //  - Bit 1: configSavingEnergyMode variable - 1=reduced, 0=lowest
  //  - Bit 2: uploadSamplesEnabled - 1=true, 0=false
  //  - Bit 3: bluetoothEnabled - 1=true, 0=false
  //  - Bit 4: wifiEnabled - 1=true, 0=false
  //  - Bit 5: webServerEnable - 1=true, 0=false
  //  - Bit 6: mqttServerEnable - 1=true, 0=false
  //  - Bit 7: secureMmqttEnable - 1=true, 0=false
  //Address 09-0C: Stores float_t batCharge (4 B)
  //Address 0D-2D:   SSID1 char []* (32 B+null=33 B)
  //Address 2E-6D:   PSSW1 char []* (63 B+null=64 B)
  //Address 6E-78:   SITE1 char []* (10 B+null=11 B)
  //Address 79-99:   SSID2 char []* (32 B+null=33 B)
  //Address 9A-D9:   PSSW2 char []* (63 B+null=64 B)
  //Address DA-E4:   SITE2 char []* (10 B+null=11 B)
  //Address E5-105:  SSID3 char []* (32 B+null=33 B)
  //Address 106-145: PSSW3 char []* (63 B+null=64 B)
  //Address 146-150: SITE3 char []* (10 B+null=11 B)
  //Address 151-190: NTP1 char []* (63 B+null=64 B)
  //Address 191-1D0: NTP2 char []* (63 B+null=64 B)
  //Address 1D1-210: NTP3 char []* (63 B+null=64 B)
  //Address 211-250: NTP4 char []* (63 B+null=64 B)
  //Address 251-289: TZEnvVar char []* (56 B+null=57 B)
  //Address 28A-2A7: TZName char []* (29 B+null=30 B)
  //Address 2A8-2B2: User name char []* (10 B+null=11 B)
  //Address 2B3-2BD: User passw char []* (10 B+null=11 B)
  //Address 2BE: Stores Misc Variable Values (1 B)
  //  - Bit 0: siteAllowToUploadSamples - 1=true, 0=false
  //  - Bit 1: siteBk1AllowToUploadSamples - 1=true, 0=false
  //  - Bit 2: siteBk2AllowToUploadSamples - 1=true, 0=false
  //Address 2BF-2FE: MQTT char []* (63 B+null=64 B)
  //Address 2FF-309: MQTT User name char []* (10 B+null=11 B)
  //Address 30A-314: MQTT User passw char []* (10 B+null=11 B)
  //Address 315-3DD: MQTT Topic name char []* (200 B+null=201 B)
  //Address 3DE: bootCount - Number of restarts since last upgrade
  //Address 3DF: resetCount - Number of non controlled resets since last upgrade

  //Adding the 3 latest mac bytes to the device name (in Hex format)
  WiFi.macAddress(mac);
  device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
    String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
    String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);
  
  //Check if it is the first run after the very first firmware upload
  for (int i=0; i<VERSION_CHAR_LENGTH; i++) firmwareVersion[i]=EEPROM.read(i);firmwareVersion[VERSION_CHAR_LENGTH]='\0';
  uint16_t readChecksum,computedChecksum;
  readChecksum=EEPROM.read(7);readChecksum=readChecksum<<8;readChecksum|=EEPROM.read(6);
  computedChecksum=checkSum((byte*)firmwareVersion,VERSION_CHAR_LENGTH);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [initVariable] - firmwareVersion="+String(firmwareVersion)+", readChecksums="+String(readChecksum)+", computedChecksum="+String(computedChecksum));}
  
  if (readChecksum!=computedChecksum) {
  //if (true) { //<-- To force writing variables in EEPPROM during tests
    //It's the first run after the very first firmware upload
    //Variable inizialization to values configured in global_setup

    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [initVariable] - first run");}

    //Save version and checksum
    byte auxBuf[]=VERSION;
    computedChecksum=checkSum(auxBuf,VERSION_CHAR_LENGTH);
    for (int i=0; i<VERSION_CHAR_LENGTH; i++) EEPROM.write(i,auxBuf[i]);EEPROM.write(VERSION_CHAR_LENGTH,'\0');
    EEPROM.write(6,(byte) computedChecksum);
    computedChecksum=computedChecksum>>8;
    EEPROM.write(7,(byte) computedChecksum);
    
    //Writing default values in EEPROM
    factoryConfReset();
    
    EEPROM.commit();
  }
  else {
    //Not the very first run after firmware upgrade.
    
    //Update the firmware version in EEPROM if needed
    if (String(firmwareVersion).compareTo(String(VERSION))!=0) {
      byte auxBuf[]=VERSION;
      computedChecksum=checkSum(auxBuf,VERSION_CHAR_LENGTH);
      for (int i=0; i<VERSION_CHAR_LENGTH; i++) EEPROM.write(i,auxBuf[i]);EEPROM.write(VERSION_CHAR_LENGTH,'\0');
      EEPROM.write(6,(byte) computedChecksum);
      computedChecksum=computedChecksum>>8;
      EEPROM.write(7,(byte) computedChecksum);
      EEPROM.commit();
    }
    
    //Get the rest of variables from EEPROM
    configVariables=EEPROM.read(0x08);
    configSavingEnergyMode=configVariables & 0x02?reducedEnergy:lowestEnergy;
    uploadSamplesEnabled=configVariables & 0x04;
    bluetoothEnabled=configVariables & 0x08;
    wifiEnabled=configVariables & 0x10;
    webServerEnabled=configVariables & 0x20;
    mqttServerEnabled=configVariables & 0x40;
    secureMqttEnabled=configVariables & 0x80;

    //Get the WiFi Credential-related variables from EEPROM or global_setup.h
    //If varialbes exist in global_setup.h and doesn't exit in EEPPROM, then update EEPROM 
    char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
         auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
    bool updateEEPROM=false;
  
    memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0x0D,auxSSID);
    if (String(auxSSID).compareTo("")==0) {
      #ifdef WIFI_SSID_CREDENTIALS
        wifiCred.wifiSSIDs[0]=WIFI_SSID_CREDENTIALS;  
        //Check if SSID must be updated in EEPROM
        if (wifiCred.wifiSSIDs[0].compareTo(String(auxSSID))!=0) {
          uint8_t auxLength=wifiCred.wifiSSIDs[0].length()+1;
          if (auxLength>WIFI_MAX_SSID_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_SSID_LENGTH-1;
            wifiCred.wifiSSIDs[0]=wifiCred.wifiSSIDs[0].substring(0,auxLength);
          }
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,wifiCred.wifiSSIDs[0].c_str(),auxLength);
          EEPROM.put(0x0D,auxSSID);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiSSIDs[0]=auxSSID;
      #endif
    } else wifiCred.wifiSSIDs[0]=auxSSID;
    memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x2E,auxPSSW);
    if (String(auxPSSW).compareTo("")==0) {
      #ifdef WIFI_PW_CREDENTIALS
        wifiCred.wifiPSSWs[0]=WIFI_PW_CREDENTIALS;
        //Check if PSSW must be updated in EEPROM
        if (wifiCred.wifiPSSWs[0].compareTo(String(auxPSSW))!=0) {
          uint8_t auxLength=wifiCred.wifiPSSWs[0].length()+1;
          if (auxLength>WIFI_MAX_PSSW_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_PSSW_LENGTH-1;
            wifiCred.wifiPSSWs[0]=wifiCred.wifiPSSWs[0].substring(0,auxLength);
          }
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,wifiCred.wifiPSSWs[0].c_str(),auxLength);
          EEPROM.put(0x2E,auxPSSW);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiPSSWs[0]=auxPSSW;
      #endif
    } else wifiCred.wifiPSSWs[0]=auxPSSW;
    memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0x6E,auxSITE);
    if (String(auxSITE).compareTo("")==0) { 
      #ifdef WIFI_SITE
        wifiCred.wifiSITEs[0]=WIFI_SITE;
        //Check if SITE must be updated in EEPROM
        if (wifiCred.wifiSITEs[0].compareTo(String(auxSITE))!=0) { 
          uint8_t auxLength=wifiCred.wifiSITEs[0].length()+1;
          if (auxLength>WIFI_MAX_SITE_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_SITE_LENGTH-1;
            wifiCred.wifiSITEs[0]=wifiCred.wifiSITEs[0].substring(0,auxLength);
          }
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,wifiCred.wifiSITEs[0].c_str(),auxLength);
          EEPROM.put(0x6E,auxSITE);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiSITEs[0]=auxSITE;
      #endif
    } else wifiCred.wifiSITEs[0]=auxSITE;
    memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0x79,auxSSID);
    if (String(auxSSID).compareTo("")==0) { 
      #ifdef WIFI_SSID_CREDENTIALS_BK1
        wifiCred.wifiSSIDs[1]=WIFI_SSID_CREDENTIALS_BK1;
        //Check if SSID_BK1 must be updated in EEPROM
        if (wifiCred.wifiSSIDs[1].compareTo(String(auxSSID))!=0) {
          uint8_t auxLength=wifiCred.wifiSSIDs[1].length()+1;
          if (auxLength>WIFI_MAX_SSID_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_SSID_LENGTH-1;
            wifiCred.wifiSSIDs[1]=wifiCred.wifiSSIDs[1].substring(0,auxLength);
          }
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,wifiCred.wifiSSIDs[1].c_str(),auxLength);
          EEPROM.put(0x79,auxSSID);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiSSIDs[1]=auxSSID;
      #endif
    } else wifiCred.wifiSSIDs[1]=auxSSID;
    memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x9A,auxPSSW);
    if (String(auxPSSW).compareTo("")==0) { 
      #ifdef WIFI_PW_CREDENTIALS_BK1
        wifiCred.wifiPSSWs[1]=WIFI_PW_CREDENTIALS_BK1;
        //Check if PSSW_BK1 must be updated in EEPROM
        if (wifiCred.wifiPSSWs[1].compareTo(String(auxPSSW))!=0) {
          uint8_t auxLength=wifiCred.wifiPSSWs[1].length()+1;
          if (auxLength>WIFI_MAX_PSSW_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_PSSW_LENGTH-1;
            wifiCred.wifiPSSWs[1]=wifiCred.wifiPSSWs[1].substring(0,auxLength);
          }
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,wifiCred.wifiPSSWs[1].c_str(),auxLength);
          EEPROM.put(0x9A,auxPSSW);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiPSSWs[1]=auxPSSW;
      #endif
    } else wifiCred.wifiPSSWs[1]=auxPSSW;
    memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0xDA,auxSITE);
    if (String(auxSITE).compareTo("")==0) { 
      #ifdef WIFI_SITE_BK1
        wifiCred.wifiSITEs[1]=WIFI_SITE_BK1;
        //Check if SITE_BK1 must be updated in EEPROM
        if (wifiCred.wifiSITEs[1].compareTo(String(auxSITE))!=0) { //Substring if greater that max length
          uint8_t auxLength=wifiCred.wifiSITEs[1].length()+1;
          if (auxLength>WIFI_MAX_SITE_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_SITE_LENGTH-1;
            wifiCred.wifiSITEs[1]=wifiCred.wifiSITEs[1].substring(0,auxLength);
          }
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,wifiCred.wifiSITEs[1].c_str(),auxLength);
          EEPROM.put(0xDA,auxSITE);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiSITEs[1]=auxSITE;
      #endif
    } else wifiCred.wifiSITEs[1]=auxSITE;
    memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0xE5,auxSSID);
    if (String(auxSSID).compareTo("")==0) { 
      #ifdef WIFI_SSID_CREDENTIALS_BK2
        wifiCred.wifiSSIDs[2]=WIFI_SSID_CREDENTIALS_BK2;
        //Check if SSID_BK2 must be updated in EEPROM
        if (wifiCred.wifiSSIDs[2].compareTo(String(auxSSID))!=0) {
          uint8_t auxLength=wifiCred.wifiSSIDs[2].length()+1;
          if (auxLength>WIFI_MAX_SSID_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_SSID_LENGTH-1;
            wifiCred.wifiSSIDs[2]=wifiCred.wifiSSIDs[2].substring(0,auxLength);
          }
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,wifiCred.wifiSSIDs[2].c_str(),auxLength);
          EEPROM.put(0xE5,auxSSID);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiSSIDs[2]=auxSSID;
      #endif
    } else wifiCred.wifiSSIDs[2]=auxSSID;
    memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x106,auxPSSW);
    if (String(auxPSSW).compareTo("")==0) { 
      #ifdef WIFI_PW_CREDENTIALS_BK2
        wifiCred.wifiPSSWs[2]=WIFI_PW_CREDENTIALS_BK2;
        //Check if PSSW_BK2 must be updated in EEPROM
        if (wifiCred.wifiPSSWs[2].compareTo(String(auxPSSW))!=0) {
          uint8_t auxLength=wifiCred.wifiPSSWs[2].length()+1;
          if (auxLength>WIFI_MAX_PSSW_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_PSSW_LENGTH-1;
            wifiCred.wifiPSSWs[2]=wifiCred.wifiPSSWs[2].substring(0,auxLength);
          }
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,wifiCred.wifiPSSWs[2].c_str(),auxLength);
          EEPROM.put(0x106,auxPSSW);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiPSSWs[2]=auxPSSW;
      #endif
    } else wifiCred.wifiPSSWs[2]=auxPSSW;
    memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0x146,auxSITE);
    if (String(auxSITE).compareTo("")==0) { 
      #ifdef WIFI_SITE_BK2
        wifiCred.wifiSITEs[2]=WIFI_SITE_BK2;
        //Check if SITE_BK2 must be updated in EEPROM
        if (wifiCred.wifiSITEs[2].compareTo(String(auxSITE))!=0) {
          uint8_t auxLength=wifiCred.wifiSITEs[2].length()+1;
          if (auxLength>WIFI_MAX_SITE_LENGTH-1) { //Substring if greater that max length
            auxLength=WIFI_MAX_SITE_LENGTH-1;
            wifiCred.wifiSITEs[2]=wifiCred.wifiSITEs[2].substring(0,auxLength);
          }
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,wifiCred.wifiSITEs[2].c_str(),auxLength);
          EEPROM.put(0x146,auxSITE);
          updateEEPROM=true;
        }
      #else
        wifiCred.wifiSITEs[2]=auxSITE;
      #endif
    } else wifiCred.wifiSITEs[2]=auxSITE;

    memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x151,auxNTP);
    if (String(auxNTP).compareTo("")==0) { 
      #ifdef NTP_SERVER
        ntpServers[0]=NTP_SERVER;  
        //Check if NTP must be updated in EEPROM
        if (ntpServers[0].compareTo(String(auxNTP))!=0) {
          uint8_t auxLength=ntpServers[0].length()+1;
          if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
            auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
            ntpServers[0]=ntpServers[0].substring(0,auxLength);
          }
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,ntpServers[0].c_str(),auxLength);
          EEPROM.put(0x151,auxNTP);
          updateEEPROM=true;
        }
      #else
        ntpServers[0]="time.apple.com"; //Always should be one NTP server
      #endif
    } else ntpServers[0]=auxNTP;

    memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x191,auxNTP);
    if (String(auxNTP).compareTo("")==0) { 
      #ifdef NTP_SERVER2
        ntpServers[0]=NTP_SERVER2;  
        //Check if NTP must be updated in EEPROM
        if (ntpServers[1].compareTo(String(auxNTP))!=0) {
          uint8_t auxLength=ntpServers[1].length()+1;
          if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
            auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
            ntpServers[1]=ntpServers[1].substring(0,auxLength);
          }
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,ntpServers[1].c_str(),auxLength);
          EEPROM.put(0x191,auxNTP);
          updateEEPROM=true;
        }
      #else
        ntpServers[1]=auxNTP;
      #endif
    } else ntpServers[1]=auxNTP;

    memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x1D1,auxNTP);
    if (String(auxNTP).compareTo("")==0) { 
      #ifdef NTP_SERVER3
        ntpServers[0]=NTP_SERVER3;  
        //Check if NTP must be updated in EEPROM
        if (ntpServers[2].compareTo(String(auxNTP))!=0) {
          uint8_t auxLength=ntpServers[2].length()+1;
          if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
            auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
            ntpServers[2]=ntpServers[2].substring(0,auxLength);
          }
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,ntpServers[2].c_str(),auxLength);
          EEPROM.put(0x1D1,auxNTP);
          updateEEPROM=true;
        }
      #else
        ntpServers[2]=auxNTP;
      #endif
    } else ntpServers[2]=auxNTP;

    memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x211,auxNTP);
    if (String(auxNTP).compareTo("")==0) {
      #ifdef NTP_SERVER4
        ntpServers[0]=NTP_SERVER4;  
        //Check if NTP must be updated in EEPROM
        if (ntpServers[3].compareTo(String(auxNTP))!=0) {
          uint8_t auxLength=ntpServers[3].length()+1;
          if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
            auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
            ntpServers[3]=ntpServers[3].substring(0,auxLength);
          }
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,ntpServers[3].c_str(),auxLength);
          EEPROM.put(0x211,auxNTP);
          updateEEPROM=true;
        }
      #else
        ntpServers[3]=auxNTP;
      #endif
    }
    else ntpServers[3]=auxNTP;

    //Update volatile TZEnvVariable & TZName. Same function is called after waking up from sleep if pushed button1
    updateEEPROM|=initTZVariables();

    //Get the WEB User Credential-related variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
    char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
    memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2A8,auxUserName);
    if (String(auxUserName).compareTo("")==0) {
      userName=WEB_USER_CREDENTIAL;  
      EEPROM.put(0x2A8,auxUserName);
      updateEEPROM|=true;
    }
    else {
      userName=String(auxUserName);
    }
    memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);EEPROM.get(0x2B3,auxUserPssw);
    if (String(auxUserPssw).compareTo("")==0) {
      userPssw=WEB_PW_CREDENTIAL;  
      EEPROM.put(0x2B3,auxUserPssw);
      updateEEPROM|=true;
    }
    else {
      userPssw=String(auxUserPssw);
    }

    //Get the rest of wifiCred.SiteAllow variables from EEPROM
    configVariables=EEPROM.read(0x2BE);
    wifiCred.SiteAllow[0]=configVariables & 0x01;
    wifiCred.SiteAllow[1]=configVariables & 0x02;
    wifiCred.SiteAllow[2]=configVariables & 0x04;

    //Get the MQTT Server variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
    char auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH];
  
    memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x2BF,auxMQTT);
    if (String(auxMQTT).compareTo("")==0) {
      #ifdef MQTT_SERVER
        mqttServer=MQTT_SERVER;  
        //Check if MQTT server must be updated in EEPROM
        if (mqttServer.compareTo(String(auxMQTT))!=0) {
          uint8_t auxLength=mqttServer.length()+1;
          if (auxLength>MQTT_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
            auxLength=MQTT_SERVER_NAME_MAX_LENGTH-1;
            mqttServer=mqttServer.substring(0,auxLength);
          }
          memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);
          memcpy(auxMQTT,mqttServer.c_str(),auxLength);
          EEPROM.put(0x2BF,auxMQTT);
          updateEEPROM=true;
        }
      #else
        mqttServer=auxMQTT;
      #endif
    } else mqttServer=auxMQTT;

    //Get the MQTT User Credential-related variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
    memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2FF,auxUserName);
    if (String(auxUserName).compareTo("")==0) {
      mqttUserName=MQTT_USER_CREDENTIAL;  
      EEPROM.put(0x2FF,auxUserName);
      updateEEPROM|=true;
    }
    else {
      mqttUserName=String(auxUserName);
    }
    memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);EEPROM.get(0x30A,auxUserPssw);
    if (String(auxUserPssw).compareTo("")==0) {
      mqttUserPssw=MQTT_PW_CREDENTIAL;  
      EEPROM.put(0x30A,auxUserPssw);
      updateEEPROM|=true;
    }
    else {
      mqttUserPssw=String(auxUserPssw);
    }

    //Get the MQTT Topic Name variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM
    char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
    memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);EEPROM.get(0x315,auxMqttTopicPrefix);
    if (String(auxMqttTopicPrefix).compareTo("")==0) {
      #ifdef MQTT_TOPIC_PREFIX
        mqttTopicPrefix=MQTT_SERVER;
        if (mqttTopicPrefix.charAt(mqttTopicPrefix.length()-1)!='/') mqttTopicPrefix+="/"; //Adding slash at the end if needed
        //Check if MQTT topic name must be updated in EEPROM
        if (mqttTopicPrefix.compareTo(String(auxMqttTopicPrefix))!=0) {
          uint8_t auxLength=mqttTopicPrefix.length()+1;
          if (auxLength>MQTT_TOPIC_NAME_MAX_LENGTH-1) { //Substring if greater that max length
            auxLength=MQTT_TOPIC_NAME_MAX_LENGTH-1;
            mqttTopicPrefix=mqttTopicPrefix.substring(0,auxLength);
          }
          memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
          memcpy(auxMqttTopicPrefix,mqttTopicPrefix.c_str(),auxLength);
          EEPROM.put(0x315,auxMqttTopicPrefix);
          updateEEPROM=true;
        }
      #else
        mqttTopicPrefix=auxMqttTopicPrefix;
      #endif
    } else mqttTopicPrefix=auxMqttTopicPrefix;
    mqttTopicName=mqttTopicPrefix+device; //Adding the device name to the MQTT Topic name

    //Set MQTT variables and functions first
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onPublish(onMqttPublish);

    //Set the bootCount from EEPROM
    bootCount=EEPROM.read(0x3DE);

    //Set the resettCount from EEPROM
    resetCount=EEPROM.read(0x3DF);
    
    if (updateEEPROM) {
      if (debugModeOn) {Serial.println(" [initVariable] - Update EEPROM variables with values taken from global_setup.h");}
      EEPROM.commit();
    }
  }
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [initVariable] - configVariables="+String(configVariables));}
  if (debugModeOn) {Serial.println(" [initVariable] - wifiCred.wifiSSIDs[0]='"+wifiCred.wifiSSIDs[0]+"', wifiCred.wifiPSSWs[0]='"+wifiCred.wifiPSSWs[0]+"', wifiCred.wifiSITEs[0]='"+wifiCred.wifiSITEs[0]+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - wifiCred.wifiSSIDs[1]='"+wifiCred.wifiSSIDs[1]+"', wifiCred.wifiPSSWs[1]='"+wifiCred.wifiPSSWs[1]+"', wifiCred.wifiSITEs[1]='"+wifiCred.wifiSITEs[1]+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - wifiCred.wifiSSIDs[2]='"+wifiCred.wifiSSIDs[2]+"', wifiCred.wifiPSSWs[2]='"+wifiCred.wifiPSSWs[2]+"', wifiCred.wifiSITEs[2]='"+wifiCred.wifiSITEs[2]+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - ntpServers[0]='"+ntpServers[0]+"', ntpServers[1]='"+ntpServers[1]+"', ntpServers[2]='"+ntpServers[2]+"', ntpServers[3]='"+ntpServers[3]+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - TZEnvVariable='"+TZEnvVariable+"', TZName='"+TZName+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - userName='"+userName+"', userPssw='"+userPssw+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - mqttServer='"+mqttServer+"', mqttTopicPrefix='"+mqttTopicPrefix+"', mqttTopicName='"+mqttTopicName+"', mqttUserName='"+mqttUserName+"', mqttUserPssw='"+mqttUserPssw+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - wifiCred.SiteAllow[0]='"+String(wifiCred.SiteAllow[0])+"'"+", wifiCred.SiteAllow[1]='"+String(wifiCred.SiteAllow[1])+"'"+", wifiCred.SiteAllow[2]='"+String(wifiCred.SiteAllow[2])+"'");}
  if (debugModeOn) {Serial.println(" [initVariable] - bootCount="+String(bootCount)+", resetCount="+String(resetCount));}

  //If no SSID is setup, display message to warn and run AP mode
  if (wifiCred.wifiSSIDs[0]=="" && wifiEnabled) startAPMode=true;
}

void firstSetup() {
  static uint32_t wr = 1;
  static uint32_t rd = 0xFFFFFFFF;
  currentState=bootupScreen;lastState=currentState;
  displayMode=bootup;lastDisplayMode=bootup;

  if (debugModeOn) {Serial.print("\n[SETUP] - Doing regular CO2 bootup v");Serial.print(VERSION);Serial.println(" ..........");Serial.println("[setup] - Serial: OK");}

  //Date initialization to 1-Jan-2022 00:00:00
  // Necessary to avoid to get frozen when getLocalTime() is called if NTP is not synced.
  // https://stackoverflow.com/questions/72940013/why-getlocaltime-implementation-needs-delay
  ESP32Time timeTest(0);
  timeTest.setTime(0,0,0,1,1,2022);  // 1st Jan 2022 00:00:00
  if (debugModeOn) {Serial.println("[setup] - Initial date set to 1st Jan 2022 00:00:00. Will be updated after NTP sync");}
  getLocalTime(&startTimeInfo); //Getting initial local time. Will be overwritten if NTP sync suceeds
  //error_setup=NO_ERROR;

  //-->>Buttons init
  if (debugModeOn) Serial.print("[setup] - Buttons: ");
  button1.begin();
  button2.begin();
  if ((error_setup & ERROR_BUTTONS_SETUP)==0) { 
    if (debugModeOn) Serial.println("OK");
  } else {
    if (debugModeOn) {Serial.println("KO"); Serial.println("Can't continue. STOP");}
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Display init
  pinMode(PIN_TFT_BACKLIGHT,OUTPUT); 
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  delay(500);
  //Check out the TFT display
  tft.drawPixel(30,30,wr);
  rd = tft.readPixel(30,30);
  if (rd!=wr) {
    error_setup|=ERROR_DISPLAY_SETUP;
    if (debugModeOn) {
      Serial.println("[setup] - Display: KO");
      Serial.println("          Pixel value wrote = ");Serial.println(wr,HEX);
      Serial.println("          Pixel value read  = ");Serial.println(rd,HEX);
      Serial.println("          Can't continue");
      //-->> Setup LED for error indication
    }
    return; //Can't continue
  }
  else {
    if (debugModeOn) Serial.println("[setup] - Display: OK");
    tft.fillScreen(TFT_BLACK);
  }

  //Start AP Mode to configure WiFi SSIDs
  if (startAPMode) {
    if (debugModeOn) Serial.println(" [setup] - No WiFi configuration. Asking the user what to do.");
    currentState=bootAPScreen;lastState=bootupScreen;
    displayMode=bootAP;lastDisplayMode=bootup;
    
    //Based on user's entry, AP Mode is run or not
    if(runAPMode()) {
      //AP Mode is running
      if (SPIFFS.begin(true)) {
        if (debugModeOn) Serial.println(" [setup] - SPIFFS.begin=OK, SPIFFSErrors="+String(SPIFFSErrors));

        fileSystemSize = SPIFFS.totalBytes();
        fileSystemUsed = SPIFFS.usedBytes();
      }
      else {
        SPIFFSErrors++;
        if (debugModeOn) Serial.println(" [setup] - SPIFFS.begin=KO, SPIFFSErrors="+String(SPIFFSErrors));
      }
      if (debugModeOn) Serial.println(" [setup] - initAPWebServer="+String(initAPWebServer()));
    
      //Infinitive loop as ESP web server is running.
      //The ESP will be restarted by the user when config is finished
      while (true);
    }

    //No AP Mode, so there is no WiFi SSIDs. Let's continue.
    tft.fillScreen(TFT_BLACK);tft.setTextColor(TFT_YELLOW,TFT_BLACK);
    tft.drawString(" WiFi is disabled as it is not setup",5,45,TEXT_FONT_BOOT_SCREEN);
    tft.drawString("    Device will be now restarted",5,68,TEXT_FONT_BOOT_SCREEN);
    for (int i=6; i>0; i--) {tft.drawString("            in "+String(i)+" seconds",5,91,TEXT_FONT_BOOT_SCREEN);delay(1000);}
    tft.fillScreen(TFT_BLACK);
    wifiEnabled=false; //No WiFi as there is no SSID configured
    currentState=bootupScreen;lastState=currentState;
    displayMode=bootup;lastDisplayMode=bootup;
    if (debugModeOn) Serial.println(" [setup] - No AP Mode. WiFi disabled. Continue setup ");
  }

  // Create a sprite for text and initialize sprite parameters
  stext1.setColorDepth(4);
  stext1.createSprite(TFT_MAX_X, TFT_MAX_Y*SCROLL_PER_SPRITE);
  stext1.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  stext1.setTextColor(TFT_GOLD); // Gold text, no background
  stext1.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  stext1.setTextFont(TEXT_FONT_BOOT_SCREEN);
  pixelsPerLine=tft.fontHeight(TEXT_FONT_BOOT_SCREEN);
  spL=LINES_PER_TEXT_SPRITE;            //Number of Lines in the Sprite
  scL=LINES_PER_TEXT_SCROLL;            //Number of Lines in the Scroll
  spFL=1;                               //Sprite First Line Window
  spLL=spL;                             //Sprite Last Line Window
  scFL=(spL-scL)/2+1;                   //Scroll First Line Window
  scLL=scFL+scL-1;                      //Scroll Last Line Window
  pFL=scFL;                             //Pointer First Line
  pLL=pFL;                              //pointer Last Line written

  //-->loadAllIcons();
  //-->loadAllWiFiIcons();

  loadBootImage();
  delay(500);
  //Display messages
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_WHITE,TFT_BLACK);stext1.print("CO2 bootup v");stext1.print(VERSION);stext1.println(" ..........");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Display: [");stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  
  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Some Temp & Hum sensor checks and init
  if (debugModeOn) Serial.print("[setup] - Sensor Temp/HUM: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Tp/Hu:  [");
  pinMode(SI7021_SDA,INPUT_PULLUP); pinMode(SI7021_SCL,INPUT_PULLUP);
  tempHumSensor.begin(SI7021_SDA,SI7021_SCL);

  int errorSns = tempHumSensor.getError();
  uint8_t statSns = tempHumSensor.getStatus();

  if (!tempHumSensor.isConnected() || 0==tempHumSensorType.compareTo("UNKNOW"))
    error_setup|=ERROR_SENSOR_TEMP_HUM_SETUP;
  if ((error_setup & ERROR_SENSOR_TEMP_HUM_SETUP)==0) { 
    if (debugModeOn) {
      Serial.println("OK");
      Serial.print("  Tp/Hm Sensor type: "); Serial.println(tempHumSensorType); 
      //Serial.print("  Tp/Hm Sen. version: "); Serial.println(tempHumSensor.getFirmwareVersion());
      Serial.print("  Tp/Hm Sen. status: "); Serial.println(statSns,HEX);
      Serial.print("  Tp/Hm Sen.  error: "); Serial.println(errorSns,HEX);
      Serial.print("  Tp/Hm Sen. resolu.: "); Serial.println(tempHumSensor.getResolution());
    }
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Sensor: ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(tempHumSensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }
  else {
    if (debugModeOn) {
      Serial.println("KO");
      Serial.print("  Tp/Hm Sensor type: "); Serial.print(tempHumSensorType);Serial.println(" - Shouldn't be UNKNOWN");
      Serial.print("  Tp/Hm Sen. status: "); Serial.println(statSns,HEX);
      Serial.print("  Tp/Hm Sen.  error: "); Serial.print(errorSns,HEX);Serial.println(" - Should be 0");
      Serial.print("  Tp/Hm Sen. resolu.: "); Serial.println(tempHumSensor.getResolution());
    }
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sns. type: ");if (0==tempHumSensorType.compareTo("UNKNOW")) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(tempHumSensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sen. status: ");stext1.print(statSns,HEX);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sen.  error: ");if (0 != errorSns) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print(errorSns,HEX);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sen. resolu.: ");stext1.print(tempHumSensor.getResolution());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Sensor CO2 init
  if (debugModeOn) Serial.print("[setup] - Sensor: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Sens.:  [");
  co2SensorSerialPort.begin(9600);      // (Uno example) device to MH-Z19 serial start   
  co2Sensor.begin(co2SensorSerialPort); // *Serial(Stream) refence must be passed to library begin(). 
  co2Sensor.setRange(CO2_SENSOR_CO2_RANGE);             // It's aviced to setup range to 2000. Better accuracy

  //Some Co2 sensor checks
  memset(co2SensorVersion, '\0', 5);
  co2Sensor.getVersion(co2SensorVersion);
  if (CO2_SENSOR_CO2_RANGE!=co2Sensor.getRange() || (byte) 0 != co2Sensor.getAccuracy(false) ||
      0==co2SensorType.compareTo("UNKNOW"))
    error_setup|=ERROR_SENSOR_CO2_SETUP;
  if ((error_setup & ERROR_SENSOR_CO2_SETUP)==0) { 
    if (debugModeOn) {
      Serial.println("OK");
      Serial.print("  CO2 Sensor type: "); Serial.println(co2SensorType); 
      Serial.print("  CO2 Sensor version: "); Serial.println(co2SensorVersion);
      Serial.print("  CO2 Sensor Accuracy: "); Serial.println(co2Sensor.getAccuracy(false));
      Serial.print("  CO2 Sensor Range: "); Serial.println(co2Sensor.getRange());
    }
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor type: ");stext1.print(co2SensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor version: ");stext1.print(co2SensorVersion);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Accuracy: ");stext1.print(co2Sensor.getAccuracy(false));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Range: ");stext1.print(co2Sensor.getRange());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  } else {
    if (debugModeOn) {
      Serial.println("KO");
      Serial.print("  CO2 Sensor type: "); Serial.print(co2SensorType);Serial.println(" - Shouldn't be UNKNOWN");
      Serial.print("  CO2 Sensor version: "); Serial.println(co2SensorVersion);
      Serial.print("  CO2 Sensor Accuracy: "); Serial.print(co2Sensor.getAccuracy(false)); Serial.println(" - Should be 0");
      Serial.print("  CO2 Sensor Range: "); Serial.print(co2Sensor.getRange()); Serial.print(" - Should be ");Serial.println(CO2_SENSOR_CO2_RANGE);
    }
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor type: ");if (0==co2SensorType.compareTo("UNKNOW")) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(co2SensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor version: ");stext1.print(co2SensorVersion);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Accuracy: ");if ((byte) 0 != co2Sensor.getAccuracy(false)) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(co2Sensor.getAccuracy(false));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Range: ");if  (CO2_SENSOR_CO2_RANGE!=co2Sensor.getRange()) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(co2Sensor.getRange());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Initiating buffers to draw the Co2/Temp/Hum graphs
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)  {lastHourCo2Samples[i]=0;lastHourTempSamples[i]=0;lastHourHumSamples[i]=0;}
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++) {lastDayCo2Samples[i]=0;lastDayTempSamples[i]=0;lastDayHumSamples[i]=0;}

  //WiFi init
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - WiFi: ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  cuX=stext1.getCursorX(); cuY= pLL>scLL? stext1.getCursorY()-pixelsPerLine : stext1.getCursorY();
  stext1.setCursor(cuX,cuY);

  //User credentials definition already done in initVariables()
  
  if (debugModeOn) Serial.print("[setup] - WiFi: ");
  //Clean-up dots displayed after trying to get connected
  stext1.setCursor(cuX,cuY);
  for (int counter2=0; counter2<MAX_CONNECTION_ATTEMPTS*(wifiCred.activeIndex+1); counter2++) stext1.print(" ");
  stext1.setCursor(cuX,cuY);

  if (wifiEnabled) {//Only if WiFi is enabled
    error_setup|=wifiConnect(true,true,false,&auxLoopCounter,&auxCounter);

    //print Logs
    if ((error_setup & ERROR_WIFI_SETUP)==0 ) { 
      if (debugModeOn) Serial.println("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");//if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SSID:  ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print(wifiNet.ssid);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("    IP:  ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(WiFi.localIP().toString());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  MASK: ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(WiFi.subnetMask().toString());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  DFGW: ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(WiFi.gatewayIP().toString());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);

      //WifiNet is updated in printCurrentWiFi(), which is called by wifiConnect(true,X);
      //WiFi.RSSI() might be used instead, but doesn't hurt keeping wifiNet.RSSI instead, as printCurrentWiFi() is required
      // to print logs in here.
      if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
      else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
      else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
      else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
      else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
    } else {
      if (debugModeOn) Serial.println("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SSID: ");stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("No SSID Available");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      wifiCurrentStatus=wifiOffStatus;
    }
  }
  else {//If WiFi is not enabled, then inform
    if (debugModeOn) Serial.println("N/E");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("N/E");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");//if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    wifiCurrentStatus=wifiOffStatus;
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //WEB SERVER
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - WEB SERVER: [");
  if (wifiEnabled & webServerEnabled) {//Only if both Wifi WebServer is enabled
  //if (wifiEnabled & true) {//Only if both Wifi WebServer is enabled
    //error_setup|=initWebServer(webServer);
    error_setup|=initWebServer();
    
    //print Logs
    if ((error_setup & ERROR_WEB_SERVER)==0 ) { 
      if (debugModeOn) Serial.println("[setup] - WEB SERVER: OK");
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    } else {
      if (debugModeOn) {Serial.println("[setup] - WEB SERVER: KO");}
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    }
  }
  else {//If either WiFi or webServer is not enabled, then inform
    if (debugModeOn) Serial.println("[setup] - WEB SERVER: N/E");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("N/E");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));
  
  //SPIFFS
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - SPIFFS: [");
  
  if(!SPIFFS.begin(true)){
    error_setup|=ERROR_SPIFFS_SETUP;
    SPIFFSErrors++;
    if (debugModeOn) Serial.println("[setup] - SPIFFS.begin=KO, SPIFFSErrors="+String(SPIFFSErrors));
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }
  else {
    if (debugModeOn) Serial.println("[setup] - SPIFFS.begin=OK, SPIFFSErrors="+String(SPIFFSErrors));
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);

    fileSystemSize = SPIFFS.totalBytes();
    fileSystemUsed = SPIFFS.usedBytes();
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));
  
  //Pre-setting up URL things to upload samples to an external server
  //Converting SERVER_UPLOAD_SAMPLES into IPAddress variable
  serverToUploadSamplesIPAddress=stringToIPAddress(serverToUploadSamplesString);
  
  if (debugModeOn) {Serial.print("[setup] - URL: ");}
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - URL: ");

  CloudSyncCurrentStatus=CloudSyncOffStatus;
  if ((error_setup & ERROR_WIFI_SETUP)==0 && wifiEnabled && uploadSamplesEnabled) { 
    //Send HttpRequest to check the server status
    // The request updates CloudSyncCurrentStatus
    error_setup|=sendAsyncHttpRequest(true,true,error_setup,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,String(GET_REQUEST_TO_UPLOAD_SAMPLES)+"test HTTP/1.1",&whileWebLoopTimeLeft);

    if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
      if (debugModeOn) {Serial.println("[OK]");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  URL: ");
      stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);
    }
    else {
      if (debugModeOn) {Serial.println("[KO]");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  URL: ");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);
    }

    if (debugModeOn) {Serial.print("  - URL: ");Serial.println("http://"+serverToUploadSamplesIPAddress.toString()+
        String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1));
        Serial.print("  - Device name=");Serial.println(device);
    }

    stext1.print("http://"+serverToUploadSamplesIPAddress.toString()+
        String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine); 
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Device name: ");
    stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print(device);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }
  else {
    if ((error_setup & ERROR_WIFI_SETUP)!=0 || !wifiEnabled) {
      if (debugModeOn) {Serial.println("No WiFi");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("No WiFi");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    }
    else { 
      if (debugModeOn) {Serial.println("N/E");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("N/E");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    }
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //NTP Server
  CloudClockCurrentStatus=CloudClockOffStatus;
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - NTP: [");
  if (wifiCurrentStatus!=wifiOffStatus && wifiEnabled) { 
    error_setup|=setupNTPConfig(true,&auxLoopCounter2,&whileLoopTimeLeft); //Control variables were init in initVariables()
    lastTimeNTPCheck=loopStartTime+millis();  //loopStartTime=0 just right after bootup
    if ((error_setup & ERROR_NTP_SERVER)!=0) {
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  NTP Server: ");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print((String)(ntpServers[0]+" "+ntpServers[1]+" "+ntpServers[2]+" "+ntpServers[3]));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      //Add one more line as NTP servers lists might need 2 lines
      if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    }
    else {
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK); stext1.print("  Date: ");getLocalTime(&startTimeInfo);stext1.print(&startTimeInfo,"%d/%m/%Y - %H:%M:%S");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  NTP Server: ");
      stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print(ntpServers[ntpServerIndex]);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      startTimeConfigure=true;
    }
  }
  else {
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("No WiFi");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Setting up MQTT server
  MqttSyncCurrentStatus=MqttSyncOffStatus;
  error_setup|=mqttClientInit(true,true,true);

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //-->>Battery and ADC init
  if (debugModeOn) Serial.print("[setup] - Bat. ADC: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Bat. ADC [");
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE);
  initVoltageArray(); //Init battery charge array
  powerState=off;
  attenuationDb=ADC_ATTEN_DB_11;
  esp_adc_cal_characterize(ADC_UNIT_1, attenuationDb, (adc_bits_width_t) ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
  if (0!=adc1_config_channel_atten(ADC1_CHANNEL_6, attenuationDb)) error_setup|=ERROR_BAT_ADC;
  if ((error_setup & ERROR_BAT_ADC)==0 ) { 
    if (debugModeOn) {
      Serial.println("OK");
      Serial.print(" - adc_num=");Serial.println(adc1_chars.adc_num);
      Serial.print(" - atten=");Serial.println(adc1_chars.atten);
      Serial.print(" - bit_width=");Serial.println(adc1_chars.bit_width);
      Serial.print(" - coeff_a=");Serial.println(adc1_chars.coeff_a);
      Serial.print(" - coeff_b=");Serial.println(adc1_chars.coeff_b);
      Serial.print(" - vref=");Serial.println(adc1_chars.vref);
      Serial.print(" - version=");Serial.println(adc1_chars.version);
    }

    //Take BAT ADC for setup powerState
    //lastBatCharge is initiated in setup() as the value depends on the wakup reason
    batCharge=0;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
    batADCVolt=0; for (u8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE);
    
    /*--><--*///batADCVolt=1900;

    if (batADCVolt >= VOLTAGE_TH_STATE) {
      timeUSBPowerGlobal=nowTimeGlobal;
      powerState=chargingUSB; //check later if powerState is noChargingUSB instead
      
      //When USB is plugged, the Battery charge can be only guessed based on
      // the time the USB is being plugged
      batteryStatus=getBatteryStatus(batADCVolt,nowTimeGlobal-timeUSBPowerGlobal);

      energyCurrentMode=fullEnergy; //No deep sleep if USB power
      autoBackLightOff=false; //update autoBackLightOff if USB power
    }
    else {
      powerState=onlyBattery;

      //Take battery charge when the Battery is plugged
      batteryStatus=getBatteryStatus(batADCVolt,0);
      timeUSBPowerGlobal=0;
      
      //Set the rigth Saving Energy mode based on the BAT charge
      //if (batCharge>=BAT_CHG_THR_FOR_SAVE_ENERGY) energyCurrentMode=reducedEnergy;
      if (batCharge>=BAT_CHG_THR_FOR_SAVE_ENERGY) energyCurrentMode=configSavingEnergyMode; //Normally reducedEnergy if not changed in the Config Menu
      else energyCurrentMode=lowestEnergy; 
      autoBackLightOff=true; //update autoBackLightOff if BAT power
    }
    if (debugModeOn) {
      Serial.println(" - voltage="+String(batADCVolt)+", charge="+String(batCharge)+"%");
      Serial.println(" - powerState="+String(powerState)+", energyCurrentMode="+String(energyCurrentMode));
    }

    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK); stext1.print(" V="+String((int)batADCVolt)+"mv, "+String(batCharge)+"%, pwState="+String(powerState));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);    
  } else {
    if (debugModeOn) Serial.println("KO");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (debugModeOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  #if BUILD_ENV_NAME==BUILD_TYPE_DEVELOPMENT
    //Clean up ERROR_SENSOR_CO2_SETUP for development as there's no CO2 sensor
    error_setup&=~ERROR_SENSOR_CO2_SETUP; 
  #endif

  if ((error_setup & DEAD_ERRORS)!=0) {
    Serial.println("[setup] - There are dead errors. Can't continue. STOP");
    if ((error_setup & ERROR_DISPLAY_SETUP)!=0) Serial.println("  - ERROR_DISPLAY_SETUP");
    if ((error_setup & ERROR_SENSOR_TEMP_HUM_SETUP)!=0) Serial.println("  - ERROR_SENSOR_TEMP_HUM_SETUP");
    if ((error_setup & ERROR_SENSOR_CO2_SETUP)!=0) Serial.println("  - ERROR_SENSOR_CO2_SETUP");
    if ((error_setup & ERROR_BUTTONS_SETUP)!=0) Serial.println("  - ERROR_BUTTONS_SETUP");
    if ((error_setup & ERROR_SPIFFS_SETUP)!=0) Serial.println("  - ERROR_SPIFFS_SETUP");
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.print(" ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("Detected dead errors. Can't continue.");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine); //Add one more line
  }
  else if ((error_setup | NO_ERROR)!=0) {
    if (debugModeOn) Serial.println("Ready to start but with limitations, error_setup="+String(error_setup));
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("Buttons to scroll UP/DOWN");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_ORANGE_4_BITS_PALETTE,TFT_BLACK);stext1.print("Ready in ");cuX=stext1.getCursorX();cuY=stext1.getCursorY();
    stext1.print(BOOTUP_TIMEOUT);stext1.print(" sec. but with limitations");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  } else {
    if (debugModeOn) Serial.println("Ready to start");
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("Buttons to scroll UP/DOWN");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("Ready to start in ");cuX=stext1.getCursorX();cuY=stext1.getCursorY();
    stext1.print(BOOTUP_TIMEOUT);stext1.print(" sec.");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  //First setup of periods
  switch (energyCurrentMode) {
    case fullEnergy:
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD;
      samplePeriod=SAMPLE_PERIOD;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD;
      BLEPeriod=BLE_PERIOD;
      BLEOnTimeout=BLE_ON_TIMEOUT;
      if (BLEPeriod<=BLEOnTimeout) BLEPeriod=BLEOnTimeout+500;
    break;
    case reducedEnergy:
      //If TFT is off then sleep mode is active, so let's reduce the VOLTAGE_CHECK_PERIOD period
      //This makes detection of USB/Battery supply every VOLTAGE_CHECK_PERIOD when the display is ON
      if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_RE; 
      else voltageCheckPeriod=VOLTAGE_CHECK_PERIOD;
      samplePeriod=SAMPLE_PERIOD_RE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_RE;
      BLEPeriod=BLE_PERIOD_RE;
      BLEOnTimeout=BLE_ON_TIMEOUT_RE;
      if (BLEPeriod<=BLEOnTimeout) BLEPeriod=BLEOnTimeout+500;
    break;
    case lowestEnergy:
      //If TFT is off then sleep mode is active, so let's reduce the VOLTAGE_CHECK_PERIOD period
      //This makes detection of USB/Battery supply every VOLTAGE_CHECK_PERIOD when the display is ON
      if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_SE; 
      else voltageCheckPeriod=VOLTAGE_CHECK_PERIOD;
      samplePeriod=SAMPLE_PERIOD_SE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_SE;
      BLEPeriod=BLE_PERIOD_SE;
      BLEOnTimeout=BLE_ON_TIMEOUT_SE;
      if (BLEPeriod<=BLEOnTimeout) BLEPeriod=BLEOnTimeout+500;
    break;
  }
  
  //Updating timers after first setup
  nowTimeGlobal=loopStartTime+millis();
  previousTime=nowTimeGlobal;
  remainingBootupTime=BOOTUP_TIMEOUT*1000;
  remainingBootupSeconds=(uint8_t)(remainingBootupTime/1000);
  
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [setup] - exit");}
}

boolean warmingUp() {
  //Run this code only after first bootup

  nowTimeGlobal=loopStartTime+millis();
  if ((int)remainingBootupTime>0) {
    //Only while showing bootup screen
    remainingBootupTime-=(nowTimeGlobal-previousTime);
    previousTime=nowTimeGlobal;
  }

  //Keep showing bootup screen for a while
  if (currentState==bootupScreen) {
      if (remainingBootupSeconds!=(uint8_t)(remainingBootupTime/1000)) {
      remainingBootupSeconds=(uint8_t)(remainingBootupTime/1000);
      
      //Update screen if last line is visible in the scroll window
      if (pLL-1>=scFL && pLL-1<=scLL) {
        //Only if not dead errors
        //if (ERROR_DISPLAY_SETUP!=error_setup && ERROR_SENSOR_CO2_SETUP!=error_setup && 
        //ERROR_SENSOR_TEMP_HUM_SETUP!=error_setup && ERROR_BUTTONS_SETUP!=error_setup) {
        if ((error_setup & DEAD_ERRORS)==0) {
          stext1.setCursor(cuX,(pLL-2)*pixelsPerLine);stext1.print(remainingBootupSeconds);
          stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
        }
      }
    }
    
    //Scroll DOWN
    if (button1.pressed()) { 
      if (pFL<scFL) {
        pFL++;
        if (pLL-1<spLL) pLL++;
        stext1.scroll(0,pixelsPerLine);
        stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
      //Reset timeout to give more time to read while scrolling the screen 
      if (pLL-1>=scFL && pLL-1<=scLL)
        remainingBootupTime=BOOTUP_TIMEOUT*1000;
      else
        remainingBootupTime=BOOTUP_TIMEOUT2*1000; //If last line is not in scroll windows, wait a bit more
    }

    //Scroll UP
    if (button2.pressed()) {
      if (pLL-1>scLL) {
        pLL--;
        if (pFL>spFL) pFL--;
        stext1.scroll(0,-pixelsPerLine);
        stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
      //Reset timeout to give more time to read while scrolling the screen 
      if (pLL-1>=scFL && pLL-1<=scLL)
        remainingBootupTime=BOOTUP_TIMEOUT*1000;
      else
        remainingBootupTime=BOOTUP_TIMEOUT2*1000; //If last line is not in scroll windows, wait a bit more
    }
    
    //if (ERROR_DISPLAY_SETUP==error_setup || ERROR_SENSOR_CO2_SETUP==error_setup || 
    //    ERROR_SENSOR_TEMP_HUM_SETUP==error_setup || ERROR_BUTTONS_SETUP==error_setup)
    if ((error_setup & DEAD_ERRORS)!=0) //There's a dead error
          remainingBootupTime=BOOTUP_TIMEOUT*1000;  //Can't continue. Dead errors. Infinite loop
          
    if ((int)remainingBootupTime<=0) {
      //After BOOTUP_TIMEOUT secons with no activity leaves bootupScreen and continue
      tft.setCursor(0,0,TEXT_FONT);
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(TEXT_SIZE);
      displayMode=sampleValue;lastDisplayMode=bootup; //Will make starting in sampleValue      
      lastState=currentState;currentState=displayingSequential;  //Transition to the next state
      previousTime=0;
    }
  
    return (true);
  }
  
  //Show Warming up screen
  uint auxCounter=0;
  while (nowTimeGlobal<co2PreheatingTime) {
    auxCounter++;
    //Waiting for the sensor to warmup before displaying value
    if (waitingMessage) {if (debugModeOn) Serial.println("Waiting for the warmup to finish");circularGauge.drawGauge2(0);waitingMessage=false;}
    circularGauge.cleanValueTextGauge();
    circularGauge.cleanUnitsTextGauge();
    circularGauge.setValue((int)(co2PreheatingTime-millis())/1000);
    circularGauge.drawTextGauge("warmup",TEXT_SIZE,true,TEXT_SIZE_UNITS_CO2,TEXT_FONT,TEXT_FONT_UNITS_CO2,TFT_GREENYELLOW);

    //Update icons every 5 seconds
    if (5==auxCounter) {
      //Update WiFi icon
      if (WiFi.status()!=WL_CONNECTED) {
        //WiFi is not connected. Update wifiCurrentStatus properly
        wifiCurrentStatus=wifiOffStatus;
      }
      else {
        //Take RSSI to update the WiFi icon
        if (WiFi.RSSI()>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
        else if (WiFi.RSSI()>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
        else if (WiFi.RSSI()>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
        else if (WiFi.RSSI()>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
        else if (WiFi.RSSI()<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
      }
      //Update BAT icon
      updateBatteryVoltageAndStatus(millis(), &timeUSBPowerGlobal);
      
      auxCounter=0;
    }
    
    showIcons();
    delay(1000);
    nowTimeGlobal=loopStartTime+millis();
  }
  lastTimeTurnOffBacklightCheck=nowTimeGlobal;
  if (runningMessage) {getLocalTime(&nowTimeInfo);Serial.print("Time: ");Serial.print(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");Serial.println(".  Running.... :-)");runningMessage=false;}
  
  button1.released(); //Avoids passing the button status to the loop()
  button2.released(); //Avoids passing the button status to the loop()

  //Release RAM
  stext1.deleteSprite();

  return (false);
}

void setup() {
  loopStartTime=loopEndTime+millis()+sleepTimer/1000;
  nowTimeGlobal=loopStartTime;

  // Initialize EEPROM with predefined size. Config variables ares stored there
  EEPROM.begin(EEPROM_SIZE);
  
  Serial.begin(115200);
  if (debugModeOn) {Serial.println("\n"+String(nowTimeGlobal)+" [SETUP] - bootCount="+String(bootCount)+", nowTime="+String(millis())+", nowTimeGlobal="+String(nowTimeGlobal));}
  randomSeed(analogRead(GPIO_NUM_32));

  //Init BLE pointers before using TFT Sprite due to memory size constraints
  //BLE Memory is released after setupBLE is finished
  BLEClurrentStatus=BLEOffStatus;
  if (EEPROM.read(0x08) & 0x08) { //bluetoothEnabled
    //error_setup=0;
    error_setup=setupBLE();
    if (error_setup==0) {
      BLEClurrentStatus=BLEOnStatus;
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [SETUP] - BLE init OK");}
    }
    else
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [SETUP] - BLE init KO");}
  }
  else
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [SETUP] - BLE N/E");}
    
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT1: //Wake up from Hibernate Mode by long pressing Button1
      initVariable(); //Init Global variables after hibernate mode
      loopStartTime=millis();loopEndTime=0;sleepTimer=0;nowTimeGlobal=loopStartTime;
      if (debugModeOn) {Serial.println("  - Wakeup caused by external signal using RTC_CNT - Ext1");Serial.println("   - loopEndTime="+String(loopEndTime)+", sleepTimer="+String(sleepTimer/1000)+", loopStartTime="+String(loopStartTime)+", nowTimeGlobal="+String(nowTimeGlobal));}
      //To wake from hibernate, Button1 must be preseed TIME_LONG_PRESS_BUTTON1_HIBERNATE seconds
      button1.begin();
      boolean stopCheck;
      stopCheck=false;
      if (!button1.pressed()) stopCheck=true;
      while (!stopCheck) { //Wait till the button is released
        if (button1.released()) stopCheck=true;
        else delay(100);
      } //Wait to release button1
      
      if (debugModeOn) {Serial.println("    - Time elapsed: "+String(millis()-nowTimeGlobal)+" ms");}
      if ((millis()-nowTimeGlobal) > TIME_LONG_PRESS_BUTTON1_HIBERNATE) { //Hard bootup - Run global setup from scratch
        
        //Getting the last known value of Bat charge when waken up from hibernation
        uint32_t auxBatCharge=0;
        for (int i=(9+sizeof(auxBatCharge)-1); i>=9; i--) {auxBatCharge=auxBatCharge<<8;auxBatCharge|=EEPROM.read(i);}
        lastBatCharge=(float_t)auxBatCharge;
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"    - afte wakeup, lastBatCharge="+String(lastBatCharge));}

        buttonWakeUp=true;
        firstSetup();
        nowTimeGlobal=loopStartTime+millis();
        return;
      }  
      else { //Going back to hibernate - Button1 wasn't pressed time enough
        if (debugModeOn) {Serial.println("    - Going back to hibernate");delay(1000);}
        go_to_hibernate();
      } 
    break;
    case ESP_SLEEP_WAKEUP_EXT0: //Wake up from Deep Sleep Mode by pressing Button1
      // No need to initit lastBatCharge
      if (debugModeOn) {
        Serial.println("  - Wakeup caused by external signal using RTC_IO - Ext0");
        Serial.println("    - setting things up back again after deep sleep specific for ad-hod wakeup");
        Serial.println("      - TFT init");
      }
      initTZVariables(); //To make sure that both NTP sync and NTP info in web are right
      CloudClockCurrentStatus=CloudClockOffStatus; //To update icons as WiFi is disconnect
      CloudSyncCurrentStatus=CloudSyncOffStatus; //To update icons as WiFi is disconnect
      //Display init
      pinMode(PIN_TFT_BACKLIGHT,OUTPUT); 
      tft.init();
      digitalWrite(PIN_TFT_BACKLIGHT,LOW); //To force checkButton1() function to setup things
      tft.setRotation(1);
      tft.fillScreen(TFT_BLACK);
      if (debugModeOn) {Serial.println("      - checkButton1() & buttonWakeUp=true");}
      checkButton1();
      buttonWakeUp=true;
      //buttonWakeUp is used for Web Server init and SPFIS init in the next WiFi interaction
      nowTimeGlobal=loopStartTime+millis();
      lastTimeTurnOffBacklightCheck=nowTimeGlobal; //To avoid TIME_TURN_OFF_BACKLIGHT 
      displayMode=sampleValue;  //To force refresh TFT with the sample value Screen
      lastDisplayMode=bootup;   //To force rendering the value graph
      forceWifiReconnect=true; //Force WiFi reconnection in the next loop - v1.1.0
      forceWebServerInit=true; //Force webServer restart - v1.2.0
      if (debugModeOn) {Serial.println("    - end");}
    break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      // No need to initit lastBatCharge
      if (debugModeOn) {
        Serial.println("  - Wakeup caused by timer");
        Serial.println("    - setting things up back again after deep sleep specific for periodic wakeup");
        Serial.println("      - timers init");
      }
      nowTimeGlobal=loopStartTime+millis();
      if (debugModeOn) {Serial.println("    - end");}
    break;
    default:
      if (debugModeOn) {Serial.println("  - Wakeup was not caused by deep sleep: "+String(wakeup_reason)+" (0=POWERON_RESET, including HW Reset)");}
      lastBatCharge=0; //Can't be initiated in initVariable() as the value depends on the reason to wakeup.
      initVariable(); //Init Global variables (it makes sure configVariables take the values stored in EEPROM)
      bootCount++;EEPROM.write(0x3DE,bootCount); //Update bootCount every time the device start (no wake up due to hibernate nor deep sleep)
      //Check if resetCount needs to be updated
      switch (esp_reset_reason()) { //v1.2.0 - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv418esp_reset_reason_t
        case ESP_RST_UNKNOWN: //Reset reason can not be determined
        case ESP_RST_PANIC: //Software reset due to exception/panic
        case ESP_RST_INT_WDT: //Reset (software or hardware) due to interrupt watchdog.
        case ESP_RST_TASK_WDT: //Reset due to task watchdog
        case ESP_RST_WDT: //Reset due to other watchdogs
          //In all these cases, there was a wrong code that triggered the reset. Count it
          resetCount++;
          EEPROM.write(0x3DF,resetCount);
        break;
        case ESP_RST_POWERON: //Power-on event
        case ESP_RST_EXT: //External pin (not applicable for ESP32)
        case ESP_RST_SW: //Software reset via esp_restart()
        case ESP_RST_DEEPSLEEP: //Reset after exiting deep sleep mode
        case ESP_RST_BROWNOUT: //Brownout reset (software or hardware) - Supply voltage goes below safe level
        case ESP_RST_SDIO: //Reset over SDIO
        default:
        break;
      }
      EEPROM.commit(); //Update bootCount and resetCount values
      firstSetup(); //Hard bootup - Run global setup during the first boot (HW reset or power ON)
      nowTimeGlobal=loopStartTime+millis();
      return;
    break;
  }
  
  //This piece of code is run after wakeup from Deep Sleep (neither HW reset nor Hibernate)
  //Run global setup after wakeup
  nowTimeGlobal=loopStartTime+millis();

  if (debugModeOn) {
    Serial.println(String(nowTimeGlobal)+"  - Setting common things up back again after deep sleep");
    Serial.println("      - pinMode(POWER_ENABLE_PIN, OUTPUT) & initVoltageArray() Init battery charge array");
  }
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  initVoltageArray(); //Init battery charge array
  pinMode(PIN_TFT_BACKLIGHT,OUTPUT); 

  if (debugModeOn) {Serial.println("      - co2SensorSerialPort.begin() and  co2Sensor.begin() again after deep sleep");}
  co2SensorSerialPort.begin(9600);      // (Uno example) device to MH-Z19 serial start   
  co2Sensor.begin(co2SensorSerialPort); // *Serial(Stream) refence must be passed to library begin(). 
  co2Sensor.setRange(CO2_SENSOR_CO2_RANGE);             // It's aviced to setup range to 2000. Better accuracy
  
  if (debugModeOn) {Serial.println("      - tempHumSensor.begin() again after deep sleep");}
  tempHumSensor.begin(SI7021_SDA,SI7021_SCL); //Needed again after deep sleep

  if (debugModeOn) {Serial.println("      - wifiCred variables");}
  //User credentials definition - Not run if waking up either from button or timer
  char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH];
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0x0D,auxSSID);wifiCred.wifiSSIDs[0]=auxSSID;
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x2E,auxPSSW);wifiCred.wifiPSSWs[0]=auxPSSW;
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0x6E,auxSITE);wifiCred.wifiSITEs[0]=auxSITE;
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0x79,auxSSID);wifiCred.wifiSSIDs[1]=auxSSID;
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x9A,auxPSSW);wifiCred.wifiPSSWs[1]=auxPSSW;
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0xDA,auxSITE);wifiCred.wifiSITEs[1]=auxSITE;
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0xE5,auxSSID);wifiCred.wifiSSIDs[2]=auxSSID;
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x106,auxPSSW);wifiCred.wifiPSSWs[2]=auxPSSW;
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0x146,auxSITE);wifiCred.wifiSITEs[2]=auxSITE;

  //NTP servers definition - Not run if waking up from either button or timer
  char auxNTP[NTP_SERVER_NAME_MAX_LENGTH];
  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x151,auxNTP);ntpServers[0]=auxNTP;
  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x191,auxNTP);ntpServers[1]=auxNTP;
  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x1D1,auxNTP);ntpServers[2]=auxNTP;
  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x211,auxNTP);ntpServers[3]=auxNTP;

  //TZEnvVariable definition - Not run before if waking up from either button or timer
  char auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH];
  memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);EEPROM.get(0x251,auxTZEnvVar);TZEnvVariable=String(auxTZEnvVar);
  
  //wifiCred.SiteAllow definition  - Not run if waking up from either button or timer
  configVariables=EEPROM.read(0x2BE);
  wifiCred.SiteAllow[0]=configVariables & 0x01;
  wifiCred.SiteAllow[1]=configVariables & 0x02;
  wifiCred.SiteAllow[2]=configVariables & 0x04;

  if (debugModeOn) {Serial.println("      - wifiCurrentStatus=wifiOffStatus till WiFi reconnection");}
  wifiCurrentStatus=wifiOffStatus;

  //serverToUploadSamplesIPAddress=IPAddress(uploadServerIPAddressOctectArray[0],uploadServerIPAddressOctectArray[1],uploadServerIPAddressOctectArray[2],uploadServerIPAddressOctectArray[3]);
  serverToUploadSamplesIPAddress=stringToIPAddress(String(SERVER_UPLOAD_SAMPLES)); //v0.9.9.D
  device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
    String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
    String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);
  if (debugModeOn) {Serial.println("      - serverToUploadSamplesIPAddress="+IpAddress2String(serverToUploadSamplesIPAddress)+" & device="+device);}

  if (debugModeOn) {Serial.println("      - Restoring TZEnvVar="+String(TZEnvVar));}
  setenv("TZ",TZEnvVar,1); tzset(); //Restore TZ enviroment variable to show the right time

  //Web variables that need to be initialized
  flashSize = ESP.getFlashChipSize();
  programSize = ESP.getSketchSize();
  OTAAvailableSize=getAppOTAPartitionSize(ESP_PARTITION_TYPE_APP,ESP_PARTITION_SUBTYPE_ANY);
  SPIFFSAvailableSize=getAppOTAPartitionSize(ESP_PARTITION_TYPE_DATA,0x82);
  fileSystemSize=SPIFFS.totalBytes();
  fileSystemUsed=SPIFFS.usedBytes();
  
  //Init users credentials
  char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
  memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2A8,auxUserName);userName=auxUserName;
  memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);EEPROM.get(0x2B3,auxUserPssw); userPssw=auxUserPssw;
  
  //Init MQTT String stuff
  char auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH],auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
  memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x2BF,auxMQTT);mqttServer=auxMQTT;
  memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);EEPROM.get(0x315,auxMqttTopicPrefix);mqttTopicPrefix=auxMqttTopicPrefix;mqttTopicName=mqttTopicPrefix+device;
  memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2FF,auxUserName);mqttUserName=auxUserName;
  memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);EEPROM.get(0x30A,auxUserPssw);mqttUserPssw=auxUserPssw;

  nowTimeGlobal=loopStartTime+millis();
  if (debugModeOn) {Serial.print(String(nowTimeGlobal)+" [SETUP] - Exit - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
}


void loop() {
  uint32_t minHeap=esp_get_minimum_free_heap_size();
  if(minHeap<minHeapSeen) minHeapSeen=minHeap; //Track the minimun heap size (bytes)

  nowTimeGlobal=loopStartTime+millis();
  loopCount++;

  //After firstSetup, the CO2 sensor must warmup before starting to take samples
  // Bootup messages review and WarmUp screen is displayied before the CO2 sensor is 
  // ready to work
  if (firstBoot) if(warmingUp()) return;

  //********************************************************************
  //                     MAIN LOOP START HERE
  //********************************************************************
  
  //Do periodic actions based on TIMEOUTS and PERIODS

  //Cheking if BackLight should be turned off
  nowTimeGlobal=loopStartTime+millis();
  if (((nowTimeGlobal-lastTimeTurnOffBacklightCheck) >= TIME_TURN_OFF_BACKLIGHT) && !firstBoot && 
       digitalRead(PIN_TFT_BACKLIGHT)!=LOW && autoBackLightOff==true) {
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - TIME_TURN_OFF_BACKLIGHT");}
    
    if (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed ||
        currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) {

      //Switch the Display OFF and print black screen to save energy consume
      //(assuming black screen consumes less energy, even though the Display is OFF)
      tft.fillScreen(TFT_BLACK);
      digitalWrite(PIN_TFT_BACKLIGHT,LOW);
      lastTimeTurnOffBacklightCheck=nowTimeGlobal;
    }
  }
  
  // Checking if sample buffers update is needed
  nowTimeGlobal=loopStartTime+millis();
  if ((nowTimeGlobal-lastTimeHourSampleCheck) >= SAMPLE_T_LAST_HOUR*1000) {lastTimeHourSampleCheck=nowTimeGlobal; updateHourSample=true;updateHourGraph=true;}
  if ((nowTimeGlobal-lastTimeDaySampleCheck) >= SAMPLE_T_LAST_DAY*1000) {lastTimeDaySampleCheck=nowTimeGlobal; updateDaySample=true;updateDayGraph=true;}
  
  //Actions if button1 is pushed. It depens on the current state
  //Avoid this action the first loop interaction just right after wakeup by pressing a button
  nowTimeGlobal=loopStartTime+millis();
  checkButtonsActions(mainloop);
  
  //Regular actions every VOLTAGE_CHECK_PERIOD seconds
  //Checking out whether the voltage exceeds thresholds to detect energy source (bat or USB)
  // but only if Full Energy Mode (USB power to save energy consume)
  //forceGetVolt==true if buttons are pressed to wakeup CPU
  nowTimeGlobal=loopStartTime+millis();
  //if ( (((nowTimeGlobal-lastTimeVOLTCheck) >= voltageCheckPeriod) && fullEnergy==energyCurrentMode) || forceGetVolt ) {
  if (((nowTimeGlobal-lastTimeVOLTCheck) >= voltageCheckPeriod) || forceGetVolt ) {
    if (debugModeOn) {Serial.println("-------------oooooOOOOOOoooooo------------");Serial.println(String(nowTimeGlobal)+"  - VOLTAGE_CHECK_PERIOD");}

    //voltageCheckPeriod:
    // * energyCurrentMode == fullEnergy    => voltageCheckPeriod=VOLTAGE_CHECK_PERIOD (5 sg)
    // * energyCurrentMode == reducedEnergy => voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_RE (5 min)
    // * energyCurrentMode == lowestEnergy  => voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_SE (5 min)

    //batADCVolt update
    //Power state check and powerState update
    //batteryStatus update
    //Saving energy Mode update (energyCurrentMode)
    //Auto Display Switch Off update (autoBackLightOff)
    //If USB is plugged, timeUSBPowerGlobal is updated with nowTimeGlobal, to estimate batteryCharge based on time
    //If USB is unplugged, timeUSBPowerGlobal is set to zero
    updateBatteryVoltageAndStatus(nowTimeGlobal, &timeUSBPowerGlobal);
  
    lastTimeVOLTCheck=nowTimeGlobal;  //There're no updates of lastTimeVOLTCheck in updateBatteryVoltageAndStatus()
    if (forceGetVolt) forceGetVolt=false;
  }

  //Regular actions every SAMPLE_PERIOD seconds.
  //Taking CO2, Temp & Hum samples. Moving buffers at the right time
  //forceGetSample==true if buttons are pressed
  nowTimeGlobal=loopStartTime+millis();
  if (((nowTimeGlobal-lastTimeSampleCheck) >= samplePeriod) || forceGetSample || firstBoot) {  
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - SAMPLE_PERIOD");}

    previousLastTimeSampleCheck=lastTimeSampleCheck;
    lastTimeSampleCheck=nowTimeGlobal; //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
    
    //Getting CO2 & Temp values. VDC below MIN_VOLT, the measure is not realiable
    if (batADCVolt<=MIN_VOLT) valueCO2=-1;
    else valueCO2=(float_t)co2Sensor.getCO2();
    if (debugModeOn) {Serial.println("    - valueCO2="+String(valueCO2));}
    
    //tempMeasure=co2Sensor.getTemperature(true,true);
    tempHumSensor.read();
    //tempMeasure=tempHumSensor.getTemperature();
    tempMeasure=0.9944*tempHumSensor.getTemperature()-0.8073; //Calibrated value
    //valueHum=0;
    valueHum=tempHumSensor.getHumidityCompensated();
    
    if (tempMeasure>-50.0) valueT=tempMeasure;  //Discarding potential wrong values

    if (debugModeOn) {Serial.println("    - valueT="+String(valueT)+", valueHum="+String(valueHum));}

    //Updating the last hour buffers
    if (updateHourSample) {
      for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR)-1; i++) 
      {lastHourCo2Samples[i]=lastHourCo2Samples[i+1];
      lastHourTempSamples[i]=lastHourTempSamples[i+1];
      lastHourHumSamples[i]=lastHourHumSamples[i+1];
      }

      //In Reduced or Save Energy Modes, there are lower number of samples (greater period)
      //so the last hour buffer is fillied by repeating the same sample
      int lastHourPeriodRatio;
      if (fullEnergy==energyCurrentMode) lastHourPeriodRatio=1;
      else if (reducedEnergy==energyCurrentMode) lastHourPeriodRatio=(int)(SAMPLE_T_LAST_HOUR_RE/SAMPLE_T_LAST_HOUR);
      else lastHourPeriodRatio=(int)(SAMPLE_T_LAST_HOUR_SE/SAMPLE_T_LAST_HOUR);
      for (int j=0;j<lastHourPeriodRatio; j++)
      {
        lastHourCo2Samples[(int)(3600/SAMPLE_T_LAST_HOUR)-1-j]=valueCO2;
        lastHourTempSamples[(int)(3600/SAMPLE_T_LAST_HOUR)-1-j]=valueT;
        lastHourHumSamples[(int)(3600/SAMPLE_T_LAST_HOUR)-1-j]=valueHum;
      }
      updateHourSample=false;
    }
    
    //Updating the last day Co2 buffer
    if (updateDaySample) {
      for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY)-1; i++) {
        lastDayCo2Samples[i]=lastDayCo2Samples[i+1];
        lastDayTempSamples[i]=lastDayTempSamples[i+1];
        lastDayHumSamples[i]=lastDayHumSamples[i+1];
      }
      lastDayCo2Samples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueCO2;
      lastDayTempSamples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueT;
      lastDayHumSamples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueHum;
      updateDaySample=false;
    }

    //Updating JSON object with samples - Used in web webEvents
    char s[100];getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%d/%m/%Y - %H:%M:%S",&nowTimeInfo);
    samples["CO2"] = String(valueCO2);
    samples["temperature"] = String(valueT);
    samples["humidity"] =  String(valueHum);
    samples["dateUpdate"] =  String(s);
    
    // If there's web client connected, send Events it with the new readings
    if (wifiEnabled && webServerEnabled && WiFi.status()==WL_CONNECTED && webEvents.count()>0) {
      webEvents.send("ping",NULL,nowTimeGlobal);
      webEvents.send(JSON.stringify(samples).c_str(),"new_samples",nowTimeGlobal);
      if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  - SAMPLE_PERIOD - new SSE sample sent: "+JSON.stringify(samples));
    }

    // Publish MQTT message with the new samples
    if (wifiEnabled && mqttServerEnabled && WiFi.status()==WL_CONNECTED) {
      if (mqttClient.connected()) {
        //MQTT Client connected
        if (MqttSyncCurrentStatus!=MqttSyncOnStatus) MqttSyncCurrentStatus=MqttSyncOnStatus;
        if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  - SAMPLE_PERIOD - new MQTT messages published:\n    "+mqttTopicName+"/temperature "+String(valueT)+", packetId="+String(mqttClient.publish(String(mqttTopicName+"/temperature").c_str(), 0, true, String(valueT).c_str()))+
                      "\n    "+mqttTopicName+"/humidity "+String(valueHum)+", packetID="+String(mqttClient.publish(String(mqttTopicName+"/humidity").c_str(), 0, true, String(valueHum).c_str()))+
                      "\n    "+mqttTopicName+"/co2 "+String(valueCO2)+", packetId="+String(mqttClient.publish(String(mqttTopicName+"/co2").c_str(), 0, true, String(valueCO2).c_str())));
      }
      else {
        //MQTT Client disconnected
        if (MqttSyncCurrentStatus!=MqttSyncOffStatus) MqttSyncCurrentStatus=MqttSyncOffStatus;
        //Connect to MQTT broker
        if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  - SAMPLE_PERIOD - new MQTT messages can't be published as MQTT broker is disconnected. Trying to get connected again...");
        mqttClientInit(true,true,false);
      }
    }

    // Notify the new readings to the BLE client (if there's connections)
    if ( deviceConnected && bluetoothEnabled && BLEClurrentStatus!=BLEOffStatus && BLEDevice::getInitialized() ) {
      pCharacteristicCO2->setValue(valueCO2); pCharacteristicCO2->notify();
      pCharacteristicT->setValue(valueT); pCharacteristicT->notify();
      pCharacteristicHum->setValue(valueHum); pCharacteristicHum->notify();
    }

    if (forceGetSample) forceGetSample=false;

    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - SAMPLE_PERIOD - exit");}
  }

  //Regular actions every BLE_PERIOD seconds.
  //Init and send iBecon periodically
  //Doing that in the loop avoid to block huge amount of RAM that impacts in other processes (WEB server)
  nowTimeGlobal=loopStartTime+millis();
  if ( ((((nowTimeGlobal-lastTimeBLECheck) >= BLEPeriod) && (nowTimeGlobal>=lastTimeBLECheck)) || firstBoot) &&  bluetoothEnabled ) {  
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - BLE_PERIOD");}
    if (webServerResponding) {
      //To avoid heap leak don't start BLE as there's webServer actvitiy
      if(BLEDevice::getInitialized())
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - forceStop=1 - Exit - Adversiting ON");}
      else 
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - forceStop=1 - Exit - Adversiting OFF");}
    }
    else {
      //BLE init only if the following conditions are met:
      //  - Not already initiated
      //  - There's enough heap size in memory for the BLE stack
      //  - !webServerResponding avoid sending iBeacons if serving web pages - Avoid heap overflow
      //  - !button1.pressed() and !button2.pressed() - Avoid button acting getting slow
      //  - displayMode checks - Avoid button acting getting slow - Not allow in menus
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - displayMode="+String(displayMode)+", currentState="+String(currentState)+", heap="+String(esp_get_free_heap_size())+" B, minHeap="+String(minHeapSeen)+" B, webServerResponding="+String(webServerResponding));}
      long auxRandom=random(1,16); //random < 2 ==> probability ~6%
      if (!BLEDevice::getInitialized() && esp_get_free_heap_size()>=BLE_MIN_HEAP_SIZE && !webServerResponding && !button1.pressed() && !button2.pressed() && 
          (auxRandom<2 || currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) ) {
        BLEClurrentStatus=BLEOffStatus;
        isBeaconAdvertising=false;
        if (startBLEAdvertising()==0) {
        //if (setupBLE()==0) {
          BLEClurrentStatus=BLEOnStatus;
          isBeaconAdvertising=true;
          if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - Advertising ON");}
        } 
        else if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - Sending iBeacon failed. BLE disabled");}
      } 
      else if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - BLEDevice initiated, button pressed, wrong displayMode ("+String(displayMode)+") or webServerResponding "+String(webServerResponding)+" or NOT enough heap ("+String(esp_get_free_heap_size())+" B) to init BLEDevice. Required min "+BLE_MIN_HEAP_SIZE+" B.\n"+String(nowTimeGlobal)+"  - Advertising OFF");}
    }

    //Control variables setup
    previousLastTimeBLECheck=lastTimeBLECheck;
    lastTimeBLECheck=nowTimeGlobal; 
    lastTimeBLEOnCheck=nowTimeGlobal;
    
    if (debugModeOn) {Serial.print(String(nowTimeGlobal)+"  - BLE_PERIOD check ends, heap="+String(esp_get_free_heap_size())+" B, minHeap="+String(minHeapSeen)+" B,  ****** - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
  }

  //Checking BLE_ON_TIMEOUT - Switch BLE off if:
  // - It's ON, BLE is enabled and one of the following occurs:
  //   + timedout since swiched it on (last BLE_Period)
  //   + heap is being reducing 
  //   + webServerResponding - to release memory
  //   + buttons are pressed
  //   + Menu screens are displayed
  nowTimeGlobal=loopStartTime+millis();
  //if ( isBeaconAdvertising &&  bluetoothEnabled && BLEDevice::getInitialized() ) {
  if ( isBeaconAdvertising &&  bluetoothEnabled ) {
    //Doing some tasks during the time window that the BLE is active
    
    //Reset WatchDogTimer to avoid "Task watchdog got triggered" error if a HTML Request needs to be served
    rtc_wdt_feed();

    heapSizeNow=esp_get_free_heap_size();
    //heapSizeNow=40000;
    //Stop BLE Advertising if conditions are met
    //webServerResponding=true if serving a HTTPPage
    if ( ((nowTimeGlobal-lastTimeBLEOnCheck) >= BLEOnTimeout) || heapSizeNow<ABSULUTE_MIN_HEAP_THRESHOLD  ||
           webServerResponding || button1.pressed() || button2.pressed() ||
           (currentState!=displayingSampleFixed && currentState!=displayingCo2LastHourGraphFixed && 
           currentState!=displayingCo2LastDayGraphFixed && currentState!=displayingSequential ))
    {  
      //Switch BLE off
      //if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - BLE_ON_TIMEOUT, heap="+String(heapSizeNow)+" B, minHeap="+String(minHeapSeen)+" B, lastTimeBLEOnCheck="+String(lastTimeBLEOnCheck)+", BLEOnTimeout="+String(BLEOnTimeout)+", currentState="+String(currentState)+", deviceConnected="+String(deviceConnected)+", pServer->getConnectedCount()="+String(pServer->getConnectedCount()));}

      if (!(pServer->getConnectedCount()!=0 && !button1.pressed() && !button2.pressed() &&
            (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || 
            currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential))
            || webServerResponding || heapSizeNow<ABSULUTE_MIN_HEAP_THRESHOLD) {
        
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - BLE_ON_TIMEOUT, heap="+String(heapSizeNow)+" B, minHeap="+String(minHeapSeen)+" B, lastTimeBLEOnCheck="+String(lastTimeBLEOnCheck)+", BLEOnTimeout="+String(BLEOnTimeout)+", currentState="+String(currentState)+", deviceConnected="+String(deviceConnected)+", pServer->getConnectedCount()="+String(pServer->getConnectedCount()));}
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - pServer->getConnectedCount()="+String(pServer->getConnectedCount())+", deviceConnected="+String(deviceConnected));}
        if (webServerResponding && debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - webServerResponding="+String(webServerResponding)+" -  Stop BLE");}
        if ((heapSizeNow<ABSULUTE_MIN_HEAP_THRESHOLD) && debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - heapSizeNow<ABSULUTE_MIN_HEAP_THRESHOLD ("+String(ABSULUTE_MIN_HEAP_THRESHOLD)+" B) - Stop BLE");}
                
        //Stop Advertisings and release memory only if other device is not connected, but only if no other priority event occurs
        stopBLE();
        isBeaconAdvertising=false;
        lastTimeBLEOnCheck=nowTimeGlobal;

        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - Switch BLE off");}

        //Release more memory if needed
        /*if (heapSizeNow<ABSULUTE_MIN_HEAP_THRESHOLD) {
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+"  - Release more memory as heapSizeNow<ABSULUTE_MIN_HEAP_THRESHOLD");
          detachNetwork();
        }*/

        heapSizeNow=esp_get_free_heap_size();
        if (debugModeOn) {Serial.print(String(nowTimeGlobal)+"  - BLE_ON_TIMEOUT check ends, heap="+String(heapSizeNow)+" B, minHeap="+String(minHeapSeen)+" B,  ****** - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
      }
    }
  }

  //Regular actions every ICON_STATUS_REFRESH_PERIOD seconds.
  //Refresh WiFi status icon. It's supposed it doesn't consume too much energy and is inmediate
  //Only if the Display is ON, otherwise it doesn't make sense
  nowTimeGlobal=loopStartTime+millis();
  if (((nowTimeGlobal-lastTimeIconStatusRefreshCheck) >= ICON_STATUS_REFRESH_PERIOD) && !firstBoot &&
      HIGH==digitalRead(PIN_TFT_BACKLIGHT) ) {
    if (debugModeOn) {String(nowTimeGlobal)+Serial.println(String(nowTimeGlobal)+"  - ICON_STATUS_REFRESH_PERIOD");}

    lastTimeIconStatusRefreshCheck=nowTimeGlobal;  //Update at begining to prevent accumulating delays in CHECK periods as this code might take long

    if (WiFi.status()!=WL_CONNECTED) {
      //WiFi is not connected. Update wifiCurrentStatus properly
      if (wifiCurrentStatus!=wifiOffStatus) {
        wifiCurrentStatus=wifiOffStatus;
        forceWifiReconnect=true; //Don't wait next WIFI_RECONNECT_PERIOD interaction. Reconnect in this loop() interaction
      }
      else {
        //No need to update wifiCurrentStatus nor timers to avoid reconnection in this loop() interaction
        //Just wait next WIFI_RECONNECT_PERIOD interaction to WiFi reconnection
      }
    }
    else {
      //Take RSSI to update the WiFi icon
      if (WiFi.RSSI()>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
      else if (WiFi.RSSI()>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
      else if (WiFi.RSSI()>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
      else if (WiFi.RSSI()>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
      else if (WiFi.RSSI()<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
    }
  }
  
  //Regular actions every DISPLAY_MODE_REFRESH_PERIOD seconds. Selecting what's the screen to display (active screen)
  nowTimeGlobal=loopStartTime+millis();
  if ((((nowTimeGlobal-lastTimeDisplayModeCheck) >= DISPLAY_MODE_REFRESH_PERIOD) || forceDisplayModeRefresh) && 
       currentState==displayingSequential && digitalRead(PIN_TFT_BACKLIGHT)!=LOW) {
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - DISPLAY_MODE_REFRESH_PERIOD");}

    lastTimeDisplayModeCheck=nowTimeGlobal; //Update at begining to prevent accumulating delays in CHECK periods as this code might take long

    //forceDisplayModeRefresh=true if either:
    // 1) comming from menu WhatToDisplay, after change to displayingSequential mode
    // 2) comming from button1 to switch the Display ON
    // 3) USB is plug
    switch (displayMode) {
      case sampleValue:
        //if forceDisplayModeRefresh, then the first screen is always the sampleValue
        if (lastDisplayMode!=bootup && !forceDisplayModeRefresh) displayMode=co2LastHourGraph; //Next display mode only if coming from bootup screen
      break;
      case co2LastHourGraph:
        displayMode=co2LastDayGraph; //Next display mode
      break;
      case co2LastDayGraph:
        displayMode=sampleValue; //Next display mode
      break;
    }
  
    if (forceDisplayModeRefresh) forceDisplayModeRefresh=false;
  }

  //Regular actions every DISPLAY_REFRESH_PERIOD seconds. Display the active screen, but only if the Display is ON (to save energy consume)
  nowTimeGlobal=loopStartTime+millis();
  if ((((nowTimeGlobal-lastTimeDisplayCheck) >= DISPLAY_REFRESH_PERIOD) || forceDisplayRefresh) && digitalRead(PIN_TFT_BACKLIGHT)!=LOW && 
      (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || 
        currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) ) {
    
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - DISPLAY_REFRESH_PERIOD, currentState="+String(currentState)+", lastState="+String(lastState)+", forceDisplayRefresh="+String(forceDisplayRefresh)+", lastDisplayMode="+String(lastDisplayMode)+", displayMode="+String(displayMode));}
    lastTimeDisplayCheck=nowTimeGlobal; //Update at begining to prevent accumulating delays in CHECK periods as this code might take long

    switch (displayMode) {
      case  sampleValue:
        if (debugModeOn) {Serial.print("    - **** displayMode=sampleValue - Rendering only if new sample value - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
        //Sample values are displayed
        if ((nowTimeGlobal-previousLastTimeSampleCheck)>=samplePeriod || forceDisplayRefresh || firstBoot || lastDisplayMode!=sampleValue) {
          //Display refresh only when a new sample has been taken or during firstBoot

          //Cleaning the screen always the first time in
          if (lastDisplayMode!=sampleValue) {tft.fillScreen(TFT_BLACK); lastValueCO2=-1;} //Force re-rendering CO2 values in the main screen
          
          //Drawing Icons
          showIcons();

          //Cleaning & Drawing circular gauge
          tft.setTextSize(TEXT_SIZE);
          if ( lastValueCO2<0 || abs(1-lastValueCO2/valueCO2)*100>10 ||
              (lastValueCO2<circularGauge.th1 && valueCO2>=circularGauge.th1) ||
              (lastValueCO2>=circularGauge.th1 && valueCO2<circularGauge.th1) ||
              (lastValueCO2>circularGauge.th1 && lastValueCO2<circularGauge.th2 && valueCO2>circularGauge.th2) ||
              (lastValueCO2>=circularGauge.th2 && valueCO2<circularGauge.th2))
          {
            //First cleaning all the of items to avoid partial deletion of the horizontal bar
            tft.fillRect(horizontalBar.xStart,horizontalBar.yStart-tft.fontHeight(TEXT_FONT)-1,horizontalBar.width,tft.fontHeight(TEXT_FONT),TFT_BLACK);
            horizontalBar.cleanHorizontalBar();
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String("Hum.")),90,tft.textWidth(String("Hum.")),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            valueString=String("----");
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(valueString),90+tft.fontHeight(TEXT_FONT),tft.textWidth(valueString+"%"),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            circularGauge.cleanAll();
            
            //Drawing the circularGauge
            circularGauge.drawGauge2(valueCO2); lastValueCO2=valueCO2;
          }
          else {
            //First cleaning all bar and text items to before re-drawing them
            tft.fillRect(horizontalBar.xStart,horizontalBar.yStart-tft.fontHeight(TEXT_FONT)-1,horizontalBar.width,tft.fontHeight(TEXT_FONT),TFT_BLACK);
            horizontalBar.cleanHorizontalBar();
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String("Hum.")),90,tft.textWidth(String("Hum.")),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String(round(valueHum))+"%"),90+tft.fontHeight(TEXT_FONT),tft.textWidth(String(round(valueHum))+"%"),tft.fontHeight(TEXT_FONT),TFT_BLACK);

            //Partial displaying of the circular gauge (only the text value) to don't take too much displaying it
            circularGauge.setValue(valueCO2);
            circularGauge.cleanValueTextGauge();
            circularGauge.cleanUnitsTextGauge();
          }
          circularGauge.drawTextGauge("ppm");

          //Drawing temperature
          valueString=roundFloattoString(valueT,1)+"C";
          tft.setTextSize(TEXT_SIZE);
          drawText(valueT, String(valueString),TEXT_SIZE,TEXT_FONT,TFT_GREEN,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),25,19.95,TFT_BLUE,27.05,TFT_RED);
          horizontalBar.drawHorizontalBar(valueT);

          //Drawing Humidity
          valueString=String("Hum.");
          tft.setTextSize(TEXT_SIZE);
          drawText(valueHum,String(valueString),TEXT_SIZE,TEXT_FONT,TFT_MAGENTA,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),90,30,TFT_MAGENTA,55,TFT_MAGENTA);
          valueString=String(int(round(valueHum)))+"%";
          drawText(valueHum,String(valueString),TEXT_SIZE,TEXT_FONT,TFT_MAGENTA,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),90+tft.fontHeight(TEXT_FONT),30,TFT_BROWN,55,TFT_RED);
        
        }
        lastDisplayMode=sampleValue;
      break;
      case co2LastHourGraph:
        if (debugModeOn) {Serial.print("    - **** displayMode=co2LastHourGraph - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
        //Last Hour graph is displayed
        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastHourGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastHourCo2(); updateHourGraph=false;}
        else if (updateHourGraph) {drawGraphLastHourCo2(); updateHourGraph=false;}//Draw new graph only if buffer was updated

        lastDisplayMode=co2LastHourGraph;
      break;
      case co2LastDayGraph:
        if (debugModeOn) {Serial.print("    - **** displayMode=co2LastDayGraph - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
        //Last Day graph is displayed
        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastDayGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastDayCo2(); updateDayGraph=false;}
        else if (updateDayGraph) {drawGraphLastDayCo2(); updateDayGraph=false;}//Draw new graph only if buffer was updated
      
        lastDisplayMode=co2LastDayGraph;
      break;
    }   

    if (forceDisplayRefresh) forceDisplayRefresh=false;
  }

  //Regular actions every WIFI_RECONNECT_PERIOD seconds to recover WiFi connection
  //forceWifiReconnect==true if:
  // 1) after ICON_STATUS_REFRESH_PERIOD
  // 2) after configuring WiFi = ON in the Config Menu
  // 3) or wake up from sleep (either by pressing buttons or timer)
  // 4) or previous WiFi re-connection try was ABORTED (button pressed) or BREAK (need to refresh display)
  // 5) after heap size was below ABSULUTE_MIN_HEAP_THRESHOLD
  nowTimeGlobal=loopStartTime+millis();
  if ((((nowTimeGlobal-lastTimeWifiReconnectionCheck) >= WIFI_RECONNECT_PERIOD) || forceWifiReconnect ) && 
      wifiEnabled && !firstBoot && (wifiCurrentStatus==wifiOffStatus || WiFi.status()!=WL_CONNECTED) ) {
     
    if (debugModeOn) {
      Serial.println(String(nowTimeGlobal)+"  - nowTimeGlobal-lastTimeWifiReconnectionCheck >= WIFI_RECONNECT_PERIOD ("+String(nowTimeGlobal-lastTimeWifiReconnectionCheck)+" >= "+String(WIFI_RECONNECT_PERIOD/1000)+" s)");
      Serial.println("    - lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));
      Serial.println("    - forceWifiReconnect="+String(forceWifiReconnect));
      Serial.println("    - forceWebServerInit="+String(forceWebServerInit));
      Serial.println("    - !firstBoot="+String(!firstBoot));
      Serial.println("    - wifiCurrentStatus="+String(wifiCurrentStatus)+", wifiOffStatus=0");
      Serial.println("    - WiFi.status()="+String(WiFi.status())+", WL_CONNECTED=3");
    }

    //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
    if (!wifiResuming) lastTimeWifiReconnectionCheck=nowTimeGlobal; //Only if the WiFi reconnection didn't ABORT or BREAK in the previous interaction
    
    wifiStatus previousWifiCurrentStatus=wifiCurrentStatus;
    if(forceWifiReconnect) {
      forceWifiReconnect=false;
      forceNTPCheck=true; //v0.9.9 - Force NTP sync after WiFi Connection
    }
    
    //If WiFi disconnected (wifiOffStatus), then re-connect
    //Conditions for wifiCurrentStatus==wifiOffStatus
    // - no found SSID in init()
    // - no found SSID when ICON_STATUS_REFRESH_PERIOD
    //WiFi.status() gets the WiFi status inmediatly. No need to scann WiFi networks
  
    //WiFi Reconnection
    if (debugModeOn) {Serial.println("    - auxLoopCounter="+String(auxLoopCounter)+", auxCounter="+String(auxCounter));}
    
    forceWEBTestCheck=false; //If WiFi reconnection is successfull, then check CLOUD server to update ICON. Decision is done below, if NO_ERROR
    switch(wifiConnect(false,false,true,&auxLoopCounter,&auxCounter)) {
      case ERROR_ABORT_WIFI_SETUP: //Button pressed. WiFi reconnection aborted. Exit now but force reconnection next loop interaction
        wifiCurrentStatus=previousWifiCurrentStatus;
        forceWifiReconnect=true; //Force WiFi reconnection in the next loop interaction
        wifiResuming=true;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - wifiConnect() finish with ERROR_ABORT_WIFI_SETUP. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWifiReconnect="+String(forceWifiReconnect));}
      break;
      case ERROR_BREAK_WIFI_SETUP: //Time to refresh display. Exit now but force reconnection next loop interaction
        wifiCurrentStatus=previousWifiCurrentStatus;
        forceWifiReconnect=true; //Force WiFi reconnection in the next loop interaction
        wifiResuming=true;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - wifiConnect() finish with ERROR_BREAK_WIFI_SETUP. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWifiReconnect="+String(forceWifiReconnect));}
      break;
      case ERROR_WIFI_SETUP:
        wifiCurrentStatus=wifiOffStatus;
        forceWifiReconnect=false;
        wifiResuming=false;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - wifiConnect() finish with ERROR_WIFI_SETUP. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWifiReconnect="+String(forceWifiReconnect));}
      break;
      case NO_ERROR:
      default:
        if (WiFi.RSSI()>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
        else if (WiFi.RSSI()>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
        else if (WiFi.RSSI()>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
        else if (WiFi.RSSI()>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
        else if (WiFi.RSSI()<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
        if (forceWifiReconnect) forceNTPCheck=true; //v0.9.9 - Force NTP sync after WiFi Connection
        forceWifiReconnect=false;
        wifiResuming=false;
        //Send HttpRequest to check the server status
        // The request updates CloudSyncCurrentStatus
        forceWEBTestCheck=true; //Will check CLOUD server in the next loop() interaction
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - wifiConnect() finish with NO_ERROR. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWEBTestCheck="+String(forceWEBTestCheck));}
      break;
    } 

    if (WiFi.status()==WL_CONNECTED && !mqttClient.connected() && mqttServerEnabled) { //Connect to MQTT broker again
      //mqttClient.connect();
      mqttClientInit(true,true,false);
    }

    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - WIFI_RECONNECT_PERIOD - exit, lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));}
  }

  //After getting WiFi connection re-init the web server if needed
  //forceWebServerInit==true if:
  // 1) wake up from sleep, including hibernate (either by pressing buttons or timer)
  // 2) WiFi set ON from the config menu
  // 3) after heap size was below ABSULUTE_MIN_HEAP_THRESHOLD
  if (wifiEnabled && webServerEnabled && WiFi.status()==WL_CONNECTED && forceWebServerInit) { //v0.9.9 - Re-init the built-in WebServer after waking up from sleep
    if (debugModeOn) Serial.println("    - After leaving WIFI_RECONNECT_PERIOD entering to re-init the Web Server");    
    if(SPIFFS.begin(true)) {
      if (debugModeOn) Serial.println("    - wifiConnect()  - SPIFFS.begin() OK, SPIFFSErrors="+String(SPIFFSErrors));
      initWebServer();
      forceWebServerInit=false;
      if (debugModeOn) Serial.println("    - wifiConnect()  - initWebServer");
      
      fileSystemSize = SPIFFS.totalBytes();
      fileSystemUsed = SPIFFS.usedBytes();
    }
    else {
      SPIFFSErrors++;
      if (debugModeOn) Serial.println("    - wifiConnect()  - SPIFFS.begin() KO, SPIFFSErrors="+String(SPIFFSErrors));
    }
  }
  
  //Regular actions every NTP_KO_CHECK_PERIOD seconds. Cheking if NTP is off or should be checked
  //forceNTPCheck is true if:
  // 1) After NTP server config in firstSetup()
  // 2) If the previous NTP check was aborted due eithe Button action or Display Refresh
  // 2) WiFi has been setup ON in config menu
  nowTimeGlobal=loopStartTime+millis();
  if ((nowTimeGlobal-lastTimeNTPCheck) >= NTP_KO_CHECK_PERIOD || forceNTPCheck) {
    
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"  - NTP_KO_CHECK_PERIOD, last lastTimeNTPCheck="+String(lastTimeNTPCheck)+", NTPResuming="+String(NTPResuming)+", auxLoopCounter2="+String(auxLoopCounter2)+", whileLoopTimeLeft="+String(whileLoopTimeLeft));}
    
    //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
    if (!NTPResuming) lastTimeNTPCheck=nowTimeGlobal; //Only if the NTP reconnection didn't ABORT or BREAK in the previous interaction

    //setupNTPConfig if either
    // - Last NTP check failed (and currently there is no NTP sycn) - It's done always (in any Energy Mode)
    // - If in Full Energy Mode (USB power) and WiFI connected:
    //   + Every 6 hours in avarage per day (probability ~ 17% ==> prob= random(1,7)<2) 
    // - If in either Reduce or Save Energy Mode, everytime there's WiFi connecton to minimize the
    //     time drift due to Deep Sleep (0,337 s per Deep Sleep min).
    //   + In average, every WIFI_RECONNECT_PERIOD=300000 (5 min)
    // - If the previous NTP check was aborted due to Button action (forceNTPCheck=true)
    
    long auxRandom=random(1,7);
    if( wifiCurrentStatus!=wifiOffStatus && wifiEnabled &&
        ( CloudClockCurrentStatus==CloudClockOffStatus || forceNTPCheck ||
          (fullEnergy==energyCurrentMode && (auxRandom<2)) ||
           reducedEnergy==energyCurrentMode || 
           lowestEnergy==energyCurrentMode ) ) {
      if (debugModeOn) {
        Serial.println("      - setupNTPConfig() for NTP Sync");
        if (CloudClockCurrentStatus==CloudClockOffStatus) Serial.println("        - Reason: CloudClockCurrentStatus==CloudClockOffStatus");
        if (forceNTPCheck) Serial.println("        - Reason: forceNTPCheck");
        if (fullEnergy==energyCurrentMode && (auxRandom<2)) Serial.println("        - Reason: fullEnergy==energyCurrentMode && (auxRandom(="+String(auxRandom)+")<2)");
        if (reducedEnergy==energyCurrentMode) Serial.println("        - Reason: reducedEnergy==energyCurrentMode");
        if (lowestEnergy==energyCurrentMode) Serial.println("        - Reason: lowestEnergy==energyCurrentMode");
      }
      forceNTPCheck=false;
      switch(setupNTPConfig(false,&auxLoopCounter2,&whileLoopTimeLeft)) { //NTP Sync and CloudClockCurrentStatus update
        case ERROR_ABORT_NTP_SETUP: //Button pressed. NTP reconnection aborted. Exit now but force reconnection next loop interaction 
          forceNTPCheck=true; //Force NTP reconnection in the next loop interaction
          NTPResuming=true;
          if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - setupNTPConfig() finish with ERROR_ABORT_NTP_SETUP. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
        break;
        case ERROR_BREAK_NTP_SETUP: //Button pressed. NTP reconnection aborted. Exit now but force reconnection next loop interaction
          forceNTPCheck=true; //Force NTP reconnection in the next loop interaction
          NTPResuming=true;
          if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - setupNTPConfig() finish with ERROR_BREAK_NTP_SETUP. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
        break;
        case ERROR_NTP_SERVER:
          forceNTPCheck=false;
          NTPResuming=false;
          if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - setupNTPConfig() finish with ERROR_NTP_SERVER. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
        break;
        case NO_ERROR:
          forceNTPCheck=false;
          NTPResuming=false;
          if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - setupNTPConfig() finish with NO_ERROR. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
        break;
      }
    }
    else
      if( wifiCurrentStatus==wifiOffStatus || !wifiEnabled) if (forceNTPCheck) forceNTPCheck=false; //v0.9.9 If no WiFi, don't enter in NTP_KO_CHECK_PERIOD even if it was BREAK or ABORT in previous intercation
    
    if (debugModeOn) {
      Serial.println(String(loopStartTime+millis())+"      - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      getLocalTime(&nowTimeInfo);Serial.print("  - NTP_KO_CHECK_PERIOD, lastTimeNTPCheck="+String(lastTimeNTPCheck));Serial.println(&nowTimeInfo," - Exit - Time: %d/%m/%Y - %H:%M:%S");
    }
  }
  
  //Regular actions every UPLOAD_SAMPLES_PERIOD seconds - Upload samples to external server
  //wifiCred.SiteAllow[wifiCred.activeIndex] must be set, otherwise samples are not uploaded
  //forceWEBCheck is set when in the previous interaction the WEB connection was either:
  // ABORTED (Button Pressed) or
  // BREAK  (Display Refresh)
  //This way it resume WEB connection and the Display or Buttons can be monitorized
  //forceWEBTestCheck is set if WiFi is recovered from disconnection in order to force WEB
  // server check and ICON update. If test overlaps the uploadSamplesPeriod timing, the later
  // has higher priority in order to send the samples on time.
  nowTimeGlobal=loopStartTime+millis();
  if ((((nowTimeGlobal-lastTimeUploadSampleCheck) >= uploadSamplesPeriod) || firstBoot || forceWEBCheck || forceWEBTestCheck) && uploadSamplesEnabled &&
      wifiCurrentStatus!=wifiOffStatus && wifiEnabled && (forceWEBTestCheck || wifiCred.SiteAllow[wifiCred.activeIndex]) ) {
      //(0==wifiCred.wifiSITEs[wifiCred.activeIndex].compareTo(UPLOAD_SAMPLES_FROM_SITE)) ) {
    
    //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
    if (!webResuming && !forceWEBTestCheck) lastTimeUploadSampleCheck=nowTimeGlobal; //Only if the HTTP connection didn't ABORT or BREAK in the previous interaction
    
    if (forceWEBTestCheck && ((nowTimeGlobal-lastTimeUploadSampleCheck) >= uploadSamplesPeriod))
     {forceWEBTestCheck=false;} //Timer has priority over WEB test. httpRequest will be setup to send samples 

    if (debugModeOn) {Serial.print(String(nowTimeGlobal)+"  - UPLOAD_SAMPLES_PERIOD - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
    
    String httpRequest=String(GET_REQUEST_TO_UPLOAD_SAMPLES);

    //In Reduce or Save Energy Modes, there were no battery updates to save energy consume, 
    //so let's update battery parameters now before uploading the httpRequest
    if (reducedEnergy==energyCurrentMode || lowestEnergy==energyCurrentMode) {
      //batADCVolt update
      //Power state check and powerState update
      //batteryStatus update
      //Saving energy Mode update (energyCurrentMode)
      //Auto Display Switch Off update (autoBackLightOff)
      //If USB is plugged, timeUSBPowerGlobal is updated with nowTimeGlobal, to estimate batteryCharge based on time
      //If USB is unplugged, timeUSBPowerGlobal is set to zero
      updateBatteryVoltageAndStatus(nowTimeGlobal, &timeUSBPowerGlobal);
    } 

    //GET /lar-co2/?device=co2-sensor&local_ip_address=192.168.100.192&co2=543&temp_by_co2_sensor=25.6&hum_by_co2_sensor=55&temp_co2_sensor=28.7
    if (forceWEBTestCheck) {httpRequest=httpRequest+"test HTTP/1.1";forceWEBTestCheck=false;}
    else httpRequest=httpRequest+"device="+device+"&local_ip_address="+
      IpAddress2String(WiFi.localIP())+"&version="+String(VERSION)+
      "&co2="+valueCO2+"&temp_by_co2_sensor="+valueT+"&hum_by_co2_sensor="+valueHum+
      "&temp_co2_sensor="+co2Sensor.getTemperature(true,true)+"&powerState="+powerState+
      "&batADCVolt="+batADCVolt+"&batCharge="+batCharge+"&batteryStatus="+batteryStatus+
      "&energyCurrentMode="+energyCurrentMode+"&errorsWiFiCnt="+errorsWiFiCnt+
      "&errorsSampleUpts="+errorsSampleUpts+"&errorsNTPCnt="+errorsNTPCnt+"&webServerError1="+webServerError1+
      "&webServerError2="+webServerError2+"&webServerError3="+webServerError3+"&SPIFFSErrors="+SPIFFSErrors+
      "&heapSize="+String(esp_get_free_heap_size())+" HTTP/1.1";

    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - serverToUploadSamplesIPAddress="+IpAddress2String(serverToUploadSamplesIPAddress)+", SERVER_UPLOAD_PORT="+String(SERVER_UPLOAD_PORT)+
                   ", httpRequest="+String(httpRequest));}

    forceWEBCheck=false;
    switch(sendAsyncHttpRequest(false,false,0,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,httpRequest,&whileWebLoopTimeLeft)) { //Sending http request and CloudSyncCurrentStatus update
      case ERROR_ABORT_WEB_SETUP: //Button pressed. NTP connection aborted. Exit now but force reconnection next loop interaction 
        forceWEBCheck=true; //Force WEB reconnection in the next loop interaction
        webResuming=true;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - sendAsyncHttpRequest() finish with ERROR_ABORT_WEB_SETUP. CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", forceWEBCheck="+String(forceWEBCheck));}
      break;
      case ERROR_BREAK_WEB_SETUP: //Button pressed. WEB connection aborted. Exit now but force reconnection next loop interaction
        forceWEBCheck=true; //Force WEB reconnection in the next loop interaction
        webResuming=true;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - sendAsyncHttpRequest() finish with ERROR_BREAK_WEB_SETUP. CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", forceWEBCheck="+String(forceWEBCheck));}
      break;
      case ERROR_CLOUD_SERVER:
        forceWEBCheck=false;
        webResuming=false;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - sendAsyncHttpRequest() finish with ERROR_CLOUD_SERVER. CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", forceWEBCheck="+String(forceWEBCheck));}
      break;
      case NO_ERROR:
        forceWEBCheck=false;
        webResuming=false;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - sendAsyncHttpRequest() finish with NO_ERROR. CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", forceWEBCheck="+String(forceWEBCheck));}
      break;
    }
  
    if (debugModeOn) {Serial.print(String(loopStartTime+millis())+"  - UPLOAD_SAMPLES_PERIOD - Exit - Time: ");getLocalTime(&nowTimeInfo);Serial.print(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");Serial.println(", lastTimeUploadSampleCheck="+String(lastTimeUploadSampleCheck));}
  }

  if (firstBoot) firstBoot=false;
  if (buttonWakeUp) buttonWakeUp=false;

  //Go to hibernate if battery is ran out
  if (batCharge<=BAT_CHG_THR_TO_HIBERNATE && onlyBattery==powerState) go_to_hibernate();

  //Going to sleep, but only if the display if OFF and not there's Battery power
  //if (LOW==digitalRead(PIN_TFT_BACKLIGHT)) go_to_sleep(); //For testing in sleep mode
  //if (LOW==digitalRead(PIN_TFT_BACKLIGHT) && energyCurrentMode!=fullEnergy) go_to_sleep();
  if (LOW==digitalRead(PIN_TFT_BACKLIGHT) && onlyBattery==powerState) go_to_sleep();
}