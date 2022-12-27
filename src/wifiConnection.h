//#ifndef WIFIFRAMEWORK
#ifndef _DECLAREGLOBALPARAMETERS_
  #include <WiFi.h>
  #include "global_setup.h"
  #include "user_setup.h"

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
      uint8_t activeIndex;
    } wifiCredentials;  //Struct to store user WiFi credentials    

    #define _WIFINETWORKINFO_ 
  #endif
  extern wifiNetworkInfo wifiNet; //Struct to store WiFi parameters
  extern wifiCredentials wifiCred;
  extern String ntpServers[4];
  extern uint8_t ntpServerIndex;
#endif

void printNetData();
wifiNetworkInfo * printCurrentWiFi(boolean logsOn, int16_t *numberWiFiNetworks);
uint8_t wifiConnect(boolean logsOn, boolean msgTFT);