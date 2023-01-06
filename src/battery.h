#ifndef _DECLAREGLOBALPARAMETERS_
  #include <Arduino.h>
  #include <TFT_eSPI.h>
  #include "global_setup.h"
  #include "user_setup.h"
  #include "esp_adc_cal.h"  

  #ifndef _DISPLAYSUPPORTINFO_
    enum displayModes {bootup,menu,sampleValue,co2LastHourGraph,co2LastDayGraph,AutoSwitchOffMessage};
    enum availableStates {bootupScreen,menuGlobal,menuWhatToDisplay,displayInfo,displayInfo1,displayInfo2,displayInfo3,displayInfo4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential};
    enum CloudClockStatus {CloudClockOnStatus,CloudClockOffStatus};
  #endif

  #ifndef _BATTERYFRAMEWORK_
    enum powerModes {off,chargingUSB,onlyBattery,noChargingUSB};
    enum batteryChargingStatus {batteryCharging000,batteryCharging010,batteryCharging025,batteryCharging050,
                                batteryCharging075,batteryCharging100,
                                battery000,battery010,battery025,battery050,battery075,battery100};
    enum energyModes {fullEnergy, reducedEnergy, saveEnergy};
    #define _BATTERYFRAMEWORK_ 
  #endif
#endif


extern adc_atten_t attenuationDb;
extern esp_adc_cal_characteristics_t adc1_chars;
extern bool logsOn;
extern TFT_eSPI tft;
extern RTC_DATA_ATTR float_t batADCVolt,lastBatCharge,batCharge;
extern RTC_DATA_ATTR enum powerModes powerState;
extern RTC_DATA_ATTR enum batteryChargingStatus batteryStatus;
extern RTC_DATA_ATTR enum energyModes energyCurrentMode;
extern RTC_DATA_ATTR boolean autoBackLightOff,forceDisplayModeRefresh,forceDisplayRefresh,forceGetSample;
extern RTC_DATA_ATTR enum availableStates stateSelected,currentState;
extern RTC_DATA_ATTR ulong nowTimeGlobal,previousLastTimeSampleCheck,lastTimeTurnOffBacklightCheck;
extern RTC_DATA_ATTR enum displayModes displayMode,lastDisplayMode;


//Function declarations
enum batteryChargingStatus getBatteryStatus(float_t batADCVolt, ulong timeLast);
void initVoltageArray();
float_t getChargePercentage(double_t milliVolts);
void updateBatteryVoltageAndStatus(ulong nowTime, ulong *timeUSBPower);