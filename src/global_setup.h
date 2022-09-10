/* 
    Definition of global parameters
    No user modification is required in her
*/

#include "user_setup.h"

#define VERSION 0.5

//Global board stuff
#define ERROR_LED 25
#define TFT_X_WIDTH 240
#define TFT_Y_HEIGH 135

//Co2 Sensor stuff
#define MH_Z19_RX 26 //12
#define MH_Z19_TX 27 //13
#define CO2_IN    37

//Error stuff
#define NO_ERROR                0x00
#define ERROR_DISPLAY_SETUP     0x01
#define ERROR_SENSOR_SETUP      0x02
#define ERROR_BUTTONS_SETUP     0x03
#define ERROR_WIFI_SETUP        0x04
#define ERROR_BLE_SETUP         0x05
#define ERROR_SSID_CONNECTION   0x06

//Display stuff
#define TFT_BLACK 0x0000 // black
#define TEXT_SIZE_BOOT_SCREEN 1
#define TEXT_FONT_BOOT_SCREEN 2
#define TEXT_SIZE 3
#define TEXT_FONT 1
#define VALUE_REFRESH_PERIOD 10000 //milliseconds


//WiFi stuff
#define MAX_CONNECTION_ATTEMPTS 16

//Global stuff
#ifdef _DECLAREGLOBALPARAMETERS_
  bool logsOn = true;         //Whether enable or not logs on the seriaml line [TRUE | FALSE]

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

  wifiNetworkInfo wifiNet;
  
  #undef _DECLAREGLOBALPARAMETERS_
#endif



