#include <Arduino.h>
#include "ButtonChecks.h"
#include "display_support.h"

void checkButton1() {
  /*Serial.println("[B1] - Begin");
  Serial.print("  - currentState="); Serial.println(currentState);
  Serial.print("  - stateSelected="); Serial.println(stateSelected);
  Serial.print("  - lastState="); Serial.println(lastState);*/

  //If TFT is off, turn it on and exit;
  if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) {
      digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
      previousTurnOffBacklightTime=nowTime;
      gapTurnOffBacklight=0;

      //Display samples when back to light if displaying in sequential mode
      gapTimeDisplay=DISPLAY_REFRESH_PERIOD;
      if (currentState==displayingSequential) displayMode=sampleValue;

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

  /*Serial.println("[B1] - End");
  Serial.print("  - currentState="); Serial.println(currentState);
  Serial.print("  - stateSelected="); Serial.println(stateSelected);
  Serial.print("  - lastState="); Serial.println(lastState);*/
}

void checkButton2() {
  /*Serial.println("[B2] - Begin");
  Serial.print("  - currentState="); Serial.println(currentState);
  Serial.print("  - stateSelected="); Serial.println(stateSelected);
  Serial.print("  - lastState="); Serial.println(lastState);*/

  //If TFT is off, turn it on and exit;
  if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) {
      digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
      previousTurnOffBacklightTime=nowTime;
      gapTurnOffBacklight=0;

      //Display samples when back to light if displaying in sequential mode
      gapTimeDisplay=DISPLAY_REFRESH_PERIOD;
      if (currentState==displayingSequential) displayMode=sampleValue;
      return;
  } else {
    previousTurnOffBacklightTime=nowTime;
    gapTurnOffBacklight=0;
  }

  
  //Actions are different based on the current state
  switch(currentState) {
    case menuGlobal:
      currentState=stateSelected;
      if (currentState==menuWhatToDisplay) {stateSelected=lastState; printMenuWhatToDisplay();}
      else if (currentState==displayInfo) {stateSelected=displayInfo1; printInfoMenu();}
      else if (currentState==displayingSampleFixed){gapTimeDisplay=DISPLAY_REFRESH_PERIOD;lastDisplayMode=menu;}
      else if (currentState==displayingCo2LastHourGraphFixed){gapTimeDisplay=DISPLAY_REFRESH_PERIOD;updateHourGraph=true;}
      else if (currentState==displayingCo2LastDayGraphFixed){gapTimeDisplay=DISPLAY_REFRESH_PERIOD;updateDayGraph=true;}
      else if (currentState==displayingSequential){gapTimeDisplay=DISPLAY_REFRESH_PERIOD;lastDisplayMode=menu;displayMode=sampleValue;}
    break;
    case menuWhatToDisplay:
      currentState=stateSelected;
      gapTimeDisplay=DISPLAY_REFRESH_PERIOD;
      gapTimeDisplayMode=DISPLAY_MODE_REFRESH_PERIOD;
      lastDisplayMode=menu;
      lastGapTime=SAMPLE_PERIOD;
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

  /*Serial.println("[B2] - End");
  Serial.print("  - currentState="); Serial.println(currentState);
  Serial.print("  - stateSelected="); Serial.println(stateSelected);
  Serial.print("  - lastState="); Serial.println(lastState);*/
}