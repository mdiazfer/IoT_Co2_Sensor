#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "global_setup.h"
#include "battery.h"
#include <string.h>
#include <assert.h>
#include "esp_partition.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include <AsyncMqttClient.h>


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

#ifndef _WIFISUPPORT_
  enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
  extern RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
  #define _WIFISUPPORT_
#endif

extern RTC_DATA_ATTR boolean debugModeOn,OTAUpgradeBinAllowed,SPIFFSUpgradeBinAllowed;
extern RTC_DATA_ATTR uint64_t loopEndTime,loopStartTime,sleepTimer;
extern RTC_DATA_ATTR float_t batCharge;
extern RTC_DATA_ATTR struct tm startTimeInfo;
extern RTC_DATA_ATTR boolean wifiEnabled,bluetoothEnabled,uploadSamplesEnabled,webServerEnabled,mqttServerEnabled,secureMqttEnabled;
extern RTC_DATA_ATTR AsyncWebServer webServer;
extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus;
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus;
extern RTC_DATA_ATTR AsyncEventSource webEvents;
extern RTC_DATA_ATTR ulong BLEPeriod,BLEOnTimeout;

extern wifiCredentials wifiCred;
extern String TZEnvVariable,TZName;
extern String ntpServers[4];
extern struct tm nowTimeInfo;
extern String userName,userPssw,mqttUserName,mqttUserPssw,mqttServer,mqttTopicPrefix,mqttTopicName,device;

void go_to_hibernate();
void go_to_sleep();
String roundFloattoString(float_t number, uint8_t decimals);
uint16_t checkSum(byte *addr, uint32_t count);
void factoryConfReset();
bool initTZVariables();
IPAddress stringToIPAddress(String stringIPAddress);
String getFileExt(const String& s);
size_t getAppOTAPartitionSize(uint8_t type, uint8_t subtype);
void logRamStats (const char text[]);
void detachNetwork(void);