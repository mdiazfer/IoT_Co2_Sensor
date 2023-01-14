#include "Button.h"

#ifndef _DECLAREGLOBALPARAMETERS_
  #include "global_setup.h"
  
  #ifndef _DISPLAYSUPPORTINFO_
    enum displayModes {bootup,menu,sampleValue,co2LastHourGraph,co2LastDayGraph,AutoSwitchOffMessage};
    enum availableStates {bootupScreen,mainMenu,showOptMenu,infoMenu,infoMenu1,infoMenu2,infoMenu3,infoMenu4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential,configMenu,confMenuWifi,confMenuBLE,confMenuUpMeas,confMenuSavBatMode};
    enum CloudClockStatus {CloudClockOnStatus,CloudClockOffStatus};
    enum CloudSyncStatus {CloudSyncOnStatus,CloudSyncOffStatus};
    enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
    enum BLEStatus {BLEOnStatus,BLEConnectedStatus,BLEOffStatus};
    #define _DISPLAYSUPPORTINFO_
  #endif

  #ifndef _BUTTONSFRAMEWORK_
    enum callingAction {mainloop,ntpcheck,wificheck,webcheck};
    #define _BUTTONSFRAMEWORK_
  #endif
#endif

extern enum availableStates currentState,lastState,stateSelected;
extern ulong timePressButton1,timeReleaseButton1,timePressButton2,timeReleaseButton2;
extern RTC_DATA_ATTR enum displayModes displayMode,lastDisplayMode;
extern RTC_DATA_ATTR uint64_t lastTimeSampleCheck,lastTimeDisplayCheck,lastTimeDisplayModeCheck,nowTimeGlobal,lastTimeTurnOffBacklightCheck,lastTimeIconStatusRefreshCheck,previousLastTimeSampleCheck;
extern RTC_DATA_ATTR boolean autoBackLightOff,updateHourGraph,updateDayGraph;
extern RTC_DATA_ATTR boolean forceWifiReconnect,forceGetSample,forceGetVolt,forceDisplayRefresh,forceDisplayModeRefresh,
                              forceNTPCheck,buttonWakeUp;
extern RTC_DATA_ATTR uint64_t nowTimeGlobal,loopStartTime,loopEndTime;
extern RTC_DATA_ATTR boolean  button1Pressed,button2Pressed;
extern RTC_DATA_ATTR Button  button1; //16 B
extern RTC_DATA_ATTR Button  button2;
extern RTC_DATA_ATTR boolean debugModeOn;
extern RTC_DATA_ATTR boolean firstBoot;
extern RTC_DATA_ATTR boolean wifiEnabled,bluetoothEnabled,uploadSamplesEnabled;
extern RTC_DATA_ATTR enum BLEStatus BLEClurrentStatus;
extern RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus;


void checkButton1();
void checkButton2();
uint8_t checkButtonsActions(enum callingAction fromAction);
extern void go_to_hibernate();