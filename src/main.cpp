#ifndef WIFIFRAMEWORK
  #define WIFIFRAMEWORK
#endif

#ifndef _DECLAREGLOBALPARAMETERS_
  #define _DECLAREGLOBALPARAMETERS_
#endif

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "global_setup.h"
#include "wifiConnection.h"
#include "display_support.h"
#include "MHZ19.h"
#include <SoftwareSerial.h>
#include "Button.h"
#include "SHT2x.h"
#include "ButtonChecks.h"
#include "time.h"
#include "httpClient.h"
#include "icons.h"
#include "battery.h"
//#include "esp_wifi.h"
#include "esp_sntp.h"
#include "misc.h"
#include <ESP32Time.h>
#include <EEPROM.h>

#ifdef __MHZ19B__
  const ulong co2PreheatingTime=MH_Z19B_CO2_WARMING_TIME;
#endif

//Global variable definitions stored in RTC memory to be persistent during Sleep periods. 8 KB Max
// RTC memory Arrays:
//           3* (4*3600/60 = 240 B)     =  720 B
//           3* (4*24*3600/450 = 768 B) = 2304 B
// RTC memory variables in global_setup:    56 B
// RTC memory variables:                   763 B
// ----------------------------------------------
// RTC memorty TOTAL:                     3819 B
// RTC memory left:     8000 B - 3819 B = 4181 B
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
//Address 09-0C: Stores float_t batCharge - 4 B

RTC_DATA_ATTR float_t lastHourCo2Samples[3600/SAMPLE_T_LAST_HOUR];   //4*(3600/60)=240 B - Buffer to record last-hour C02 values
RTC_DATA_ATTR float_t lastHourTempSamples[3600/SAMPLE_T_LAST_HOUR];  //4*(3600/60)=240 B - Buffer to record last-hour Temp values
RTC_DATA_ATTR float_t lastHourHumSamples[3600/SAMPLE_T_LAST_HOUR];   //4*(3600/60)=240 B - Buffer to record last-hour Hum values
RTC_DATA_ATTR float_t lastDayCo2Samples[24*3600/SAMPLE_T_LAST_DAY];  //4*(24*3600/450)=768 B - Buffer to record last-day C02 values
RTC_DATA_ATTR float_t lastDayTempSamples[24*3600/SAMPLE_T_LAST_DAY]; //4*(24*3600/450)=768 B - Buffer to record last-day Temp values
RTC_DATA_ATTR float_t lastDayHumSamples[24*3600/SAMPLE_T_LAST_DAY];  //4*(24*3600/450)=768 B - Buffer to record last-day Hum values
RTC_DATA_ATTR boolean firstBoot=true;  //1B - First boot flag.
RTC_DATA_ATTR uint64_t nowTimeGlobal=0,timeUSBPowerGlobal=0,loopStartTime=0,loopEndTime=0,
                        lastTimeSampleCheck=0,previousLastTimeSampleCheck=0, lastTimeDisplayCheck=0,lastTimeDisplayModeCheck=0,lastTimeNTPCheck=0,lastTimeVOLTCheck=0,
                        lastTimeHourSampleCheck=0,lastTimeDaySampleCheck=0,lastTimeUploadSampleCheck=0,lastTimeIconStatusRefreshCheck=0,
                        lastTimeTurnOffBacklightCheck=0,lastTimeWifiReconnectionCheck=0; //16*8=128 B
RTC_DATA_ATTR ulong voltageCheckPeriod,samplePeriod,uploadSamplesPeriod; //3*4=12B
RTC_DATA_ATTR uint64_t sleepTimer=0; //8 B
RTC_DATA_ATTR enum displayModes displayMode=bootup,lastDisplayMode=bootup; //2*4=8 B
RTC_DATA_ATTR enum availableStates stateSelected=displayingSampleFixed,currentState=bootupScreen,lastState=currentState; //3*4=12 B
RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus; //4 B
RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus; // 4B
RTC_DATA_ATTR boolean updateHourSample=true,updateDaySample=true,updateHourGraph=true,updateDayGraph=true,
              autoBackLightOff=true,button1Pressed=false,button2Pressed=false; //8*1=8 B 
RTC_DATA_ATTR enum powerModes powerState=off; //1*4=4 B
RTC_DATA_ATTR enum batteryChargingStatus batteryStatus=battery000; //1*4=4 B
RTC_DATA_ATTR enum energyModes energyCurrentMode,configSavingEnergyMode; //2*4=8 B
RTC_DATA_ATTR uint8_t bootCount=0,loopCount=0; //2*1=2 B
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
#ifndef NTP_TZ_ENV_VARIABLE
  RTC_DATA_ATTR const long gmtOffset_sec=GMT_OFFSET_SEC; //4 B
  RTC_DATA_ATTR const int daylightOffset_sec=DAYLIGHT_OFFSET_SEC; //4 B
