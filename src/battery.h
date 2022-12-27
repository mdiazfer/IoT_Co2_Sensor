#ifndef _DECLAREGLOBALPARAMETERS_
  #include <Arduino.h>
  #include "global_setup.h"
  #include "user_setup.h"
  #include "esp_adc_cal.h"  

  #ifndef _BATTERYFRAMEWORK_
    enum powerModes {off,chargingUSB,onlyBattery,noChargingUSB};
    enum batteryChargingStatus {batteryCharging000,batteryCharging010,batteryCharging025,batteryCharging050,
                                batteryCharging075,batteryCharging100,
                                battery000,battery010,battery025,battery050,battery075,battery100};
    #define _BATTERYFRAMEWORK_ 
  #endif
#endif

extern float_t batADCVolt,lastBatCharge,batCharge;
extern adc_atten_t attenuationDb;
extern esp_adc_cal_characteristics_t adc1_chars;
extern enum powerModes powerState,lastPowerState;
extern enum batteryChargingStatus batteryStatus;
extern bool logsOn;

//Function declarations
enum batteryChargingStatus getBatteryStatus(float_t batADCVolt, ulong timeLast);
void initVoltageArray();
float_t getChargePercentage(double_t milliVolts);
void updateBatteryVoltageAndStatus(ulong nowTime, ulong *timeUSBPower);