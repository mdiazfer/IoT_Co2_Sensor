#include <Arduino.h>
#include "ButtonChecks.h"
#include "display_support.h"

void checkButton1() {
  //If TFT is off, turn it on and exit;
  if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) {
    digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
    lastTimeTurnOffBacklightCheck=nowTimeGlobal;
    #if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE_2
      digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
      float_t batADCVolt=0; for (u_int8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
      digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume
      if (batADCVolt >= VOLTAGE_TH_STATE) calibrationNextState=usbOnDisplayOnTransition;
      else calibrationNextState=usbOffDisplayOnTransition;
      lastCalibrationStateChange=loopStartTime+millis();
      getLocalTime(&lastCalibrationStateChangeTimeInfo);
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [checkButton1] - State change. calibrationCurrentState="+String(calibrationCurrentState)+", calibrationNextState="+String(calibrationNextState)+", batADCVolt="+String(batADCVolt)+", lastCalibrationStateChange="+String(lastCalibrationStateChange)+", time to chage status="+String(lastCalibrationStateChange+transitionEndTime[calibrationNextState]-nowTimeGlobal));}
    #endif

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
      if (fullEnergy!=energyCurrentMode) { //Force things after wakeup
        forceWifiReconnect=true; //Force WiFi reconection after wakeup
        forceWebServerInit=true; //v0.9.D - Next WiFi reconnection, force Web Server Init after waking up from sleep
        initTZVariables(); //v0.9.D - To make sure that both NTP sync and NTP info in web are right
        CloudClockCurrentStatus=CloudClockOffStatus; //v0.9.D - To update icons as WiFi is disconnect
        CloudSyncCurrentStatus=CloudSyncOffStatus; //v0.9.D - To update icons as WiFi is disconnect
      }
    }

    return;
  }

  //Actions are different based on the current state
  switch(currentState) {
    case bootupScreen:
    case bootAPScreen:
    break;
    case mainMenu:
    //Changing Menus: Main Menu -> What to Show Menu -> Display Gral. Info -> back
      switch(stateSelected) {
        case showOptMenu:
          stateSelected=infoMenu;
        break;
        case infoMenu:
          stateSelected=configMenu;
        break;
        case configMenu:
          stateSelected=factResetMenu;
        break;
        case factResetMenu:
          stateSelected=lastState;
        break;
        default:
          stateSelected=showOptMenu;
        break;
      }
      printMainMenu();
    break;
    case factResetMenu:
      switch(stateSelected) {
        case factReset:
          stateSelected=mainMenu;
        break;
        case factResetMenu:
        default:
          stateSelected=factReset;
        break;
      }
      printFactoryResetMenu();
    break;
    case showOptMenu:
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
      printshowOptMenu();
    break;
    case infoMenu:
      switch(stateSelected) {
          case infoMenu1:
            stateSelected=infoMenu2;
          break;
          case infoMenu2:
            stateSelected=infoMenu3;
          break;
          case infoMenu3:
            stateSelected=infoMenu4;
          break;
          case infoMenu4:
            stateSelected=mainMenu;
          break;
          default:
            stateSelected=infoMenu1;
          break;
        }
        printInfoMenu();
    break;
    case infoMenu1:
    case infoMenu2:
    case infoMenu3:
    case infoMenu4:
    break;
    case configMenu:
      switch (stateSelected) {
        case confMenuWifi:
          stateSelected=confMenuBLE;
        break;
        case confMenuBLE:
          if (wifiEnabled) stateSelected=confMenuUpMeas; //Only if WiFi is enabled
          else stateSelected=confMenuSavBatMode;
        break;
        case confMenuUpMeas:
          stateSelected=confMenuSavBatMode;
        break;
        case confMenuSavBatMode:
          stateSelected=mainMenu;
        break;
        default:
          stateSelected=confMenuWifi;
        break;
      }
      printConfigMenu();
    break;
    default:
      //Landing here after Button1 pressed from regular screen (sample or graphs)
      //lastState is the screen which came from
      if (debugModeOn) {Serial.println("    - [checkButton1] - B1, antes de cambiar, lastState="+String(lastState));}
      lastState=currentState;
      if (debugModeOn) {Serial.println("    - [checkButton1] - B1, despues de cambiar, lastState="+String(lastState));}
      stateSelected=showOptMenu;
      currentState=mainMenu;
      printMainMenu();
    break;
  }
}

