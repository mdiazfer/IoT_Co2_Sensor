/* 
    Definition of global parameters
    No user modification is required in her
*/

#define BUILD_TYPE_DEVELOPMENT  1
#define BUILD_TYPE_SENSOR_CASE  2
#define BUILD_TYPE_SENSOR_CASE_OFI  3

#include "user_setup.h"

#define VERSION "0.9.5"
#define _STRINGIFY_(PARAMETER) #PARAMETER
#define _CONCATENATE_(PARAMETER) MH_Z19B ## PARAMETER                    //This two-level macro concatenates 2 labels. Useful to make some
#define _CO2_SENSOR_PARAMETER_(PARAMETER) _CONCATENATE_(_ ## PARAMETER)  // parameters sensor-model-independant

#define __MHZ19B__
#define __TFT_DISPLAY_PRESENT__
#define __SI7021__

//Global board stuff
#define ERROR_LED 25    //Customized for TTGO T-DISPLAY (T1) board
#define TFT_X_WIDTH 240 //Customized for TTGO T-DISPLAY (T1) board
#define TFT_Y_HEIGH 135 //Customized for TTGO T-DISPLAY (T1) board
#define BUTTON1     35  //Customized for TTGO T-DISPLAY (T1) board
#define BUTTON2     0   //Customized for TTGO T-DISPLAY (T1) board
#define CO2_RX 26 //RX pin in the ESP board (TX pin in the CO2 sensor)
#define CO2_TX 27 //TX pin in the ESP board (TX pin in the CO2 sensor)
#define I2C_SDA 21 //I2C - SDA pin in the ESP board (SDA pin in the sensor)
#define I2C_SCL 22 //I2C - SCL pin in the ESP board (SCL pin in the sensor)
#define DEVICE_NAME_PREFIX "co2-sensor"
#define PIN_TFT_BACKLIGHT 4
#define DEBUG_MODE_ON true

//Co2 Sensor stuff
#ifdef __MHZ19B__   //Sensor model dependant parameters
  #define CO2_SENSOR  _STRINGIFY_(MH-Z19B)
  #define CO2_SENSOR_TYPE  "MH-Z19B"
  #define MH_Z19B_RX CO2_RX //RX pin in the ESP board (TX pin in the CO2 sensor)
  #define MH_Z19B_TX CO2_TX //TX pin in the ESP board (TX pin in the CO2 sensor)
  #define MH_Z19B_CO2_IN    37 //GPIO pin in the ESB board to connect the PWM CO2 sensor output
  #define MH_Z19B_CO2_WARMING_TIME 0 //30000 //90000  //Preheat time according to the datasheet
  #define MH_Z19B_CO2_RANGE 2000  //Range of CO2 measurments. 0-2000 is adviced for MHZ19B as per datasheet for better accuracy
  #define MH_Z19B_CO2_MIN   0
  #define MH_Z19B_CO2_MAX   MH_Z19B_CO2_RANGE
  #define MH_Z19B_TEMP_MAX  50
  #define MH_Z19B_TEMP_MIN  -10
  #define MH_Z19B_MIN_VOLT  1850  //Min voltage for right sample - millivolts
#else
  //For other sensor models copy the parameters for __MHZ19B__ customized for the rith model
  #define CO2_SENSOR  "UNKNOWN"
#endif

//Temperature & Humidity Sensor stuff
#ifdef  __SI7021__ //Sendor model dependant parameters
  #define TEMP_HUM_SENSOR  _STRINGIFY_(SI7021)  //SI7021, SHT21 and HTU21 are all equivalent
  #define TEMP_HUM_SENSOR_TYPE  "SI7021"
  #define SI7021_SDA I2C_SDA //I2C - SDA pin in the ESP board (SDA pin in the sensor)
  #define SI7021_SCL I2C_SCL //I2C - SCL pin in the ESP board (SCL pin in the sensor)
  #define SI7021_TEMP_MAX  50
  #define SI7021_TEMP_MIN  -10
  #define SI7021_HUM_MAX  100
  #define SI7021_HUM_MIN  0
  #define SI7021_TEMP_OFFSET  0
#else
  //For other sensor models copy the parameters for __SI7021__ customized for the rith model
  #define TEMP_HUM_SENSOR  "UNKNOWN"
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
    #define CO2_SENSOR_HUM_MAX 100
    #define CO2_SENSOR_HUM_MIN 0

