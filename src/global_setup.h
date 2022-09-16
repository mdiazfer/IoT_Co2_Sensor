/* 
    Definition of global parameters
    No user modification is required in her
*/

#include "user_setup.h"

#define VERSION 0.6.2
#define _STRINGIFY_(PARAMETER) #PARAMETER
#define _CONCATENATE_(PARAMETER) MH_Z19B ## PARAMETER               //This two-level macro concatenates 2 labels. Useful to make some
#define _CO2_SENSOR_PARAMETER_(PARAMETER) _CONCATENATE_(_ ## PARAMETER)  // parameters sensor-model-independant
#define __MHZ19B__
#define __TFT_DISPLAY_PRESENT__

//Global board stuff
#define ERROR_LED 25
#define TFT_X_WIDTH 240
#define TFT_Y_HEIGH 135

//Co2 Sensor stuff
#ifdef __MHZ19B__   //Sensor model dependant parameters
  #define CO2_SENSOR  _STRINGIFY_(MH-Z19B)
  #define CO2_SENSOR_TYPE  "MH-Z19B"
  #define MH_Z19B_RX 26 //RX pin in the ESP board (TX pin in the CO2 sensor)
  #define MH_Z19B_TX 27 //TX pin in the ESP board (TX pin in the CO2 sensor)
  #define MH_Z19B_CO2_IN    37 //GPIO pin in the ESB board to connect the PWM CO2 sensor output
  #define MH_Z19B_CO2_WARMING_TIME 30000  //Preheat time according to the datasheet
  #define MH_Z19B_CO2_RANGE 2000  //Range of CO2 measurments. 0-2000 is adviced for MHZ19B as per datasheet for better accuracy
  #define MH_Z19B_CO2_MIN   0
  #define MH_Z19B_CO2_MAX   MH_Z19B_CO2_RANGE
  #define MH_Z19B_TEMP_MAX  50
  #define MH_Z19B_TEMP_MIN  -10 
#else
  //For other sensor models copy the parameters for __MHZ19B__ customized for the rith model
  #define CO2_SENSOR  "UNKNOWN"
#endif

    //Sensor model independant parameters
    #define CO2_SENSOR_RX _CO2_SENSOR_PARAMETER_(RX)
    #define CO2_SENSOR_TX _CO2_SENSOR_PARAMETER_(TX)
    #define CO2_SENSOR_CO2_IN  _CO2_SENSOR_PARAMETER_(CO2_IN)
    #define CO2_SENSOR_WARMING_TIME _CO2_SENSOR_PARAMETER_(WARMING_TIME)
    #define CO2_SENSOR_CO2_RANGE _CO2_SENSOR_PARAMETER_(CO2_RANGE)
    #define CO2_SENSOR_CO2_MIN _CO2_SENSOR_PARAMETER_(CO2_MIN)
    #define CO2_SENSOR_CO2_MAX _CO2_SENSOR_PARAMETER_(CO2_MAX)
    #define CO2_SENSOR_TEMP_MAX _CO2_SENSOR_PARAMETER_(TEMP_MAX)
    #define CO2_SENSOR_TEMP_MIN _CO2_SENSOR_PARAMETER_(TEMP_MIN)
    #define SAMPLE_PERIOD          10000 //milliseconds

//Error stuff
#define NO_ERROR                0x00
#define ERROR_DISPLAY_SETUP     0x01
#define ERROR_SENSOR_SETUP      0x02
#define ERROR_BUTTONS_SETUP     0x03
#define ERROR_WIFI_SETUP        0x04
#define ERROR_BLE_SETUP         0x05
#define ERROR_SSID_CONNECTION   0x06

//Display stuff - Values customized for TTGO T-Display board
#define TFT_MAX_X 240
#define TFT_MAX_Y 135
#define TFT_BLACK 0x0000 // black
#define TEXT_SIZE_BOOT_SCREEN 1
#define TEXT_FONT_BOOT_SCREEN 2
#define TEXT_SIZE 3
#define TEXT_FONT 1
#define TEXT_SIZE_UNITS_CO2 2
#define TEXT_FONT_UNITS_CO2 1
#define DISPLAY_REFRESH_PERIOD      1*SAMPLE_PERIOD //milliseconds
#define DISPLAY_MODE_REFRESH_PERIOD 1*SAMPLE_PERIOD //milliseconds
#define CO2_GAUGE_X      80
#define CO2_GAUGE_Y      95
#define CO2_GAUGE_R      70
#define CO2_GAUGE_WIDTH  30
#define CO2_GAUGE_SECTOR 60  //Sector angle (degrees)
#define CO2_GAUGE_TH1    800 //Threshold for warning (ppm)
#define CO2_GAUGE_TH2    950 //Threshold for alarm (ppm)
#define CO2_GAUGE_MIN   CO2_SENSOR_CO2_MIN   //Parameter sensor model independant  
#define CO2_GAUGE_MAX   CO2_SENSOR_CO2_MAX   //Parameter sensor model independant
#define CO2_GAUGE_RANGE CO2_SENSOR_CO2_RANGE //Parameter sensor model independant
#define TEMP_BAR_X       145
#define TEMP_BAR_Y       50
#define TEMP_BAR_LENGTH  95
#define TEMP_BAR_HEIGH   10
#define TEMP_BAR_TH1     19 //Threshold from cold to normal  temperature
#define TEMP_BAR_TH2     27 //Threshold from normal to hot temperature
#define TEMP_BAR_MIN     CO2_SENSOR_TEMP_MIN //Parameter sensor model independant
#define TEMP_BAR_MAX     CO2_SENSOR_TEMP_MAX //Parameter sensor model independant
#define GRAPH_WIDTH     200 //Width of the area for drawing the evolution graph
#define GRAPH_HEIGH     100 //Heigh of the area for drawing the evolution graph
#define SAMPLE_T_LAST_HOUR  20 //Seconds - Period of last hour samples to be recorded
#define SAMPLE_T_LAST_DAY   450 //Seconds - Period of last day samples to be recorded
#define CO2_GRAPH_X     25  //X origin for co2 graph
#define CO2_GRAPH_Y     12  //Y origin for co2 graph
#define CO2_GRAPH_WIDTH 200 //WIDTH origin for co2 graph
#define CO2_GRAPH_HEIGH 100 //HEIGH origin for co2 graph
#define CO2_GRAPH_X_END CO2_GRAPH_X+CO2_GRAPH_WIDTH //X end for co2 graph
#define CO2_GRAPH_Y_END CO2_GRAPH_Y+CO2_GRAPH_HEIGH //Y end for co2 graph


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