void checkButton2() {
  //If TFT is off, turn it on and exit;
  if (digitalRead(PIN_TFT_BACKLIGHT)==LOW) {
      digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
      lastTimeTurnOffBacklightCheck=nowTimeGlobal;
      #if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE_2
        digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
        float_t batADCVolt=0; for (u_int8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
        digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume
        if (batADCVolt >= VOLTAGE_TH_STATE) calibrationNextState=usbOnDisplayOnTransition;
        else calibrationNextState=usbOffDisplayOnTransition;
        lastCalibrationStateChange=loopStartTime+millis();
        getLocalTime(&lastCalibrationStateChangeTimeInfo);
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [checkButton2] - State change. calibrationCurrentState="+String(calibrationCurrentState)+", calibrationNextState="+String(calibrationNextState)+", batADCVolt="+String(batADCVolt)+", lastCalibrationStateChange="+String(lastCalibrationStateChange)+", time to chage status="+String(lastCalibrationStateChange+transitionEndTime[calibrationNextState]-nowTimeGlobal));}
      #endif

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
        if (fullEnergy!=energyCurrentMode) { //Force things after wakeup
          forceWifiReconnect=true; //Force WiFi reconection after wakeup
          forceWebServerInit=true; //v0.9.D - Next WiFi reconnection, force Web Server Init after waking up from sleep
          initTZVariables(); //v0.9.D - To make sure that both NTP sync and NTP info in web are right
          CloudClockCurrentStatus=CloudClockOffStatus; //v0.9.D - To update icons as WiFi is disconnect
          CloudSyncCurrentStatus=CloudSyncOffStatus; //v0.9.D - To update icons as WiFi is disconnect
        }
      }

      return;
  } else {
    lastTimeTurnOffBacklightCheck=nowTimeGlobal;
  }

  
  //Actions are different based on the current state
  switch(currentState) {
    case mainMenu:
      currentState=stateSelected;
      if (currentState==showOptMenu) {stateSelected=lastState; printshowOptMenu();}
      else if (currentState==infoMenu) {stateSelected=infoMenu1; printInfoMenu();}
      else if (currentState==configMenu) {stateSelected=confMenuWifi;printConfigMenu();}
      else if (currentState==factResetMenu) {stateSelected=mainMenu;printFactoryResetMenu();} //tateSelected=mainMenu to start in CANCEL
      else if (currentState==displayingSampleFixed){forceDisplayRefresh=true;lastDisplayMode=menu;}
      else if (currentState==displayingCo2LastHourGraphFixed){forceDisplayRefresh=true;updateHourGraph=true;}
      else if (currentState==displayingCo2LastDayGraphFixed){forceDisplayRefresh=true;updateDayGraph=true;}
      else if (currentState==displayingSequential){forceDisplayRefresh=true;forceDisplayModeRefresh=true;lastDisplayMode=menu;displayMode=sampleValue;}
    break;
    case factResetMenu:
      currentState=stateSelected;
      if (currentState==mainMenu) {currentState=mainMenu;stateSelected=lastState;printMainMenu();}
      else {
        //Reset config to Factory values and save in EEPROM
        factoryConfReset();
        EEPROM.commit();
        delay(00); //Time to rightly write EEPROM
        ESP.restart();
      }
    break;
    case showOptMenu:
      currentState=stateSelected;
      forceDisplayRefresh=true;
      forceDisplayModeRefresh=true;
      lastDisplayMode=menu;
      previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD; //Refresh the circular graph for CO2 sample
    break;
    case infoMenu:
      currentState=stateSelected;
      stateSelected=infoMenu;
      if (currentState==infoMenu1) printGralInfo();
      else if (currentState==infoMenu2) printSensorsInfo();
      else if (currentState==infoMenu3) printWifiInfo();
      else if (currentState==infoMenu4) printNetInfo();
      else printMainMenu();
    break;
    case infoMenu1:
      currentState=stateSelected;
      stateSelected=infoMenu1;
      printInfoMenu();
    break;
    case infoMenu2:
      currentState=stateSelected;
      stateSelected=infoMenu2;
      printInfoMenu();
    break;
    case infoMenu3:
      currentState=stateSelected;
      stateSelected=infoMenu3;
      printInfoMenu();
    break;
    case infoMenu4:
      currentState=stateSelected;
      stateSelected=infoMenu4;
      printInfoMenu();
    break;
    case configMenu:
      if (stateSelected==confMenuWifi) {wifiEnabled=!wifiEnabled;if (!wifiEnabled ) uploadSamplesEnabled=false; printConfigMenu();}
      else if (stateSelected==confMenuBLE) {bluetoothEnabled=!bluetoothEnabled;printConfigMenu();}
      else if (stateSelected==confMenuUpMeas) {uploadSamplesEnabled=!uploadSamplesEnabled;printConfigMenu();}
      else if (stateSelected==confMenuSavBatMode) {
        configSavingEnergyMode=configSavingEnergyMode==reducedEnergy?lowestEnergy:reducedEnergy;
        //if currently lowestEnergy, don't change it as charge <= Threshold
        //if currently fullEnegegy, don't change it as USB is plugged
        if (reducedEnergy==energyCurrentMode) energyCurrentMode=configSavingEnergyMode; 
        printConfigMenu();
      }
      else { //Back
        currentState=lastState; 
        if (currentState==displayingSampleFixed){forceDisplayRefresh=true;lastDisplayMode=menu;}
        else if (currentState==displayingCo2LastHourGraphFixed){forceDisplayRefresh=true;updateHourGraph=true;}
        else if (currentState==displayingCo2LastDayGraphFixed){forceDisplayRefresh=true;updateDayGraph=true;}
        else if (currentState==displayingSequential){forceDisplayRefresh=true;forceDisplayModeRefresh=true;lastDisplayMode=menu;displayMode=sampleValue;}
        if (debugModeOn) {Serial.println("    - [checkButton2] - B2 lastState="+String(lastState)+", forceDisplayModeRefresh="+String(forceDisplayModeRefresh)+", forceDisplayRefresh="+String(forceDisplayRefresh)+", lastDisplayMode="+String(lastDisplayMode)+", displayMode="+String(displayMode));}

        //Actions based on the configuration
        if (wifiEnabled) { 
          if(WiFi.status()!=WL_CONNECTED) {
            forceWifiReconnect=true; //Next loop interaction the WiFi connection is done
            forceNTPCheck=true; //Let's force NTP sync
            forceWebServerInit=true; //v0.9.C - Next WiFi reconnection, force Web Server Init after waking up from sleep
          }
        }
        else {
          if(WiFi.status()!=WL_DISCONNECTED) {
            WiFi.disconnect();
            wifiCurrentStatus=wifiOffStatus;
            forceWifiReconnect=false; //To avoid deadlock in WIFI_RECONNECT_PERIOD chck if a previous WiFi reconnection was ongoing
            uploadSamplesEnabled=false; //To avoid uploading samples tries
            CloudClockCurrentStatus=CloudClockOffStatus;
          }
        }
        if (bluetoothEnabled) {if (BLECurrentStatus==BLEOffStatus) BLECurrentStatus=BLEStandbyStatus;}
        else {
          if ((BLECurrentStatus==BLEOnStatus) || (BLECurrentStatus==BLEStandbyStatus)) {
            BLECurrentStatus=BLEOffStatus;
            lastTimeBLEOnCheck=nowTimeGlobal-BLEOnTimeout; //To force stop BLE advertisings in the next loop cycle.
          }
        }
        if (uploadSamplesEnabled) {} //Do nothing else. CloudSyncCurrentStatus update and Upload will be done just right after WiFi (re-)connection.
        else {CloudSyncCurrentStatus=CloudSyncOffStatus;}//Do nothing else
        if (reducedEnergy==configSavingEnergyMode) {}//Do nothing esle. Update is done through the regular loop() flow
        else {} //Do nothing else

        //Writting the modified values in EEPROM
        //Reaching this point means the EEPROM bit (bit 0) for the very firs run is set (0x01).
        uint8_t currentConfigVariables=0x01;
        if (reducedEnergy==configSavingEnergyMode) currentConfigVariables|=0x02;
        if (uploadSamplesEnabled) currentConfigVariables|=0x04;
        if (bluetoothEnabled) currentConfigVariables|=0x08;
        if (wifiEnabled) currentConfigVariables|=0x10;
        if (webServerEnabled) currentConfigVariables|=0x20;
        if (configVariables!=currentConfigVariables) {
          // save the LED state in flash memory
            configVariables=currentConfigVariables;
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
        }
      }
    break;
    default:
    break;
  }
}

