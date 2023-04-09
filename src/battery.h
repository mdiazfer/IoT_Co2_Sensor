/* Dealing with Battery stuff

There are 4 different Saving Energy modes:
 - fullEnergy: Active if USB is powering the device. CPU is always UP and Running.
 - reducedEnergy. Active if BAT is powering the device. CPU goes to sleep and wakenup after a TIMER
 - lowestEnergy. BAT is powering the device and the charge is below BAT_CHG_THR_FOR_SAVE_ENERGY (25%).
                 CPU goes to sleep and wakenup after a TIMER. User can also setup this on the Config Menu
 - hibernate. Active is Button1 is pressed longer than TIME_LONG_PRESS_BUTTON1_HIBERNATE (5 s). CPU goes to hibernate (Switch off)

 Action Timers depend on the Saving Energy Mode.

.                                                 fullEnergy                   reduceEnergy                    lowestEnergy
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 TFT Backlight Check                  |     TIME_TURN_OFF_BACKLIGHT    |            same              |            same                |
 (TIME_TURN_OFF_BACKLIGHT)            |               30 s             |                              |                                |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Battery Level                        |     VOLTAGE_CHECK_PERIOD       |     With Sample Uploads      |      With Sample Uploads       |
 (VOLTAGE_CHECK_PERIOD)               |               5 s              |            5 min             |             5 min              |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Taking Measurement                   |          SAMPLE_PERIOD         |       SAMPLE_PERIOD_RE       |       SAMPLE_PERIOD_RE         |
 (SAMPLE_PERIOD)                      |              20 s              |             60 s             |             5 min              |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Checking WiFi signal strength        |   ICON_STATUS_REFRESH_PERIOD   |  NOT DONE if Display is off  |   NOT DONE if Display is off   |
 (ICON_STATUS_REFRESH_PERIOD)         |               5 s              |                              |                                |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Select the screen (only Seq. Mode)   |   DISPLAY_MODE_REFRESH_PERIOD  |  NOT DONE if Display is off  |   NOT DONE if Display is off   |
 (DISPLAY_MODE_REFRESH_PERIOD)        |               5 s              |                              |                                |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Update Screen                        |     DISPLAY_REFRESH_PERIOD     |  NOT DONE if Display is off  |   NOT DONE if Display is off   |
 (DISPLAY_REFRESH_PERIOD)             |               5 s              |                              |                                |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Checking WiFi Connection             |     WIFI_RECONNECT_PERIOD      |            same              |            same                |
 (WIFI_RECONNECT_PERIOD)              |              5 min             |                              |                                |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Checking NTP Connection              |     NTP_KO_CHECK_PERIOD        |            same              |            same                |
 (NTP_KO_CHECK_PERIOD)                |           60 s (random)        |     5 min as needs WiFi      |      5 min as needs WiFi       |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Upload Measurements to the server    |     UPLOAD_SAMPLES_PERIOD      |   UPLOAD_SAMPLES_PERIOD_RE   |   TIME_TO_SLEEP_SAVE_ENERGY_SE |
 (UPLOAD_SAMPLES_PERIOD)              |       5 min (WiFi needed)      |       5 min (WiFi needed)    |       5 min (WiFi needed)      |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
 Wakeup Period                        |   TIME_TO_SLEEP_FULL_ENERGY    | TIME_TO_SLEEP_REDUCED_ENERGY |   TIME_TO_SLEEP_SAVE_ENERGY    |
 (sleepTimer)                         |   5 s  (No sleep currently)    |             60 s             |            5 min               |
+-------------------------------------+--------------------------------+------------------------------+--------------------------------+
*/

#ifndef _DECLAREGLOBALPARAMETERS_
  #include <Arduino.h>
  #include <TFT_eSPI.h>
  #include "global_setup.h"
  #include "user_setup.h"
  #include "esp_adc_cal.h"

  #ifndef _DISPLAYSUPPORTINFO_
    enum displayModes {bootup,bootAP,menu,sampleValue,co2LastHourGraph,co2LastDayGraph,AutoSwitchOffMessage};
    enum availableStates {bootupScreen,bootAPScreen,mainMenu,showOptMenu,infoMenu,infoMenu1,infoMenu2,infoMenu3,infoMenu4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential,configMenu,confMenuWifi,confMenuBLE,confMenuUpMeas,confMenuSavBatMode,factResetMenu,factReset};
    enum CloudClockStatus {CloudClockOnStatus,CloudClockOffStatus};
    enum CloudSyncStatus {CloudSyncOnStatus,CloudSyncOffStatus};
    //#define _DISPLAYSUPPORTINFO_
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


extern adc_atten_t attenuationDb;
extern esp_adc_cal_characteristics_t adc1_chars;
extern bool debugModeOn;
extern TFT_eSPI tft;
extern RTC_DATA_ATTR float_t batADCVolt,lastBatCharge,batCharge;
extern RTC_DATA_ATTR enum powerModes powerState;
extern RTC_DATA_ATTR enum batteryChargingStatus batteryStatus;
extern RTC_DATA_ATTR enum energyModes energyCurrentMode,configSavingEnergyMode;
extern RTC_DATA_ATTR boolean autoBackLightOff,forceDisplayModeRefresh,forceDisplayRefresh,
                              forceGetSample,forceWifiReconnect,forceWebServerInit;
extern RTC_DATA_ATTR enum availableStates stateSelected,currentState;
extern RTC_DATA_ATTR uint64_t nowTimeGlobal,previousLastTimeSampleCheck,lastTimeTurnOffBacklightCheck;
extern RTC_DATA_ATTR enum displayModes displayMode,lastDisplayMode;
extern RTC_DATA_ATTR ulong voltageCheckPeriod,samplePeriod,uploadSamplesPeriod; //3*4=12B
extern RTC_DATA_ATTR  enum CloudClockStatus CloudClockCurrentStatus;
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus;
extern IPAddress serverToUploadSamplesIPAddress;
extern bool initTZVariables();
extern IPAddress stringToIPAddress(String stringIPAddress);
extern esp_sleep_wakeup_cause_t wakeup_reason;


//Function declarations
enum batteryChargingStatus getBatteryStatus(float_t batADCVolt, ulong timeLast);
void initVoltageArray();
float_t getChargePercentage(double_t milliVolts);
void updateBatteryVoltageAndStatus(uint64_t nowTime, uint64_t *timeUSBPower);