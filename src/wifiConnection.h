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
    
    #define _WIFINETWORKINFO_ 
  #endif
  extern wifiNetworkInfo wifiNet; //Struct to store WiFi parameters
#endif

/*void printWifiData();
void printCurrentNet();
uint8_t wifiConnect();*/

void printNetData();
wifiNetworkInfo * printCurrentWiFi();
uint8_t wifiConnect();