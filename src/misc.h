#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "global_setup.h"
#include "battery.h"

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


extern RTC_DATA_ATTR boolean debugModeOn;
extern RTC_DATA_ATTR uint64_t loopEndTime,loopStartTime,sleepTimer;
extern RTC_DATA_ATTR float_t batCharge;
extern RTC_DATA_ATTR struct tm startTimeInfo;
extern RTC_DATA_ATTR boolean wifiEnabled,bluetoothEnabled,uploadSamplesEnabled,webServerEnabled;
extern RTC_DATA_ATTR AsyncWebServer webServer;
extern wifiCredentials wifiCred;
extern String TZEnvVariable,TZName;
extern String ntpServers[4];

void go_to_hibernate();
void go_to_sleep();
String roundFloattoString(float_t number, uint8_t decimals);
uint16_t checkSum(byte *addr, uint32_t count);
void factoryConfReset();
bool initTZVariables();
IPAddress stringToIPAddress(String stringIPAddress);