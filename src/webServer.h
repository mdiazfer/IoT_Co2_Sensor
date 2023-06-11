
#include <Arduino.h>
#include "global_setup.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "Update.h"
#include "misc.h"
#include "MHZ19.h"
#include <WiFi.h>
#include <EEPROM.h>
#include <Arduino_JSON.h>
#include <AsyncMqttClient.h>
#include <BLEDevice.h>

#ifndef _DISPLAYSUPPORTINFO_
  enum BLEStatus {BLEOnStatus,BLEConnectedStatus,BLEStandbyStatus,BLEOffStatus};
  #define _DISPLAYSUPPORTINFO_
#endif

#ifndef _WIFISUPPORT_
  enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
  extern RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
  #define _WIFISUPPORT_
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
extern RTC_DATA_ATTR AsyncEventSource webEvents;
extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
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
extern RTC_DATA_ATTR uint64_t lastTimeBLECheck,loopStartTime;
extern RTC_DATA_ATTR uint8_t BLEnoLoadedCount,BLEunloadsCount,softResetReason;
extern RTC_DATA_ATTR int connectivityFailsCounter,webServerFailsCounter,softResetCounter,BLEnoLoadedCounter,BLEunloadsCounter,minHeapSeenCounter;
#ifdef __MHZ19B__
  extern MHZ19 co2Sensor;  //64 B
#endif
extern RTC_DATA_ATTR boolean debugModeOn,wifiEnabled,bluetoothEnabled,uploadSamplesEnabled,autoBackLightOff,
                              forceDisplayRefresh,forceDisplayModeRefresh,forceWifiReconnect,forceNTPCheck,
                              forceWEBTestCheck,reconnectWifiAndRestartWebServer,resyncNTPServer,
                              deviceReset,factoryReset,forceMQTTConnect;
extern String ntpServers[4];
extern uint8_t ntpServerIndex;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus;
extern IPAddress serverToUploadSamplesIPAddress;
extern RTC_DATA_ATTR enum energyModes energyCurrentMode,configSavingEnergyMode;
extern RTC_DATA_ATTR enum availableStates currentState;
extern RTC_DATA_ATTR uint64_t lastTimeTurnOffBacklightCheck,loopStartTime,previousLastTimeSampleCheck,nowTimeGlobal;
//extern RTC_DATA_ATTR enum displayModes displayMode,lastDisplayMode;
extern RTC_DATA_ATTR enum BLEStatus BLECurrentStatus;
extern RTC_DATA_ATTR uint64_t nowTimeGlobal,lastTimeUploadSampleCheck,lastTimeBLEOnCheck;
extern RTC_DATA_ATTR ulong uploadSamplesPeriod,BLEOnTimeout;
extern RTC_DATA_ATTR int errorsWiFiCnt,errorsSampleUpts,errorsNTPCnt,SPIFFSErrors;
extern RTC_DATA_ATTR boolean OTAUpgradeBinAllowed,SPIFFSUpgradeBinAllowed;
extern RTC_DATA_ATTR char BLEProximityUUID[BLE_BEACON_UUID_LENGH];
extern RTC_DATA_ATTR uint16_t BLEMajor,BLEMinor;
extern wifiCredentials wifiCred;
extern uint8_t fileUpdateError,errorOnActiveCookie,errorOnWrongCookie,bootCount,resetCount;
extern size_t fileUpdateSize,OTAAvailableSize,SPIFFSAvailableSize;
extern int updateCommand;
extern uint32_t flashSize,programSize,fileSystemSize,fileSystemUsed,minHeapSeen;
extern String fileUpdateName;
extern char activeCookie[],currentSetCookie[];
extern JSONVar samples;
extern String mqttTopicName,lastURI;
extern bool webServerResponding,isBeaconAdvertising,BLEtoBeLoaded;

extern void stopBLE(uint8_t caller);

String processorInfo(const String& var);
String processorBasic(const String& var);
String processorCloud(const String& var);
String processorBluetooth(const String& var);
String processorMaintenance(const String& var);
String processorContainer(const String& var);
String processorAP(const String& var);
uint32_t initWebServer();
uint32_t initAPWebServer();
extern uint32_t mqttClientInit(bool fromSetup, bool debugModeOn, bool TFTDisplayLogs);
extern void BLEstop();
