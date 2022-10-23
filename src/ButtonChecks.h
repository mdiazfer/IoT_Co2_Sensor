#ifndef _DECLAREGLOBALPARAMETERS_
  #include "global_setup.h"
  
  #ifndef _DISPLAYSUPPORTINFO_
    enum displayModes {bootup,menu,sampleValue,co2LastHourGraph,co2LastDayGraph,AutoSwitchOffMessage};
    enum availableStates {bootupScreen,menuGlobal,menuWhatToDisplay,displayInfo,displayInfo1,displayInfo2,displayInfo3,displayInfo4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential};
    
    #define _DISPLAYSUPPORTINFO_
  #endif
#endif

extern enum availableStates currentState,lastState,stateSelected;
extern enum displayModes displayMode,lastDisplayMode;
extern unsigned long lastGapTime,gapTimeDisplay,gapTimeDisplayMode,nowTime,previousTurnOffBacklightTime,gapTurnOffBacklight;
extern boolean autoBackLightOff,updateHourGraph,updateDayGraph;

void checkButton1();
void checkButton2();