#endif
RTC_DATA_ATTR char TZEnvVar[50]="\0"; //50 B Should be enough - To back Time Zone Variable up
RTC_DATA_ATTR struct tm startTimeInfo; //36 B
RTC_DATA_ATTR boolean firstWifiCheck=true,forceWifiReconnect=false,forceGetSample=false,forceGetVolt=false,
        forceDisplayRefresh=false,forceDisplayModeRefresh=false,forceNTPCheck=false,buttonWakeUp=false,
        forceWEBCheck=false,forceWEBTestCheck=false; // 11*1=11 B
RTC_DATA_ATTR int uploadServerIPAddressOctectArray[4]; // 4*4B = 16B - To store upload server's @IP
RTC_DATA_ATTR byte mac[6]; //6*1=6B - To store WiFi MAC address
RTC_DATA_ATTR float_t valueCO2,valueT,valueHum=0,lastValueCO2=-1,tempMeasure; //5*4=20B
RTC_DATA_ATTR int errorsWiFiCnt=0,errorsSampleUpts=0,errorsNTPCnt=0,webServerError1=0,webServerError2=0,webServerError3=0; //2*4=8B - Error stats
RTC_DATA_ATTR boolean wifiEnabled,bluetoothEnabled,uploadSamplesEnabled; //3*1=3B
RTC_DATA_ATTR boolean debugModeOn=DEBUG_MODE_ON; //1*1=1B
RTC_DATA_ATTR enum BLEStatus BLEClurrentStatus=BLEOffStatus; //1*4=4B

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
uint32_t error_setup=NO_ERROR,remainingBootupSeconds=0;
int16_t cuX,cuY;
TFT_eSprite stext1 = TFT_eSprite(&tft); // Sprite object stext1
ulong previousTime=0,timePressButton1,timeReleaseButton1,timePressButton2,timeReleaseButton2,
      remainingBootupTime=BOOTUP_TIMEOUT*1000;
String valueString;
String serverToUploadSamplesString(SERVER_UPLOAD_SAMPLES);
IPAddress serverToUploadSamplesIPAddress; //8 B
String device(DEVICE_NAME_PREFIX); //16 B
static const char hex_digits[] = "0123456789ABCDEF";
boolean waitingMessage=true,runningMessage=true,wifiResuming=false,NTPResuming=false,webResuming=false;
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


//Code

