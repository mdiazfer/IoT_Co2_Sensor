#include <Arduino.h>
#include "ButtonChecks.h"
#include "display_support.h"

void checkButton1() {
  //If TFT is off, turn it on and exit;
  if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) {
    digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
    lastTimeTurnOffBacklightCheck=nowTimeGlobal;

    //Display samples when back to light if displaying in sequential mode
    forceDisplayRefresh=true;
    forceDisplayModeRefresh=true;
    if (currentState==displayingSequential || currentState==displayingSampleFixed) {
      //Tune time counter to take actions before diplaying the active screen 
      forceGetVolt=true;                    //Update battery charge
      forceGetSample=true;                  //Take CO2, Temp, Hum sample
      previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD; //Refresh the circular graph for CO2 sample
      displayMode=sampleValue;
      lastDisplayMode=AutoSwitchOffMessage; //Force re-rendering CO2 values in the main screen
      if (fullEnergy!=energyCurrentMode) forceWifiReconnect=true; //Force WiFi reconection after wakeup
    }

    return;
  }

  //Actions are different based on the current state
  switch(currentState) {
    case bootupScreen:
    break;
    case menuGlobal:
    //Changing Menus: Global Menu -> What to display Menu -> Display Gra. Info -> back
      switch(stateSelected) {
        case menuWhatToDisplay:
          stateSelected=displayInfo;
        break;
        case displayInfo:
          stateSelected=lastState;
        break;
        default:
          stateSelected=menuWhatToDisplay;
        break;
      }
      printGlobalMenu();
    break;
    case menuWhatToDisplay:
    //Changing displayingMode: sampleFixed -> co2LastHourGraphFixe -> co2LastDayGraphFixed -> sequential
      switch(stateSelected) {
        case displayingSampleFixed:
          stateSelected=displayingCo2LastHourGraphFixed;
          displayMode=co2LastHourGraph;
        break;
        case displayingCo2LastHourGraphFixed:
          stateSelected=displayingCo2LastDayGraphFixed;
          displayMode=co2LastDayGraph;
        break;
        case displayingCo2LastDayGraphFixed:
          stateSelected=displayingSequential;
          displayMode=co2LastDayGraph;
        break;
        case displayingSequential:
          stateSelected=displayingSampleFixed;
          displayMode=sampleValue;
        break;
      }
      printMenuWhatToDisplay();
    break;
    case displayInfo:
      switch(stateSelected) {
          case displayInfo1:
            stateSelected=displayInfo2;
          break;
          case displayInfo2:
            stateSelected=displayInfo3;
          break;
          case displayInfo3:
            stateSelected=displayInfo4;
          break;
          case displayInfo4:
            stateSelected=menuGlobal;
          break;
          default:
            stateSelected=displayInfo1;
          break;
        }
        printInfoMenu();
    break;
    case displayInfo1:
    case displayInfo2:
    case displayInfo3:
    case displayInfo4:
    break;
    default:
      lastState=currentState;
      stateSelected=menuWhatToDisplay;
      currentState=menuGlobal;
      printGlobalMenu();
    break;
  }
}

void checkButton2() {
  //If TFT is off, turn it on and exit;
  if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) {
      digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
      lastTimeTurnOffBacklightCheck=nowTimeGlobal;

      //Display samples when back to light if displaying in sequential mode
      forceDisplayRefresh=true;
      forceDisplayModeRefresh=true;
      if (currentState==displayingSequential || currentState==displayingSampleFixed) 
      {
        //Tune time counter to take actions before diplaying the active screen
        forceGetVolt=true; //Update battery charge
        forceGetSample=true;                //Take CO2, Temp, Hum sample
        previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD; //Refresh the circular graph for CO2 sample
        displayMode=sampleValue;
        lastDisplayMode=AutoSwitchOffMessage; //Force re-rendering CO2 values in the main screen
        if (fullEnergy!=energyCurrentMode) forceWifiReconnect=true; //Force WiFi reconection after wakeup
      }

      return;
  } else {
    lastTimeTurnOffBacklightCheck=nowTimeGlobal;
  }

  
  //Actions are different based on the current state
  switch(currentState) {
    case menuGlobal:
      currentState=stateSelected;
      if (currentState==menuWhatToDisplay) {stateSelected=lastState; printMenuWhatToDisplay();}
      else if (currentState==displayInfo) {stateSelected=displayInfo1; printInfoMenu();}
      else if (currentState==displayingSampleFixed){forceDisplayRefresh=true;lastDisplayMode=menu;}
      else if (currentState==displayingCo2LastHourGraphFixed){forceDisplayRefresh=true;updateHourGraph=true;}
      else if (currentState==displayingCo2LastDayGraphFixed){forceDisplayRefresh=true;updateDayGraph=true;}
      else if (currentState==displayingSequential){forceDisplayRefresh=true;lastDisplayMode=menu;displayMode=sampleValue;}
    break;
    case menuWhatToDisplay:
      currentState=stateSelected;
      forceDisplayRefresh=true;
      forceDisplayModeRefresh=true;
      lastDisplayMode=menu;
      previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD; //Refresh the circular graph for CO2 sample
      //tft.fillScreen(MENU_BACK_COLOR);
    break;
    case displayInfo:
      currentState=stateSelected;
      stateSelected=displayInfo;
      if (currentState==displayInfo1) printInfoGral();
      else if (currentState==displayInfo2) printInfoSensors();
      else if (currentState==displayInfo3) printInfoWifi();
      else if (currentState==displayInfo4) printInfoNet();
      else printGlobalMenu();
    break;
    case displayInfo1:
      currentState=stateSelected;
      stateSelected=displayInfo1;
      printInfoMenu();
    break;
    case displayInfo2:
      currentState=stateSelected;
      stateSelected=displayInfo2;
      printInfoMenu();
    break;
    case displayInfo3:
      currentState=stateSelected;
      stateSelected=displayInfo3;
      printInfoMenu();
    break;
    case displayInfo4:
      currentState=stateSelected;
      stateSelected=displayInfo4;
      printInfoMenu();
    break;
    default:
    break;
  }
}