//Error stuff
#define NO_ERROR                0x00
#define ERROR_DISPLAY_SETUP     0x01
#define ERROR_SENSOR_TEMP_HUM_SETUP  0x02
#define ERROR_SENSOR_CO2_SETUP  0x03
#define ERROR_BUTTONS_SETUP     0x04
#define ERROR_WIFI_SETUP        0x05
#define ERROR_BLE_SETUP         0x06
#define ERROR_SSID_CONNECTION   0x07
#define ERROR_NTP_SERVER        0x08
#define ERROR_WEB_SERVER        0x09
#define ERROR_BAT_ADC           0x0A
#define ERROR_ABORT_WIFI_SETUP  0x0B
#define ERROR_BREAK_WIFI_SETUP  0x0C
#define ERROR_ABORT_NTP_SETUP   0x0D
#define ERROR_BREAK_NTP_SETUP   0x0E
#define ERROR_ABORT_WEB_SETUP   0x0F
#define ERROR_BREAK_WEB_SETUP   0x10

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
#define TEXT_SIZE_MENU 2
#define TEXT_FONT_MENU 1
#define SCROLL_PER_SPRITE 5.5
#define LINES_PER_TEXT_SCROLL 8
#define LINES_PER_TEXT_SPRITE SCROLL_PER_SPRITE*LINES_PER_TEXT_SCROLL
#define FIRST_LINE_TO_PRINT 5
#define MENU_GLOBAL_FORE_COLOR TFT_CYAN
#define MENU_GLOBAL_BACK_COLOR TFT_BLACK
#define MENU_FORE_COLOR TFT_GOLD
#define MENU_BACK_COLOR TFT_BLACK
#define MENU_INFO_FORE_COLOR TFT_WHITE
#define MENU_INFO_BACK_COLOR TFT_BLACK
#define MENU_CONFIG_FORE_COLOR TFT_VIOLET
#define MENU_CONFIG_BACK_COLOR TFT_BLACK
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
#define CO2_GRAPH_X     25  //X origin for co2 graph
#define CO2_GRAPH_Y     12  //Y origin for co2 graph
#define CO2_GRAPH_WIDTH 200 //WIDTH origin for co2 graph
#define CO2_GRAPH_HEIGH 100 //HEIGH origin for co2 graph
#define CO2_GRAPH_X_END CO2_GRAPH_X+CO2_GRAPH_WIDTH //X end for co2 graph
#define CO2_GRAPH_Y_END CO2_GRAPH_Y+CO2_GRAPH_HEIGH //Y end for co2 graph
#define TFT_BLACK_4_BITS_PALETTE  0    //  0  ^
#define TFT_BROWN_4_BITS_PALETTE  1  //  1  |
#define TFT_RED_4_BITS_PALETTE    2  //  2  |
#define TFT_ORANGE_4_BITS_PALETTE 3  //  3  |
#define TFT_YELLOW_4_BITS_PALETTE 4  //  4  Colours 0-9 follow the resistor colour code!
#define TFT_GREEN_4_BITS_PALETTE  5  //  5  |
#define TFT_BLUE_4_BITS_PALETTE   6  //  6  |
#define TFT_PURPLE_4_BITS_PALETTE 7  //  7  |
#define TFT_DARKGREY_4_BITS_PALETTE 8//  8  |
#define TFT_WHITE_4_BITS_PALETTE  9  //  9  v
#define TFT_CYAN_4_BITS_PALETTE   10  // 10  Blue+green mix
#define TFT_MAGENTA_4_BITS_PALETTE 11 // 11  Blue+red mix
#define TFT_MAROON_4_BITS_PALETTE  12 // 12  Darker red colour
#define TFT_DARKGREEN_4_BITS_PALETTE 13// 13  Darker green colour
#define TFT_NAVY_4_BITS_PALETTE   14  // 14  Darker blue colour
#define TFT_PINK_4_BITS_PALETTE   15   // 15

//WiFi stuff
#define WIFI_ENABLED  true
#define MAX_CONNECTION_ATTEMPTS 10
#if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE
  #define NTP_SERVER  "10.88.50.5"
  #define NTP_SERVER2  "time2.google.com"  //216.239.35.4
  #define NTP_SERVER3  "time4.google.com"  //216.239.35.12
  #define NTP_SERVER4  "time.apple.com"
#endif
#if BUILD_ENV_NAME==BUILD_TYPE_DEVELOPMENT
  #define NTP_SERVER  "10.88.50.5"
  #define NTP_SERVER2  "time2.googles.com"  //216.239.35.4
  #define NTP_SERVER3  "time4.google.com"  //216.239.35.12
  #define NTP_SERVER4  "time.apple.com"
#endif
#ifndef NTP_SERVER
  #define NTP_SERVER  "time.google.com"