void initVariable() {
  //Global variables init. Needed as they have random values after wakeup from hiberte mode
  firstBoot=true;
  nowTimeGlobal=0;timeUSBPowerGlobal=0;loopStartTime=0;loopEndTime=0;
  lastTimeSampleCheck=0;previousLastTimeSampleCheck=0; lastTimeDisplayCheck=0;lastTimeDisplayModeCheck=0;
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
  bootCount=0;loopCount=0;
  circularGauge=CircularGauge(0,0,CO2_GAUGE_RANGE,CO2_GAUGE_X,CO2_GAUGE_Y,CO2_GAUGE_R,
                            CO2_GAUGE_WIDTH,CO2_GAUGE_SECTOR,TFT_DARKGREEN,
                            CO2_GAUGE_TH1,TFT_YELLOW,CO2_GAUGE_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);
  horizontalBar=HorizontalBar(0,TEMP_BAR_MIN,TEMP_BAR_MAX,TEMP_BAR_X,TEMP_BAR_Y,
                              TEMP_BAR_LENGTH,TEMP_BAR_HEIGH,TFT_GREEN,TEMP_BAR_TH1,
                              TFT_BLUE,TEMP_BAR_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);
  firstWifiCheck=true;forceWifiReconnect=false;forceGetSample=false;forceGetVolt=false;
  forceDisplayRefresh=false;forceDisplayModeRefresh=false;forceNTPCheck=false;buttonWakeUp=false;
  forceWEBCheck=false;forceWEBTestCheck=false;
  valueCO2=0;valueT=0;valueHum=0;lastValueCO2=-1;tempMeasure=0;
  errorsWiFiCnt=0;errorsSampleUpts=0;errorsNTPCnt=0;webServerError1=0;webServerError2=0;webServerError3=0;
  error_setup=NO_ERROR;remainingBootupSeconds=0;
  previousTime=0;remainingBootupTime=BOOTUP_TIMEOUT*1000;
  static const char hex_digits[] = "0123456789ABCDEF";
  waitingMessage=true;runningMessage=true;
  auxLoopCounter=0;auxLoopCounter2=0;auxCounter=0;
  whileLoopTimeLeft=NTP_CHECK_TIMEOUT;whileWebLoopTimeLeft=HTTP_ANSWER_TIMEOUT;
  wifiResuming=false;NTPResuming=false;webResuming=false;
  debugModeOn=DEBUG_MODE_ON;
  BLEClurrentStatus=BLEOffStatus;
  
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
  //Address 09-0C: Stores float_t batCharge - 4 B

  //Check if it is the first run after the very first firmware upload
  for (int i=0; i<VERSION_CHAR_LENGTH; i++) firmwareVersion[i]=EEPROM.read(i);firmwareVersion[VERSION_CHAR_LENGTH]='\0';
  uint16_t readChecksum,computedChecksum;
  readChecksum=EEPROM.read(7);readChecksum=readChecksum<<8;readChecksum|=EEPROM.read(6);
  computedChecksum=checkSum((byte*)firmwareVersion,VERSION_CHAR_LENGTH);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [initVariable] - firmwareVersion="+String(firmwareVersion)+", readChecksums="+String(readChecksum)+", computedChecksum="+String(computedChecksum));}
  
  if (readChecksum!=computedChecksum) {
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

    wifiEnabled=WIFI_ENABLED;
    bluetoothEnabled=BLE_ENABLED;
    uploadSamplesEnabled=UPLOAD_SAMPLES_ENABLED;
    configSavingEnergyMode=reducedEnergy; //Default value
    
    //Now initialize configVariables
    configVariables=0x01; //Bit 0, notFirstRun=true
    if (reducedEnergy==configSavingEnergyMode) configVariables|=0x02; //Bit 1: configSavingEnergyMode
    if (uploadSamplesEnabled) configVariables|=0x04; //Bit 2: uploadSamplesEnabled
    if (bluetoothEnabled) configVariables|=0x08; //Bit 3: bluetoothEnabled
    if (wifiEnabled) configVariables|=0x10; //Bit 4: wifiEnabled

    //Write in EEPROM to be available the next boots up
    EEPROM.write(0x08,configVariables);
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
  }
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [initVariable] - configVariables="+String(configVariables));}

}

