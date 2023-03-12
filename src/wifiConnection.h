//#ifndef WIFIFRAMEWORK
#ifndef _DECLAREGLOBALPARAMETERS_
  #include <WiFi.h>
  #include "global_setup.h"
  #include "user_setup.h"
  #include "ButtonChecks.h"
  #include "misc.h"

  extern bool logsOn;
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

  #ifndef _DISPLAYSUPPORTINFO_
    enum CloudClockStatus {CloudClockOnStatus,CloudClockOffStatus};
    #define _DISPLAYSUPPORTINFO_
  #endif

  extern wifiNetworkInfo wifiNet; //Struct to store WiFi parameters
  extern wifiCredentials wifiCred;
  extern String ntpServers[4];
  extern uint8_t ntpServerIndex;
  #ifdef NTP_TZ_ENV_VARIABLE
    extern String TZEnvVariable;
  #endif
#endif

extern RTC_DATA_ATTR int errorsWiFiCnt;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus;
extern RTC_DATA_ATTR int errorsNTPCnt;
extern boolean NTPResuming;
extern RTC_DATA_ATTR struct tm startTimeInfo;
extern RTC_DATA_ATTR char TZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH];
extern RTC_DATA_ATTR uint64_t lastTimeNTPCheck;

void printNetData();
wifiNetworkInfo * printCurrentWiFi(boolean logsOn, int16_t *numberWiFiNetworks);
uint32_t wifiConnect(boolean logsOn,boolean msgTFT,boolean checkButtons,uint8_t* auxLoopCounter,uint8_t* auxCounter);
uint32_t setupNTPConfig(boolean fromSetup,uint8_t* auxLoopCounter,uint64_t* whileLoopTimeLeft);
bool runAPMode();