#endif
#define NTP_TZ_ENV_VARIABLE "CET-1CEST,M3.5.0,M10.5.0/3"  //POSIX.1 format for Europe/Madrid TZ env variable
#ifndef NTP_TZ_ENV_VARIABLE //Use GNUB Time Zone format if not POSI.1 one is provided with
  #define GMT_OFFSET_SEC 3600
  #define DAYLIGHT_OFFSET_SEC 7200 //3600 for CEST
#endif
#define UPLOAD_SAMPLES_ENABLED true
#define UPLOAD_SAMPLES_FROM_SITE "home"
#if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE
  #define SERVER_UPLOAD_SAMPLES  "10.88.50.5"
#endif
#if BUILD_ENV_NAME==BUILD_TYPE_DEVELOPMENT
  #define SERVER_UPLOAD_SAMPLES  "10.88.50.5"
#endif
#ifndef SERVER_UPLOAD_SAMPLES
  #define SERVER_UPLOAD_SAMPLES "195.201.42.50"
#endif
#define SERVER_UPLOAD_PORT  80
//GET /lar-co2/?device=co2-sensor-XXXXXX&local_ip_address=192.168.100.192&co2=543&temp_by_co2_sensor=25.6&hum_by_co2_sensor=55&temp_co2_sensor=28.7&....
#define GET_REQUEST_TO_UPLOAD_SAMPLES  "GET /lar-co2/?"
#define WIFI_100_RSSI -60  //RSSI > -60 dBm Excellent - Consider 100% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_075_RSSI -70  //RSSI > -70 dBm Very Good - Consider 75% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_050_RSSI -80  //RSSI > -80 dBm Good - Consider 50% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_025_RSSI -90  //RSSI > -90 dBm Low - Consider 25% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html
#define WIFI_000_RSSI -100 //RSSI < -100 dBm No Signal - Lower values mean no SSID visibiliy, 0% signal strength - https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html

//BLE stuff
#define BLE_ENABLED  true

//Battery stuff
#define SAVING_BATTERY_MODE  reducedEnergy //Other values: reducedEnergy lowestEnergy
#define BAT_ADC_PIN 34
#define POWER_ENABLE_PIN  14
#define BAT_CHECK_ENABLE HIGH
#define BAT_CHECK_DISABLE LOW
#define VOLTAGE_TH_STATE  2150 //mv - Threshold to consider USB or BAT power
#define ADC_SAMPLES 20
#define BAT_ADC_MAX 2100  //Max Battery voltage divide by 2 (there is a voltage divisor in the board) - mv
#define BAT_ADC_MIN 1550  //Min Battery voltage divide by 2 (there is a voltage divisor in the board) - mv
#ifdef __MHZ19B__ 
  #undef  MIN_VOLT
  #define MIN_VOLT  MH_Z19B_MIN_VOLT
#endif
#ifndef MIN_VOLT
  #define MIN_VOLT  1850  //Min voltage for getting right CO2 sample - millivolts
#endif
#define BAT_CHG_THR_FOR_SAVE_ENERGY 25  //Bat. charge threshold to enter in saving energy mode