void firstSetup() {
  static uint32_t wr = 1;
  static uint32_t rd = 0xFFFFFFFF;
  currentState=bootupScreen;lastState=currentState;
  displayMode=bootup;lastDisplayMode=bootup;

  if (logsOn) {Serial.print("\n[SETUP] - Doing regular CO2 bootup v");Serial.print(VERSION);Serial.println(" ..........");Serial.println("[setup] - Serial: OK");}

  //Date initialization to 1-Jan-2022 00:00:00
  // Necessary to avoid get frozen when getLocalTime() is called if NTP is not synced.
  // https://stackoverflow.com/questions/72940013/why-getlocaltime-implementation-needs-delay
  ESP32Time timeTest(0);
  timeTest.setTime(0,0,0,1,1,2022);  // 1st Jan 2022 00:00:00
  if (logsOn) {Serial.println("[setup] - Initial date set to 1st Jan 2022 00:00:00");}
  error_setup=NO_ERROR;

  //Display init
  pinMode(PIN_TFT_BACKLIGHT,OUTPUT); 
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

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
  
  delay(500);
  //Check out the TFT display
  tft.drawPixel(30,30,wr);
  rd = tft.readPixel(30,30);
  if (rd!=wr) {
    error_setup=ERROR_DISPLAY_SETUP;
    if (logsOn) {
      Serial.println("[setup] - Display: KO");
      Serial.println("          Pixel value wrote = ");Serial.println(wr,HEX);
      Serial.println("          Pixel value read  = ");Serial.println(rd,HEX);
      Serial.println("          Can't continue");
      //-->> Setup LED for error indication
    }
    return; //Can't continue
  }
  else {
    if (logsOn) Serial.println("[setup] - Display: OK");
    tft.fillScreen(TFT_BLACK);
  }
  
  //-->loadAllIcons();
  //-->loadAllWiFiIcons();

  loadBootImage();
  delay(500);
  //Display messages
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_WHITE,TFT_BLACK);stext1.print("CO2 bootup v");stext1.print(VERSION);stext1.println(" ..........");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Display: [");stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  
  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Some Temp & Hum sensor checks and init
  if (logsOn) Serial.print("[setup] - Sensor Temp/HUM: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Tp/Hu:  [");
  pinMode(SI7021_SDA,INPUT_PULLUP); pinMode(SI7021_SCL,INPUT_PULLUP);
  tempHumSensor.begin(SI7021_SDA,SI7021_SCL);

  int errorSns = tempHumSensor.getError();
  uint8_t statSns = tempHumSensor.getStatus();

  if (!tempHumSensor.isConnected() || 0==tempHumSensorType.compareTo("UNKNOW"))
    error_setup|=ERROR_SENSOR_TEMP_HUM_SETUP;
  if ((error_setup & ERROR_SENSOR_TEMP_HUM_SETUP)==0) { 
    if (logsOn) {
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
    if (logsOn) {
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

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Sensor CO2 init
  if (logsOn) Serial.print("[setup] - Sensor: ");
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
    if (logsOn) {
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
    if (logsOn) {
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

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Initiating buffers to draw the Co2/Temp/Hum graphs
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)  {lastHourCo2Samples[i]=0;lastHourTempSamples[i]=0;lastHourHumSamples[i]=0;}
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++) {lastDayCo2Samples[i]=0;lastDayTempSamples[i]=0;lastDayHumSamples[i]=0;}

  //-->>Buttons init
  if (logsOn) Serial.print("[setup] - Buttons: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Buttons: [");
  button1.begin();
  button2.begin();
  if ((error_setup & ERROR_BUTTONS_SETUP)==0) { 
    if (logsOn) Serial.println("OK");
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");
  } else {
    if (logsOn) {Serial.println("KO"); Serial.println("Can't continue. STOP");}
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    //stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.print(" ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    //stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Can't continue. STOP");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    //return;
  }
  stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //WiFi init
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - WiFi: ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  cuX=stext1.getCursorX(); cuY= pLL>scLL? stext1.getCursorY()-pixelsPerLine : stext1.getCursorY();
  stext1.setCursor(cuX,cuY);

  //User credentials definition
  #ifdef WIFI_SSID_CREDENTIALS
    wifiCred.wifiSSIDs[0]=WIFI_SSID_CREDENTIALS;
  #endif
  #ifdef WIFI_PW_CREDENTIALS
    wifiCred.wifiPSSWs[0]=WIFI_PW_CREDENTIALS;
  #endif
  #ifdef WIFI_SITE
    wifiCred.wifiSITEs[0]=WIFI_SITE;
  #endif
  #ifdef WIFI_SSID_CREDENTIALS_BK1
    wifiCred.wifiSSIDs[1]=WIFI_SSID_CREDENTIALS_BK1;
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK1
    wifiCred.wifiPSSWs[1]=WIFI_PW_CREDENTIALS_BK1;
  #endif
  #ifdef WIFI_SITE_BK1
    wifiCred.wifiSITEs[1]=WIFI_SITE_BK1;
  #endif
  #ifdef WIFI_SSID_CREDENTIALS_BK2
    wifiCred.wifiSSIDs[2]=WIFI_SSID_CREDENTIALS_BK2;
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK2
    wifiCred.wifiPSSWs[2]=WIFI_PW_CREDENTIALS_BK2;
  #endif
  #ifdef WIFI_SITE_BK2
    wifiCred.wifiSITEs[2]=WIFI_SITE_BK2;
  #endif

  if (logsOn) Serial.print("[setup] - WiFi: ");
  //Clean-up dots displayed after trying to get connected
  stext1.setCursor(cuX,cuY);
  for (int counter2=0; counter2<MAX_CONNECTION_ATTEMPTS*(wifiCred.activeIndex+1); counter2++) stext1.print(" ");
  stext1.setCursor(cuX,cuY);

  if (wifiEnabled) {//Only if WiFi is enabled
    error_setup|=wifiConnect(true,true,false,&auxLoopCounter,&auxCounter);

    //print Logs
    if ((error_setup & ERROR_WIFI_SETUP)==0 ) { 
      if (logsOn) Serial.println("OK");
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
      if (logsOn) Serial.println("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SSID: ");stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("No SSID Available");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      wifiCurrentStatus=wifiOffStatus;
    }
  }
  else {//If WiFi is not enabled, then inform
    if (logsOn) Serial.println("N/E");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("N/E");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");//if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    wifiCurrentStatus=wifiOffStatus;
  }

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //Pre-setting up URL things to upload samples to an external server
  //Converting SERVER_UPLOAD_SAMPLES into IPAddress variable
  char charToTest;
  uint lastBegin=0,indexArray=0;
  for (uint i=0; i<=serverToUploadSamplesString.length(); i++) {
    charToTest=serverToUploadSamplesString.charAt(i);
    if (charToTest=='.') {    
      uploadServerIPAddressOctectArray[indexArray]=serverToUploadSamplesString.substring(lastBegin,i).toInt();
      lastBegin=i+1;
      if (indexArray==2) {
        indexArray++;
        uploadServerIPAddressOctectArray[indexArray]=serverToUploadSamplesString.substring(lastBegin,serverToUploadSamplesString.length()).toInt();
      }
      else indexArray++;
    }
  }
  serverToUploadSamplesIPAddress=IPAddress(uploadServerIPAddressOctectArray[0],uploadServerIPAddressOctectArray[1],uploadServerIPAddressOctectArray[2],uploadServerIPAddressOctectArray[3]);

  //Adding the 3 latest mac bytes to the device name (in Hex format)
  WiFi.macAddress(mac);
  device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
    String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
    String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);
  
  if (logsOn) {Serial.print("[setup] - URL: ");}
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - URL: ");

  CloudSyncCurrentStatus=CloudSyncOffStatus;
  if ((error_setup & ERROR_WIFI_SETUP)==0 && wifiEnabled && uploadSamplesEnabled) { 
    //Send HttpRequest to check the server status
    // The request updates CloudSyncCurrentStatus
    error_setup|=sendAsyncHttpRequest(false,true,error_setup,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,String(GET_REQUEST_TO_UPLOAD_SAMPLES)+"test HTTP/1.1",&whileWebLoopTimeLeft);

    if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
      if (logsOn) {Serial.println("[OK]");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  URL: ");
      stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);
    }
    else {
      if (logsOn) {Serial.println("[KO]");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  URL: ");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);
    }

    if (logsOn) {Serial.print("  - URL: ");Serial.println("http://"+serverToUploadSamplesIPAddress.toString()+
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
      if (logsOn) {Serial.println("No WiFi");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("No WiFi");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    }
    else { 
      if (logsOn) {Serial.println("N/E");}
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("N/E");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    }
  }

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

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
    }
  }
  else {
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("No WiFi");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //-->>BLE init
  
  if (logsOn) Serial.print("[setup] - BLE: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - BLE:     [");
  BLEClurrentStatus=BLEOffStatus;
  if (bluetoothEnabled) {  
    if ((error_setup & ERROR_BLE_SETUP)==0 ) { 
      if (logsOn) Serial.println("OK");
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
      BLEClurrentStatus=BLEOnStatus;
    } else {
      if (logsOn) Serial.println("KO");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
    }
  }
  else {
    if (logsOn) Serial.println("N/E");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("N/E");
  }
  stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  
  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

  //-->>Battery and ADC init
  if (logsOn) Serial.print("[setup] - Bat. ADC: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Bat. ADC [");
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE);
  initVoltageArray(); //Init battery charge array
  powerState=off;
  attenuationDb=ADC_ATTEN_DB_11;
  esp_adc_cal_characterize(ADC_UNIT_1, attenuationDb, (adc_bits_width_t) ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
  if (0!=adc1_config_channel_atten(ADC1_CHANNEL_6, attenuationDb)) error_setup|=ERROR_BAT_ADC;
  if ((error_setup & ERROR_BAT_ADC)==0 ) { 
    if (logsOn) {
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
    if (logsOn) {
      Serial.println(" - voltage="+String(batADCVolt)+", charge="+String(batCharge)+"%");
      Serial.println(" - powerState="+String(powerState)+", energyCurrentMode="+String(energyCurrentMode));
    }

    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK); stext1.print(" V="+String((int)batADCVolt)+"mv, "+String(batCharge)+"%, pwState="+String(powerState));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);    
  } else {
    if (logsOn) Serial.println("KO");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  if (logsOn) Serial.println(" [setup] - error_setup="+String(error_setup));

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
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.print(" ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("Detected dead errors. Can't continue.");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine); //Add one more line
  }
  else if ((error_setup | NO_ERROR)!=0) {
    if (logsOn) Serial.println("Ready to start but with limitations, error_setup="+String(error_setup));
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("Buttons to scroll UP/DOWN");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_ORANGE_4_BITS_PALETTE,TFT_BLACK);stext1.print("Ready in ");cuX=stext1.getCursorX();cuY=stext1.getCursorY();
    stext1.print(BOOTUP_TIMEOUT);stext1.print(" sec. but with limitations");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  } else {
    if (logsOn) Serial.println("Ready to start");
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
    break;
    case reducedEnergy:
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_RE; //Keeping it for future. In this version No BAT checks in Reduce Engergy Mode to save energy
      samplePeriod=SAMPLE_PERIOD_RE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_RE;
    break;
    case lowestEnergy:
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_SE; //Keeping it for future. In this version No BAT checks in Save Engergy Mode to save energy
      samplePeriod=SAMPLE_PERIOD_SE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_SE;
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
    if (waitingMessage) {if (logsOn) Serial.println("Waiting for the warmup to finish");circularGauge.drawGauge2(0);waitingMessage=false;}
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
  if (runningMessage) {if (logsOn) Serial.println("Running.... :-)");runningMessage=false;}
  
  button1.released(); //Avoids passing the button status to the loop()
  button2.released(); //Avoids passing the button status to the loop()
  
  return (false);
}

void setup() {
  loopStartTime=loopEndTime+millis()+sleepTimer/1000;
  nowTimeGlobal=loopStartTime;
  bootCount++;

  // Initialize EEPROM with predefined size. Config variables ares stored there
  EEPROM.begin(EEPROM_SIZE);
  
  Serial.begin(115200);
  if (debugModeOn) {Serial.println("\n"+String(nowTimeGlobal)+" [SETUP] - bootCount="+String(bootCount)+", nowTime="+String(millis())+", nowTimeGlobal="+String(nowTimeGlobal));}
  randomSeed(analogRead(GPIO_NUM_32));

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT1: //Wake from Hibernate Mode by long pressing Button1
      initVariable(); //Init Global variables after hibernate mode
      loopStartTime=millis();loopEndTime=0;sleepTimer=0;nowTimeGlobal=loopStartTime;
      bootCount=0;
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
    case ESP_SLEEP_WAKEUP_EXT0: //Wake from Deep Sleep Mode by pressing Button1
      // No need to initit lastBatCharge
      if (debugModeOn) {
        Serial.println("  - Wakeup caused by external signal using RTC_IO - Ext0");
        Serial.println("    - setting things up back again after deep sleep specific for ad-hod wakeup");
        Serial.println("      - TFT init");
      }
      //Display init
      pinMode(PIN_TFT_BACKLIGHT,OUTPUT); 
      tft.init();
      digitalWrite(PIN_TFT_BACKLIGHT,LOW); //To force checkButton1() function to setup things
      tft.setRotation(1);
      tft.fillScreen(TFT_BLACK);
      if (debugModeOn) {Serial.println("      - checkButton1() & buttonWakeUp=true");}
      checkButton1();
      buttonWakeUp=true;
      nowTimeGlobal=loopStartTime+millis();
      lastTimeTurnOffBacklightCheck=nowTimeGlobal; //To avoid TIME_TURN_OFF_BACKLIGHT 
      displayMode=sampleValue;  //To force refresh TFT with the sample value Screen
      lastDisplayMode=bootup;   //To force rendering the value graph
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
      firstSetup(); //Hard bootup - Run global setup during the first boot (HW reset or power ON)
      nowTimeGlobal=loopStartTime+millis();
      return;
    break;
  }
  
  //This piece os code is run after wakeup from Deep Sleep (neither HW reset nor Hibernate)
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
  //User credentials definition
  #ifdef WIFI_SSID_CREDENTIALS
    wifiCred.wifiSSIDs[0]=WIFI_SSID_CREDENTIALS;
  #endif
  #ifdef WIFI_PW_CREDENTIALS
    wifiCred.wifiPSSWs[0]=WIFI_PW_CREDENTIALS;
  #endif
  #ifdef WIFI_SITE
    wifiCred.wifiSITEs[0]=WIFI_SITE;
  #endif
  #ifdef WIFI_SSID_CREDENTIALS_BK1
    wifiCred.wifiSSIDs[1]=WIFI_SSID_CREDENTIALS_BK1;
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK1
    wifiCred.wifiPSSWs[1]=WIFI_PW_CREDENTIALS_BK1;
  #endif
  #ifdef WIFI_SITE_BK1
    wifiCred.wifiSITEs[1]=WIFI_SITE_BK1;
  #endif
  #ifdef WIFI_SSID_CREDENTIALS_BK2
    wifiCred.wifiSSIDs[2]=WIFI_SSID_CREDENTIALS_BK2;
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK2
    wifiCred.wifiPSSWs[2]=WIFI_PW_CREDENTIALS_BK2;
  #endif
  #ifdef WIFI_SITE_BK2
    wifiCred.wifiSITEs[2]=WIFI_SITE_BK2;
  #endif

  if (debugModeOn) {Serial.println("      - wifiCurrentStatus=wifiOffStatus till WiFi reconnection");}
  wifiCurrentStatus=wifiOffStatus;

  if (debugModeOn) {Serial.println("      - serverToUploadSamplesIPAddress & device");}
  serverToUploadSamplesIPAddress=IPAddress(uploadServerIPAddressOctectArray[0],uploadServerIPAddressOctectArray[1],uploadServerIPAddressOctectArray[2],uploadServerIPAddressOctectArray[3]);
  device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
    String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
    String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);

  if (debugModeOn) {Serial.println("      - Restoring TZEnvVar="+String(TZEnvVar));}
  setenv("TZ",TZEnvVar,1); tzset(); //Restore TZ enviroment variable to show the right time

  nowTimeGlobal=loopStartTime+millis();
  if (debugModeOn) {Serial.print(String(nowTimeGlobal)+" [SETUP] - Exit - Time: ");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo, "%d/%m/%Y - %H:%M:%S");}
}


