
#include <Arduino.h>
#include "global_setup.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "misc.h"
#include "MHZ19.h"
#include <WiFi.h>
#include <EEPROM.h>

#ifndef _DISPLAYSUPPORTINFO_
  enum CloudSyncStatus {CloudSyncOnStatus,CloudSyncOffStatus};
  enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
  enum BLEStatus {BLEOnStatus,BLEConnectedStatus,BLEOffStatus};
  #define _DISPLAYSUPPORTINFO_
#endif

#ifndef _WIFINETWORKINFO_
  typedef struct {
    uint8_t networkItem;
    String ssid;
    uint8_t encryptionType;
    int32_t RSSI;
    uint8_t* BSSID;
    int32_t channel;
  } wifiNetworkInfo;  //Struct to store WiFi parameters

  typedef struct {
    String wifiSSIDs[3];
    String wifiPSSWs[3];
    String wifiSITEs[3];
    bool   SiteAllow[3]; //whether to upload samples from the Site or not
    uint8_t activeIndex;
  } wifiCredentials;  //Struct to store user WiFi credentials    
  
  #define _WIFINETWORKINFO_ 
#endif

extern RTC_DATA_ATTR AsyncWebServer webServer;
extern RTC_DATA_ATTR float_t valueCO2,valueT,valueHum;
extern String device;
extern RTC_DATA_ATTR enum powerModes powerState;
extern RTC_DATA_ATTR struct tm startTimeInfo;
extern String TZEnvVariable,TZName;
extern RTC_DATA_ATTR char TZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH];
extern RTC_DATA_ATTR const String tempHumSensorType;
extern RTC_DATA_ATTR const String co2SensorType;
extern RTC_DATA_ATTR char co2SensorVersion[5];
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus;
extern RTC_DATA_ATTR uint32_t error_setup;
#ifdef __MHZ19B__
  extern MHZ19 co2Sensor;  //64 B
#endif
extern RTC_DATA_ATTR boolean debugModeOn,wifiEnabled,bluetoothEnabled,uploadSamplesEnabled,autoBackLightOff,
                              forceDisplayRefresh,forceDisplayModeRefresh,forceWifiReconnect,forceNTPCheck,
                              forceWEBTestCheck,reconnectWifiAndRestartWebServer,resyncNTPServer,
                              deviceReset,factoryReset;
extern String ntpServers[4];
extern uint8_t ntpServerIndex;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus;
extern IPAddress serverToUploadSamplesIPAddress;
extern RTC_DATA_ATTR enum energyModes energyCurrentMode,configSavingEnergyMode;
extern RTC_DATA_ATTR enum availableStates currentState;
extern RTC_DATA_ATTR uint64_t lastTimeTurnOffBacklightCheck,loopStartTime,previousLastTimeSampleCheck,nowTimeGlobal;
extern RTC_DATA_ATTR enum displayModes displayMode,lastDisplayMode;
extern RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
extern RTC_DATA_ATTR enum BLEStatus BLEClurrentStatus;
extern RTC_DATA_ATTR uint64_t nowTimeGlobal,lastTimeUploadSampleCheck;
extern RTC_DATA_ATTR ulong uploadSamplesPeriod;
extern RTC_DATA_ATTR int errorsWiFiCnt,errorsSampleUpts,errorsNTPCnt;
extern wifiCredentials wifiCred;

String processor(const String& var);
String processorAP(const String& var);
uint32_t initWebServer();
uint32_t initAPWebServer();