//Timers and Global stuff
#define INITIAL_BOOTIME 25  //milliseconds - Time to bootup as per board measurements
#define uS_TO_S_FACTOR  1000000
#define BOOTUP_TIMEOUT  7  //Seconds. Timeout to leave bootup screen
#define BOOTUP_TIMEOUT2 50 //Seconds. Timeout to leave bootup screen after scrolling UP/DOWN
#define DISPLAY_MODE_REFRESH_PERIOD 5000 //milliseconds
#define DISPLAY_REFRESH_PERIOD      5000 //milliseconds
#define FULL_CHARGE_TIME 9000000 //Milliseconds for 100% charge 9000000=2h30m
#define HTTP_ANSWER_TIMEOUT 7000  //Millisenconds
#define ICON_STATUS_REFRESH_PERIOD  DISPLAY_REFRESH_PERIOD  //milliseconds
#define NTP_KO_CHECK_PERIOD  60000 //Milliseconds. 1 minute
#define NTP_CHECK_TIMEOUT     5000  //Millisecons. Should have NTP anser within 2 sc.
#define POWER_ENABLE_DELAY 50 //250 //Milliseconds
#define SAMPLE_PERIOD          20000  //milliseconds - Full Energy Mode (USB powered)
#define SAMPLE_PERIOD_RE       60000  //milliseconds - 1 min in Reduce Energy Mode (BAT powered)
#define SAMPLE_PERIOD_SE      300000  //milliseconds - 5 mim in Saving Energy Mode
#define SAMPLE_T_LAST_HOUR     20 //Seconds - Period of last hour samples to be recorded
#define SAMPLE_T_LAST_HOUR_RE  60 //Seconds - Period of last hour samples to be recorded in Reduced Energy Mode (BAT powered)
#define SAMPLE_T_LAST_HOUR_SE  300 //Seconds - Period of last hour samples to be recorded in Save Energy Mode (BAT powered)
#define SAMPLE_T_LAST_DAY  450 //Seconds - Period of last day samples to be recorded
#define TIME_LONG_PRESS_BUTTON1_HIBERNATE  5000 // 
#define TIME_LONG_PRESS_BUTTON2_TOGGLE_BACKLIGHT  5000 // 
#define TIME_TO_SLEEP_FULL_ENERGY 5*uS_TO_S_FACTOR 
#define TIME_TO_SLEEP_REDUCED_ENERGY 60*uS_TO_S_FACTOR 
#define TIME_TO_SLEEP_SAVE_ENERGY 300*uS_TO_S_FACTOR 
#define TIME_TURN_OFF_BACKLIGHT 30000 //millisenconds
#define UPLOAD_SAMPLES_PERIOD 300000  //millisenconds - 5 min
#define UPLOAD_SAMPLES_PERIOD_RE UPLOAD_SAMPLES_PERIOD  //millisenconds - 5 min in Reduce Energy Mode
#define UPLOAD_SAMPLES_PERIOD_SE UPLOAD_SAMPLES_PERIOD  //millisenconds - 5 min in Save Energy Mode
#define VOLTAGE_CHECK_PERIOD 5000 //Milliseconds - 5 s
#define VOLTAGE_CHECK_PERIOD_RE 300000 //Milliseconds - 5 min in Reduced Energy Mode
#define VOLTAGE_CHECK_PERIOD_SE 300000 //Milliseconds - 5 min in Save Energy Mode
#define WIFI_RECONNECT_PERIOD  300000 //milliseconds - 5 min

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

    typedef struct {
      String wifiSSIDs[3];
      String wifiPSSWs[3];
      String wifiSITEs[3];
      uint8_t activeIndex;
    } wifiCredentials;  //Struct to store user WiFi credentials    
    
    #define _WIFINETWORKINFO_ 
  #endif

  wifiNetworkInfo wifiNet;
  wifiCredentials wifiCred;
  String ntpServers[4];
  uint8_t ntpServerIndex;
  #ifdef NTP_TZ_ENV_VARIABLE
    String TZEnvVariable=String(NTP_TZ_ENV_VARIABLE);
  #endif
  
  #ifndef _DISPLAYSUPPORTINFO_
    enum displayModes {bootup,menu,sampleValue,co2LastHourGraph,co2LastDayGraph,AutoSwitchOffMessage};
    enum availableStates {bootupScreen,mainMenu,showOptMenu,infoMenu,infoMenu1,infoMenu2,infoMenu3,infoMenu4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential,configMenu,confMenuWifi,confMenuBLE,confMenuUpMeas,confMenuSavBatMode};
    RTC_DATA_ATTR enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status} wifiCurrentStatus;
    enum BLEStatus {BLEOnStatus,BLEConnectedStatus,BLEOffStatus};
    enum CloudClockStatus {CloudClockOnStatus,CloudClockOffStatus};
    enum CloudSyncStatus {CloudSyncOnStatus,CloudSyncOffStatus};
    #define _DISPLAYSUPPORTINFO_
  #endif

  #ifndef _BATTERYFRAMEWORK_
    #include "esp_adc_cal.h"
    RTC_DATA_ATTR float_t batADCVolt,lastBatCharge,batCharge; //3*4=12 B
    adc_atten_t attenuationDb;
    static esp_adc_cal_characteristics_t adc1_chars;
    enum powerModes {off,chargingUSB,onlyBattery,noChargingUSB};
    enum batteryChargingStatus {batteryCharging000,batteryCharging010,batteryCharging025,batteryCharging050,
                                batteryCharging075,batteryCharging100,
                                battery000,battery010,battery025,battery050,battery075,battery100};
    enum energyModes {fullEnergy, reducedEnergy, lowestEnergy};
    #define _BATTERYFRAMEWORK_
  #endif

  #ifndef _BUTTONSFRAMEWORK_
    enum callingAction {mainloop,ntpcheck,wificheck,webcheck};
    #define _BUTTONSFRAMEWORK_
  #endif

  #undef _DECLAREGLOBALPARAMETERS_
#endif