void loop() {
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
  if ( (((nowTimeGlobal-lastTimeVOLTCheck) >= voltageCheckPeriod) && fullEnergy==energyCurrentMode) || forceGetVolt ) {
    if (debugModeOn) {Serial.println("-------------oooooOOOOOOoooooo------------");Serial.println(String(nowTimeGlobal)+"  - VOLTAGE_CHECK_PERIOD");}
    
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

    if (forceGetSample) forceGetSample=false;

    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - SAMPLE_PERIOD - exit");}
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
        if (debugModeOn) {Serial.print("    - **** displayMode=sampleValue - Rendering only if new sample value - Time: ");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
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
        if (debugModeOn) {Serial.print("    - **** displayMode=co2LastHourGraph - Time: ");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
        //Last Hour graph is displayed
        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastHourGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastHourCo2(); updateHourGraph=false;}
        else if (updateHourGraph) {drawGraphLastHourCo2(); updateHourGraph=false;}//Draw new graph only if buffer was updated

        lastDisplayMode=co2LastHourGraph;
      break;
      case co2LastDayGraph:
        if (debugModeOn) {Serial.print("    - **** displayMode=co2LastDayGraph - Time: ");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo, "%d/%m/%Y - %H:%M:%S ****");}
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
  // 3) or buttons are pressed to wake up from sleep
  // 4) or previous WiFi re-connection try was ABORTED (button pressed) or BREAK (need to refresh display)
  nowTimeGlobal=loopStartTime+millis();
  if ((((nowTimeGlobal-lastTimeWifiReconnectionCheck) >= WIFI_RECONNECT_PERIOD) || forceWifiReconnect ) && 
      wifiEnabled && !firstBoot && (wifiCurrentStatus==wifiOffStatus || WiFi.status()!=WL_CONNECTED) ) {
     
    if (debugModeOn) {
      Serial.println(String(nowTimeGlobal)+"  - WIFI_RECONNECT_PERIOD ("+String(WIFI_RECONNECT_PERIOD/1000)+" s)");
      Serial.println("    - lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));
      Serial.println("    - nowTimeGlobal-lastTimeWifiReconnectionCheck="+String(nowTimeGlobal-lastTimeWifiReconnectionCheck));
      Serial.println("    - forceWifiReconnect="+String(forceWifiReconnect));
      Serial.println("    - !firstBoot="+String(!firstBoot));
      Serial.println("    - wifiCurrentStatus="+String(wifiCurrentStatus)+", wifiOffStatus=0");
      Serial.println("    - WiFi.status()="+String(WiFi.status())+", WL_CONNECTED=3");
    }

    //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
    if (!wifiResuming) lastTimeWifiReconnectionCheck=nowTimeGlobal; //Only if the WiFi reconnection didn't ABORT or BREAK in the previous interaction
    
    wifiStatus previousWifiCurrentStatus=wifiCurrentStatus;
    if(forceWifiReconnect) forceWifiReconnect=false;
    
    //If WiFi disconnected (wifiOffStatus), then re-connect
    //Conditions for wifiCurrentStatus==wifiOffStatus
    // - no found SSID in init()
    // - no found SSID when ICON_STATUS_REFRESH_PERIOD
    //WiFi.status() gets the WiFi status inmediatly. No need to scann WiFi networks
  
    //WiFi Reconnection
    if (debugModeOn) {Serial.println("    - auxLoopCounter="+String(auxLoopCounter)+", auxCounter="+String(auxCounter));}
    
    forceWEBTestCheck=false; //If WiFi reconnection is successfull, then check WEB server to update ICON. Decision is done below, if NO_ERROR
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
        forceWifiReconnect=false;
        wifiResuming=false;
        //Send HttpRequest to check the server status
        // The request updates CloudSyncCurrentStatus
        //if (uploadSamplesEnabled) 
        forceWEBTestCheck=true; //Will check WEB server in the next loop() interaction
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - wifiConnect() finish with NO_ERROR. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWifiReconnect="+String(forceWifiReconnect)+", forceWEBTestCheck="+String(forceWEBTestCheck));}
      break;
    } 

    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - WIFI_RECONNECT_PERIOD - exit, lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));}
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
    
    if (debugModeOn) {
      Serial.println(String(loopStartTime+millis())+"      - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      getLocalTime(&startTimeInfo);Serial.print("  - NTP_KO_CHECK_PERIOD, lastTimeNTPCheck="+String(lastTimeNTPCheck));Serial.println(&startTimeInfo," - Exit - Time: %d/%m/%Y - %H:%M:%S");
    }
  }
  
  //Regular actions every UPLOAD_SAMPLES_PERIOD seconds - Upload samples to external server
  //WiFi SITE must be UPLOAD_SAMPLES_FROM_SITE (case sensitive), otherwise sample is not uploaded
  //forceWEBCheck is set when in the previous interaction the WEB connection was either:
  // ABORTED (Button Pressed) or
  // BREAK  (Display Refresh)
  //This way it resume WEB connection and the Display or Buttons can be monitorized
  //forceWEBTestCheck is set if WiFi is recovered from disconnection in order to force WEB
  // server check and ICON update. If test overlaps the uploadSamplesPeriod timing, the later
  // has higher priority in order to send the samples on time.
  nowTimeGlobal=loopStartTime+millis();
  if ((((nowTimeGlobal-lastTimeUploadSampleCheck) >= uploadSamplesPeriod) || firstBoot || forceWEBCheck || forceWEBTestCheck) && uploadSamplesEnabled &&
      wifiCurrentStatus!=wifiOffStatus && wifiEnabled &&
      (0==wifiCred.wifiSITEs[wifiCred.activeIndex].compareTo(UPLOAD_SAMPLES_FROM_SITE)) ) {
    
    //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
    if (!webResuming && !forceWEBTestCheck) lastTimeUploadSampleCheck=nowTimeGlobal; //Only if the HTTP connection didn't ABORT or BREAK in the previous interaction
    
    if (forceWEBTestCheck && ((nowTimeGlobal-lastTimeUploadSampleCheck) >= uploadSamplesPeriod))
     {forceWEBTestCheck=false;} //Timer has priority over WEB test. httpRequest will be setup to send samples 

    if (debugModeOn) {Serial.print(String(nowTimeGlobal)+"  - UPLOAD_SAMPLES_PERIOD - Time: ");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo, "%d/%m/%Y - %H:%M:%S");}
    
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
      "&webServerError2="+webServerError2+"&webServerError3="+webServerError3+" HTTP/1.1";

    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - serverToUploadSamplesIPAddress="+String(serverToUploadSamplesIPAddress)+", SERVER_UPLOAD_PORT="+String(SERVER_UPLOAD_PORT)+
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
      case ERROR_WEB_SERVER:
        forceWEBCheck=false;
        webResuming=false;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - sendAsyncHttpRequest() finish with ERROR_WEB_SERVER. CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", forceWEBCheck="+String(forceWEBCheck));}
      break;
      case NO_ERROR:
        forceWEBCheck=false;
        webResuming=false;
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - sendAsyncHttpRequest() finish with NO_ERROR. CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", forceWEBCheck="+String(forceWEBCheck));}
      break;
    }
  
    if (debugModeOn) {Serial.print(String(loopStartTime+millis())+"  - UPLOAD_SAMPLES_PERIOD - Exit - Time: ");getLocalTime(&startTimeInfo);Serial.print(&startTimeInfo, "%d/%m/%Y - %H:%M:%S");Serial.println(", lastTimeUploadSampleCheck="+String(lastTimeUploadSampleCheck));}
  }

  if (firstBoot) firstBoot=false;
  if (buttonWakeUp) buttonWakeUp=false;

  //Go to hibernate if battery is ran out
  if (batCharge<=BAT_CHG_THR_TO_HIBERNATE && onlyBattery==powerState) go_to_hibernate();

  //Going to sleep, but only if the display if OFF and not there's Battery power
  //if (LOW==digitalRead(PIN_TFT_BACKLIGHT)) go_to_sleep();
  //if (LOW==digitalRead(PIN_TFT_BACKLIGHT) && energyCurrentMode!=fullEnergy) go_to_sleep();
  if (LOW==digitalRead(PIN_TFT_BACKLIGHT) && onlyBattery==powerState) go_to_sleep();
}