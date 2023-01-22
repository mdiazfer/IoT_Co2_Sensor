/* Library for displaying graphs in the TFT

*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "wifiConnection.h"
#include "MHZ19.h"
#include "time.h"
#include "Button.h"
#include "ButtonChecks.h"
#include "wifiConnection.h"

#ifndef _DECLAREGLOBALPARAMETERS_
  #include "global_setup.h"
  
  #ifndef _DISPLAYSUPPORTINFO_
    enum availableStates {bootupScreen,mainMenu,showOptMenu,infoMenu,infoMenu1,infoMenu2,infoMenu3,infoMenu4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential,configMenu,confMenuWifi,confMenuBLE,confMenuUpMeas,confMenuSavBatMode};
    enum CloudClockStatus {CloudClockOnStatus,CloudClockOffStatus};
    
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
      uint8_t activeIndex;
    } wifiCredentials;  //Struct to store user WiFi credentials    

    extern wifiNetworkInfo wifiNet; //Struct to store WiFi parameters
    extern wifiCredentials wifiCred;
    extern String ntpServers[4];
    extern uint8_t ntpServerIndex;
    #define _WIFINETWORKINFO_ 
  #endif

  #ifndef _BATTERYFRAMEWORK_
    enum powerModes {off,chargingUSB,onlyBattery,noChargingUSB};
    enum batteryChargingStatus {batteryCharging000,batteryCharging010,batteryCharging025,batteryCharging050,
                                batteryCharging075,batteryCharging100,
                                battery000,battery010,battery025,battery050,battery075,battery100};
    enum energyModes {fullEnergy, reducedEnergy, lowestEnergy};
    #define _BATTERYFRAMEWORK_ 
  #endif
#endif

#ifdef __MHZ19B__
  extern MHZ19 co2Sensor;
#endif

extern TFT_eSPI tft;
extern const String tempHumSensorType,co2SensorType;
extern char co2SensorVersion[];
extern struct tm startTimeInfo;
extern Button button2;
extern IPAddress serverToUploadSamplesIPAddress;
extern boolean uploadSamplesEnabled;
extern RTC_DATA_ATTR uint64_t timeUSBPowerGlobal,nowTimeGlobal;
extern RTC_DATA_ATTR enum availableStates stateSelected,currentState;
extern RTC_DATA_ATTR  enum CloudClockStatus CloudClockCurrentStatus;
extern RTC_DATA_ATTR enum powerModes powerState;
extern RTC_DATA_ATTR enum batteryChargingStatus batteryStatus;
extern RTC_DATA_ATTR enum energyModes energyCurrentMode,configSavingEnergyMode;
extern RTC_DATA_ATTR float_t batADCVolt,lastBatCharge,batCharge;
extern RTC_DATA_ATTR float_t lastHourCo2Samples[3600/SAMPLE_T_LAST_HOUR];
extern RTC_DATA_ATTR float_t lastHourTempSamples[3600/SAMPLE_T_LAST_HOUR];
extern RTC_DATA_ATTR float_t lastHourHumSamples[3600/SAMPLE_T_LAST_HOUR];
extern RTC_DATA_ATTR float_t lastDayCo2Samples[24*3600/SAMPLE_T_LAST_DAY];
extern RTC_DATA_ATTR float_t lastDayTempSamples[24*3600/SAMPLE_T_LAST_DAY];
extern RTC_DATA_ATTR float_t lastDayHumSamples[24*3600/SAMPLE_T_LAST_DAY];

class HorizontalBar {
  public:
    int32_t xStart, yStart, xEnd, yEnd, width, high, arrowHigh;
    int32_t xValue, xValueMin, xValueMax, xTh1, xTh2;
    float_t value, valueMin, valueMax, valueRange, th1, th2;
    uint32_t normalColor, coldColor, hotColor, colorBar, colorBackground;

    HorizontalBar(float_t value, float_t valueMin, float_t valueMax, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color,
            float_t th1, uint32_t color1, float_t th2, uint32_t color2, uint32_t colorBar, uint32_t colorBackground);
    void drawHorizontalBar();
    void drawHorizontalBar(float_t value);
    void cleanHorizontalBar();

};

class CircularGauge {
  private:
    //Backwards compatibility variables
    float_t angleValue=value*angleRange/valueRange+angleStart,
            angleTh1=th1*angleRange/valueRange+angleStart,
            angleTh2=th2*angleRange/valueRange+angleStart;
    int32_t x, y;
    
  public:
    int32_t xCenter, yCenter, rExt, rInt, width, xStart, yStart, xEnd, yEnd, rDeltaMark=5;
    float_t value, valueMin, valueMax, valueRange, th1, th2;
    float_t valueAngle, valueMinAngle, valueMaxAngle, th1Angle, th2Angle, segmentAngle;
    float_t angleStart=90+segmentAngle,angleEnd=450-segmentAngle,angleRange=360-2*segmentAngle;
    uint32_t normalColor, warningColor, alarmColor, colorGauge, colorBackground, colorText, colorValue;
    String valueString, unitsString;
    int32_t xStartValueText=0, yStartValueText=0, xEndValueText=0, yEndValueText=0;
    int32_t xStartUnitsText=0, yStartUnitsText=0, xEndUnitsText=0, yEndUnitsText=0;

    CircularGauge(float_t value, float_t valueMin, float_t valueMax, int32_t x, int32_t y, int32_t r, int32_t width, float_t segment, uint32_t color,
                float_t th1, uint32_t color1, float_t th2, uint32_t color2, uint32_t colorGauge, uint32_t colorBackground);
    void setValue(float_t value);
    void drawGauge();
    void drawGauge(float_t value);
    void drawGauge1();
    void drawGauge1(float_t value);
    void drawGauge2();
    void drawGauge2(float_t value);
    void drawTextGauge(String unitsString, uint32_t valueTextSize=TEXT_SIZE, boolean isInteger=true,
                      uint32_t unitsTextSize=TEXT_SIZE_UNITS_CO2, uint32_t valueTextFont=TEXT_FONT,
                      uint32_t unitsTextFont=TEXT_FONT_UNITS_CO2, uint32_t colorText=0);
    void cleanGauge();
    void cleanValueTextGauge();
    void cleanUnitsTextGauge();
    void cleanAll();
};

void drawText(float_t value, String textString, int32_t textSize, int32_t font, uint32_t colorForeground,
              uint32_t colorBackground, int32_t x, int32_t y, 
              float_t th1, uint32_t warningColor, float_t th2, uint32_t alarmColor);

//Definition in display_support.cpp
void printMainMenu();
void printshowOptMenu();
void printInfoMenu();
void printConfigMenu();
void printGralInfo();
void printSensorsInfo();
void printWifiInfo();
void printNetInfo();
void loadBootImage();
void showIcons();
void loadAllIcons();
void loadAllWiFiIcons();
void drawGraphLastHourCo2();
void drawGraphLastDayCo2();