uint8_t checkButtonsActions(enum callingAction fromAction) {

  //Actions if button1 is pushed. It depens on the current state
  //Avoid this action the first loop interaction just right after wakeup by pressing a button
  //if (button1.pressed() && !buttonWakeUp && !button1Pressed) {
  if (button1.pressed() && !button1Pressed) { //No need to avoid the first loop interaction just right after wakeup by pressing a button since v0.9.3 as WiFi is no longer blocking buttons
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - button1.pressed, currentState="+String(currentState)+", buttonWakeUp="+String(buttonWakeUp)+", firstBoot="+String(firstBoot));}

    //Take time to check if it is long press
    button1Pressed=true;
    if (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed ||
        currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential)
      timePressButton1=millis();
    else
      timePressButton1=0;
  }

  //if (button1.released() && !buttonWakeUp) {
  if (button1.released()) { //No need to avoid the first loop interaction just right after wakeup by pressing a button since v0.9.3 as WiFi is no longer blocking buttons
    button1Pressed=false;

    checkButton1();
    //Specific action based on where this functions has been called from
    switch (fromAction) {
      case mainloop:
        //Do nothing else
      break;
      case ntpcheck:
      case wificheck:
      case mqttcheck:
        return(1);
      break;
      case askAPloop:
        return(4);
      break;
    }
  }

  //Check if Button1 was long pressed
  //Avoid this action the first loop interaction just right after wakeup by pressing a button
  //if (button1Pressed && timePressButton1!=0 && !buttonWakeUp) {
  if (button1Pressed && timePressButton1!=0) { //No need to avoid the first loop interaction just right after wakeup by pressing a button since v0.9.3 as WiFi is no longer blocking buttons

    if ((millis()-timePressButton1) > TIME_LONG_PRESS_BUTTON1_HIBERNATE) {
      //Long press, so toggle going to hibernate
      //Preparing to display message in the screen
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(TEXT_SIZE_MENU);
      tft.setTextColor(TFT_RED,TFT_BLACK);
      tft.setCursor(0,tft.height()/2-2*(tft.fontHeight(TEXT_FONT_MENU)+3),TEXT_FONT_MENU);tft.println  ("       Device");
      tft.setCursor(0,tft.height()/2-(tft.fontHeight(TEXT_FONT_MENU)+3),TEXT_FONT_MENU);tft.println("     Switch OFF");
      tft.setTextColor(TFT_GOLD,TFT_BLACK);
      tft.setCursor(0,tft.height()/2+10,TEXT_FONT_MENU);tft.println("  Button1 Switch ON");
      delay(3000);

      tft.fillScreen(TFT_BLACK);
      go_to_hibernate();
    } 
  }

  //Actions if button2 is pushed. It depens on the current state
  //Avoid this action the first loop interaction just right after wakeup by pressing a button
  //nowTimeGlobal=loopStartTime+millis();
  //if (button2.pressed() && !buttonWakeUp) {
  if (button2.pressed()) { //No need to avoid the first loop interaction just right after wakeup by pressing a button since v0.9.3 as WiFi is no longer blocking buttons
    if (debugModeOn) {String(loopStartTime+millis())+Serial.println("  - button2.pressed");}

    //Take time to check if it is long press
    button2Pressed=true;
    if (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed ||
        currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential)
      timePressButton2=millis();
    else
      timePressButton2=0;
    checkButton2();
    if (debugModeOn) {Serial.println("  - [checkButtonsActions]  - B2 lastState="+String(lastState)+", forceDisplayRefresh="+String(forceDisplayRefresh)+", lastDisplayMode="+String(lastDisplayMode)+", displayMode="+String(displayMode));}

    //Specific actions based on where this functions has been called from
    switch (fromAction) {
      case mainloop:
        //Do nothing else
      break;
      case ntpcheck:
      case wificheck:
      case mqttcheck:
        return(2);
      break;
      case askAPloop:
        return(5);
      break;
    }
  }

  //if (button2.released() && !buttonWakeUp) {
  if (button2.released()) { //No need to avoid the first loop interaction just right after wakeup by pressing a button since v0.9.3 as WiFi is no longer blocking buttons
    button2Pressed=false;

    //Specific actions based on where this functions has been called from
    switch (fromAction) {
      case mainloop:
      case askAPloop:
        //Do nothing else
      break;
      case ntpcheck:
      case wificheck:
      case mqttcheck:
        return(3);
      break;
    }
  }

  //Check if Button2 was long pressed
  //Avoid this action the first loop interaction just right after wakeup by pressing a button
  //if (button2Pressed && timePressButton2!=0 && !buttonWakeUp) { 
  if (button2Pressed && timePressButton2!=0) { //No need to avoid the first loop interaction just right after wakeup by pressing a button since v0.9.3 as WiFi is no longer blocking buttons
    if ((millis()-timePressButton2) > TIME_LONG_PRESS_BUTTON2_TOGGLE_BACKLIGHT) {
      //Long press, so toggle autoBackLightOff and display message
      autoBackLightOff=!autoBackLightOff;
      timePressButton2=0;
      lastDisplayMode=AutoSwitchOffMessage;

      //Preparing to display message in the screen
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(TEXT_SIZE_MENU);
      tft.setTextColor(TFT_GOLD,TFT_BLACK);
      tft.setCursor(0,tft.height()/2-2*(tft.fontHeight(TEXT_FONT_MENU)+3),TEXT_FONT_MENU);tft.println("      Display");
      tft.setCursor(0,tft.height()/2-(tft.fontHeight(TEXT_FONT_MENU)+3),TEXT_FONT_MENU);tft.println("  Auto switch off");
      
      if (autoBackLightOff) {
        //Display message in the screen
        tft.setTextColor(TFT_GREEN,TFT_BLACK);
        tft.setCursor(0,tft.height()/2+2,TEXT_FONT_MENU);tft.println("      Enabled");
        delay(2500);
        tft.fillScreen(TFT_BLACK);

        //Turn off back light
        digitalWrite(PIN_TFT_BACKLIGHT,LOW);
        lastTimeTurnOffBacklightCheck=loopStartTime+millis();
        #if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE_2
          digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
          float_t batADCVolt=0; for (u_int8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
          digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume
          if (batADCVolt >= VOLTAGE_TH_STATE) calibrationNextState=usbOnDisplayOffTransition;
          else calibrationNextState=usbOffDisplayOffTransition;
          lastCalibrationStateChange=loopStartTime+millis();
          getLocalTime(&lastCalibrationStateChangeTimeInfo);
          if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [checkButtonsActions] - State change. calibrationCurrentState="+String(calibrationCurrentState)+", calibrationNextState="+String(calibrationNextState)+", batADCVolt="+String(batADCVolt)+", lastCalibrationStateChange="+String(lastCalibrationStateChange)+", time to chage status="+String(lastCalibrationStateChange+transitionEndTime[calibrationNextState]-nowTimeGlobal));}
        #endif
      }
      else {
        //Display message in the screen
        tft.setTextColor(TFT_RED,TFT_BLACK);
        tft.setCursor(0,tft.height()/2+2,TEXT_FONT_MENU);tft.println("     Disabled");
        delay(2500);
        tft.fillScreen(TFT_BLACK);

        #if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE_2
          digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
          float_t batADCVolt=0; for (u_int8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
          digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume
          if (batADCVolt >= VOLTAGE_TH_STATE) calibrationNextState=usbOnDisplayOnTransition;
          else calibrationNextState=usbOffDisplayOnTransition;
          lastCalibrationStateChange=loopStartTime+millis();
          getLocalTime(&lastCalibrationStateChangeTimeInfo);
          if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [checkButtonsActions] - State change. calibrationCurrentState="+String(calibrationCurrentState)+", calibrationNextState="+String(calibrationNextState)+", batADCVolt="+String(batADCVolt)+", lastCalibrationStateChange="+String(lastCalibrationStateChange)+", time to chage status="+String(lastCalibrationStateChange+transitionEndTime[calibrationNextState]-nowTimeGlobal));}
        #endif
      }
    }
  }

  return(0);
}
