/* Dealing with WiFi stuff

*/

#include "wifiConnection.h"
#include "esp_sntp.h"

#ifdef __TFT_DISPLAY_PRESENT__
  #include <TFT_eSPI.h>
  extern TFT_eSPI tft;
  extern TFT_eSprite stext1;
#endif

int status = WL_IDLE_STATUS;     // the Wifi radio's status

/******************************************************
 Function printNetData
 Target: prints Network parameters (@IP,@MAC, Default GW, Mask, DNS)
 *****************************************************/
void printNetData() {
  // print MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("  MAC address: ");
  Serial.print(mac[0], HEX); Serial.print(":");
  Serial.print(mac[1], HEX); Serial.print(":");
  Serial.print(mac[2], HEX); Serial.print(":");
  Serial.print(mac[3], HEX); Serial.print(":");
  Serial.print(mac[4], HEX); Serial.print(":");
  Serial.println(mac[5], HEX);

  // print IP address, etc.
  //IPAddress ip = WiFi.localIP();
  Serial.print("  IP Address : ");Serial.println(WiFi.localIP().toString());
  Serial.print("  Mask       : ");Serial.println(WiFi.subnetMask().toString());
  Serial.print("  Default GW : ");Serial.println(WiFi.gatewayIP().toString());
}

/******************************************************
 Function printCurrentWiFi
 Target: prints WiFi parameters on Serial and returs variable with Wifi parameters
 *****************************************************/
wifiNetworkInfo * printCurrentWiFi(boolean debugModeOn=true, int16_t *numberWiFiNetworks=nullptr) {
  // print current network parameters
  WiFiScanClass wifiScan = WiFiScanClass();
  int16_t countWiFiNetworks=wifiScan.scanNetworks();
  int16_t scanReturn=-1;
  while (scanReturn<0){
    scanReturn=wifiScan.scanComplete();
    switch (scanReturn) {
      case WIFI_SCAN_FAILED:
        if (debugModeOn) Serial.println("  [error] - Failed to scan WiFi networks");
        return nullptr;
      break;
      case WIFI_SCAN_RUNNING:
        //Waiting for the scan to finish
      break;
      default:
          //Serial.print("  Number of SSIDs detected: "); Serial.println(scanReturn);
          break;  //Scan finished. Exit the while loop
      break;
    }
  } //Wait till scan is finished

  *numberWiFiNetworks=scanReturn;

  int16_t wifiNetworkNode=-1;
  for (int16_t i=0; i<countWiFiNetworks; i++) {
    //String mySSID=String(WIFI_SSID_CREDENTIALS);
    String mySSID=wifiCred.wifiSSIDs[wifiCred.activeIndex];
    wifiScan.getNetworkInfo(i, wifiNet.ssid, wifiNet.encryptionType, wifiNet.RSSI, wifiNet.BSSID, wifiNet.channel);
    if (mySSID.compareTo(wifiNet.ssid) == 0) {
      //if (debugModeOn) {Serial.print(mySSID);Serial.print(" is equal than "); Serial.println(wifiNet.ssid);
      //             Serial.print("networkItem=");Serial.println(i);}
      wifiNetworkNode=i;
      break;
    }
    else {
      //if (debugModeOn) {Serial.print(mySSID);Serial.print(" is not equal than "); Serial.println(wifiNet.ssid);}
    }
  }

  if (-1==wifiNetworkNode) {
    //WiFi SSID was lost
    if (debugModeOn) {
      Serial.print("  SSID: ");Serial.print(wifiCred.wifiSSIDs[wifiCred.activeIndex]);Serial.println(" lost");
    }
    return nullptr;
  }

  // print current network parameters
  if (debugModeOn) {
    Serial.print("  Wifi Network Node: ");Serial.print(wifiNetworkNode);
    Serial.print(" and "); Serial.print(scanReturn); Serial.println(" detected");
    //Serial.print("SSID: ");Serial.println(WiFi.SSID());
    Serial.print("  SSID: ");Serial.println(wifiNet.ssid);

    // print the MAC address of the router you're attached to:
    uint8_t bssid[6];
    //memcpy(bssid, WiFi.BSSID(), 6);
    memcpy(bssid, wifiNet.BSSID, 6);
    Serial.print("  BSSID: ");
    Serial.print(bssid[5], HEX); Serial.print(":");
    Serial.print(bssid[4], HEX); Serial.print(":");
    Serial.print(bssid[3], HEX); Serial.print(":");
    Serial.print(bssid[2], HEX); Serial.print(":");
    Serial.print(bssid[1], HEX); Serial.print(":");
    Serial.println(bssid[0], HEX);

    // print the received signal strength:
    //Serial.print("signal strength (RSSI):");Serial.println((long) WiFi.RSSI());
    Serial.print("  Signal strength (RSSI): ");Serial.println(wifiNet.RSSI);

    // print the encryption type:
    Serial.print("  Encryption Type: ");Serial.println(wifiNet.encryptionType, HEX);

    // print the channel:
    Serial.print("  WiFi Channel: ");Serial.println(wifiNet.channel);
  }

  return &wifiNet;
}

/******************************************************
 Function wifiConnect
 Target: to get connected to the WiFi
 *****************************************************/
uint32_t wifiConnect(boolean debugModeOn=true, boolean msgTFT=true, boolean checkButtons=false, uint8_t* auxLoopCounter=nullptr, uint8_t* auxCounter=nullptr) {
  // Parameters:
  // - debugModeOn: Print or not log messages in Serial line. Diferent prints out are done base on its value
  //      Value false: no log messages are printed out
  //      Value true:  log messages are printed out
  // - msgTFT: Print or not log messages in the Display during the boot time. Diferent prints out are done base on its value
  //      Value false: no log messages are displayed out
  //      Value true:  log messages are displayed out
  // - checkButtons: Check if buttons are pressed or not to go through Menus
  //      Value false: no buttons checks
  //      Value true:  buttons are checked
  // - *auxLoopCounter is a pointer for the index of the SSID array to check. It's sent from the main loop
  //      Value 0: The SSID will start from the very first SSID - First call to the funcion
  //      Value other: The SSID will resume the connection with the same SSID used in the previous interaction
  //          which was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  // - *auxCounter is the counter for the while() loop to stop checking the index of he SSID array. It's sent from the main loop
  //      Value 0: The SSID connection will start from the beginig - First call to the funcion
  //      Value other: The SSID connection will resume the connection at the same point where the previous interaction
  //          was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  // *auxLoopCounter and *auxCounter are global variables. They are modified in here. The calling function
  //   just send them to this function.

  status = WL_IDLE_STATUS;
  uint8_t counter=0;
  boolean errorWifiConnection;
  
  //to avoid pointer issues
  if (auxLoopCounter==nullptr || auxCounter==nullptr) {
    errorsWiFiCnt++; //Something went wrong. Update error counter for stats
    return(ERROR_WIFI_SETUP);
  }

  // attempt to connect to Wifi network:
  for (uint8_t loopCounter=*auxLoopCounter; loopCounter<(uint8_t)sizeof(wifiCred.wifiSSIDs)/sizeof(String); loopCounter++) {
    counter=*auxCounter;
    errorWifiConnection=false;
    *auxLoopCounter=loopCounter;
    WiFi.begin(wifiCred.wifiSSIDs[loopCounter].c_str(), wifiCred.wifiPSSWs[loopCounter].c_str());

    long auxWhile=millis();
    while (status != WL_CONNECTED && counter < MAX_CONNECTION_ATTEMPTS) {
      if ((millis()-auxWhile)>=(MAX_CONNECTION_ATTEMPTS*1000/10)) { //Interval to show info and dots: MAX_CONNECTION_ATTEMPTS/10 *1000 milliseconds
        counter++;
        *auxCounter=counter;
        auxWhile=millis();
        if (debugModeOn) {Serial.print("[setup - wifi] Attempting to connect to WPA SSID: ");Serial.println(wifiCred.wifiSSIDs[loopCounter].c_str());}
        if (msgTFT) {
          #ifdef __TFT_DISPLAY_PRESENT__
            stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.print(".");
            stext1.pushSprite(0,(LINES_PER_TEXT_SCROLL-LINES_PER_TEXT_SPRITE)/2*tft.fontHeight(TEXT_FONT_BOOT_SCREEN));
          #endif
        }
      }
      //Check if buttons are pressed during SSID connection
      if (checkButtons) {
        switch (checkButtonsActions(wificheck)) {
          case 1:
          case 2:
          case 3:
            //Button1 or Button2 pressed or released. SSID connection is aborted if not SSID is connected
            if (WL_CONNECTED!=WiFi.status()) {
              if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - checkButtonsActions() returns 1, 2 or 3 - Returning with ERROR_ABORT_WIFI_SETUP");}
              return(ERROR_ABORT_WIFI_SETUP);
            }
          break;
          case 0:
          default:
            //Regular exit. Do nothing
          break;
        }
        //Must the Display be refreshed? This check avoids the display gets blocked during SSID connection
        if (digitalRead(PIN_TFT_BACKLIGHT)!=LOW &&
              (
                (((loopStartTime+millis()-lastTimeDisplayModeCheck) >= DISPLAY_MODE_REFRESH_PERIOD) && currentState==displayingSequential)
                ||
                (((loopStartTime+millis()-lastTimeDisplayCheck) >= DISPLAY_REFRESH_PERIOD) && 
                  (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || 
                  currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) )
              )
            ) {
          return (ERROR_BREAK_WIFI_SETUP); //Returns to refresh the display
        }
      }

      status = WiFi.status();
    } //End of while() loop

    if (counter>=MAX_CONNECTION_ATTEMPTS) { //Case if while() loop timeout.
      if (debugModeOn) {
        Serial.print("[setup - wifi] WiFi network ERROR: ");
        Serial.print("No connection to SSID ");Serial.println(wifiCred.wifiSSIDs[loopCounter].c_str());
        Serial.print("[setup - wifi] Number of connection attempts ");
        Serial.print(counter);Serial.print(">");Serial.println(MAX_CONNECTION_ATTEMPTS);
      }
      errorWifiConnection=true;
      errorsWiFiCnt++; //Something went wrong. Update error counter for stats
      *auxCounter=0;  //Reset the while() counter to be ready for the next index of the SSID array  
    }
    else {
      //End of while() due to successful SSID connection
      // (Button Pressed or Display Refresh force the function to return from the while() loop, 
      //  so this point is not reached in those cases)
      wifiCred.activeIndex=loopCounter;
      loopCounter=sizeof(wifiCred.wifiSSIDs); //loop end
    }
  }//end For() loop

  //This point is reached if either the while() loop timed out or successful SSID connection
  // (Button Pressed or Display Refresh force the function to return from the while() loop, 
  //  so this point is not reached in those cases)
  
  if (errorWifiConnection) {
    //Case for while loop timeout (no successfull SSID connection)
    if (auxLoopCounter!=nullptr) *auxLoopCounter=0;  //To avoid resuming connection the next loop interacion
    if (auxCounter!=nullptr) *auxCounter=0;          //To avoid resuming connection the next loop interacion
    return(ERROR_WIFI_SETUP); //not wifi connection
  }

  //Case for successfull SSID connection
  if (debugModeOn) {
    int16_t numberWiFiNetworks;
    Serial.println("[setup - wifi] Connected to the network. Waiting for getting WiFi info: "); printCurrentWiFi(true,&numberWiFiNetworks);
    Serial.print("[setup - wifi] Net info: \n");printNetData();
  }

  if (auxLoopCounter!=nullptr) *auxLoopCounter=0;  //To avoid resuming connection the next loop interacion
  if (auxCounter!=nullptr) *auxCounter=0;          //To avoid resuming connection the next loop interacion
  return(NO_ERROR); //WiFi Connection OK
}

uint32_t setupNTPConfig(boolean fromSetup=false,uint8_t* auxLoopCounter=nullptr,uint64_t* whileLoopTimeLeft=nullptr) {
  //If function called fron firstSetup(), then logs are displayed
  // and buttons are checked during NTP Sync handshake
  // Parameters:
  // - fromSetup: where the function was called from. Diferent prints out are done base on its value
  //      Value false: from main loop
  //      Value true:  from the firstSetup() function
  // - *auxLoopCounter is a pointer for the index of the NTP server array to check. It's sent from the main loop
  //      Value 0: The NTP sync will start from the very first NTP server - First call to the funcion
  //      Value other: The NTP sync will resume the sync with the same NTP server used in the previous interaction
  //          which was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  // - *whileLoopTimeLeft is the pointer for the time of while() loop to stop checking the index of
  //   the NTP server array. It's sent from the main loop
  //      Value NTP_CHECK_TIMEOUT: The NTP sync will start from the beginig - First call to the funcion
  //      Value other: The NTP sync will resume the sync at the same point where the previous interaction
  //          was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  // *auxLoopCounter and *whileLoopTimeLeft are global variables. They are modified in here. The calling function
  //   just send them to this function.

  if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - [setupNTPConfig] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", errorsNTPCnt="+String(errorsNTPCnt)+", auxLoopCounter="+String(*auxLoopCounter)+", whileLoopTimeLeft="+String(*whileLoopTimeLeft));}

  //to avoid pointer issues
  if (auxLoopCounter==nullptr || whileLoopTimeLeft==nullptr) {
    errorsNTPCnt++; //Something went wrong. Update error counter for stats
    return(ERROR_NTP_SERVER);
  }
  
  CloudClockStatus previousCloudClockCurrentStatus=CloudClockCurrentStatus;
  CloudClockCurrentStatus=CloudClockOffStatus;
  //User credentials definition
  /*ntpServers[0]="\0";ntpServers[1]="\0";ntpServers[2]="\0";ntpServers[3]="\0";
  #ifdef NTP_SERVER
    ntpServers[0]=NTP_SERVER;
  #endif
  #ifdef NTP_SERVER2
    ntpServers[1]=NTP_SERVER2;
  #endif
  #ifdef NTP_SERVER3
    ntpServers[2]=NTP_SERVER3;
  #endif
  #ifdef NTP_SERVER4
    ntpServers[3]=NTP_SERVER4;
  #endif
  */

  //boolean whileFlagOn=true;
  if (wifiCurrentStatus!=wifiOffStatus) {
    uint8_t auxForLoop;
    for (uint8_t loopCounter=*auxLoopCounter; loopCounter<(uint8_t)sizeof(ntpServers)/sizeof(String); loopCounter++) {
      if (ntpServers[loopCounter].charAt(0)=='\0') loopCounter++;
      else {
        if ((debugModeOn && fromSetup) || debugModeOn) {Serial.println("[setup - NTP] Connecting to NTP Server: "+ntpServers[loopCounter]);}
        if (!NTPResuming) { //Only calling configXTime() for new checks
          configTzTime(TZEnvVariable.c_str(), ntpServers[loopCounter].c_str());
        }

        *auxLoopCounter=loopCounter;
        uint64_t whileStartTime=loopStartTime+millis(),auxTime=whileStartTime;
        if (*whileLoopTimeLeft>=NTP_CHECK_TIMEOUT) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;

        if (debugModeOn) {Serial.println("    - whileStartTime="+String(whileStartTime)+", *whileLoopTimeLeft="+String(*whileLoopTimeLeft)+", *auxLoopCounter="+String(*auxLoopCounter)+", loopCounter="+String(loopCounter)+", waiting for NTPStatus=SNTP_SYNC_STATUS_COMPLETED");}
        while ( sntp_get_sync_status()!=SNTP_SYNC_STATUS_COMPLETED &&
               //*whileLoopTimeLeft<=NTP_CHECK_TIMEOUT && whileFlagOn) {
              *whileLoopTimeLeft<=NTP_CHECK_TIMEOUT) {
          *whileLoopTimeLeft=*whileLoopTimeLeft-(loopStartTime+millis()-auxTime);
          auxTime=loopStartTime+millis();
          delay(50);
          //Check if buttons are pressed during NTP sync handshake (if not in the first Setup)
          if (!fromSetup) {
            switch (checkButtonsActions(ntpcheck)) {
              case 1:
              case 2:
              case 3:
                //Button1 or Button2 pressed or released. NTP Sync process aborted if not Sync is completed
                if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - checkButtonsActions() returns 1, 2 or 3 - Returning with ERROR_ABORT_NTP_SETUP");}
                if (sntp_get_sync_status()!=SNTP_SYNC_STATUS_COMPLETED) { //Actions required as the process is aborted
                  forceNTPCheck=true; //Let's grant NTP check again in the next loop interaction
                  CloudClockCurrentStatus=previousCloudClockCurrentStatus; //Restore Cloud Clock status 
                  loopCounter=(uint8_t)sizeof(ntpServers)/sizeof(String); //Ends the for loop
                  //whileFlagOn=false; //Breaks the while loop and continue to the regular loop() flow
                  auxForLoop=loopCounter;
                  return(ERROR_ABORT_NTP_SETUP);
                }
              break;
              case 0:
              default:
                //Regular exit. Do nothing else
              break;
            }
            //Must the Display be refreshed? This check avoids the display gets blocked during NTP sync
            if (digitalRead(PIN_TFT_BACKLIGHT)!=LOW &&
                (
                  (((loopStartTime+millis()-lastTimeDisplayModeCheck) >= DISPLAY_MODE_REFRESH_PERIOD) && currentState==displayingSequential)
                  ||
                  (((loopStartTime+millis()-lastTimeDisplayCheck) >= DISPLAY_REFRESH_PERIOD) && 
                    (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || 
                    currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) )
                )
              ) {
              return (ERROR_BREAK_NTP_SETUP); //Returns to refresh the display
            }
          }
        } //end while() loop

        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"    - End of wait - Elapsed Time: "+String(auxTime-whileStartTime));}
        
        if (*whileLoopTimeLeft>NTP_CHECK_TIMEOUT) { //Case if while() loop timeout.
          if ((debugModeOn && fromSetup) || debugModeOn) {
            Serial.println("  Time: Failed to get time");
            Serial.print("[setup] - NTP: ");Serial.println("KO");
          }
        }
        else { 
          //End of while() due to successful NTP sync
          // (Button Pressed or Display Refresh force the function to return from the while() loop, 
          //  so this point is not reached in those cases)
          if ((debugModeOn && fromSetup) || debugModeOn) {
            Serial.print("  Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo,"%d/%m/%Y - %H:%M:%S");
            Serial.print("[setup] - NTP: ");Serial.println("OK");
          }
          CloudClockCurrentStatus=CloudClockOnStatus;
          ntpServerIndex=loopCounter;
          loopCounter=(uint8_t)sizeof(ntpServers)/sizeof(String);
          memset(TZEnvVar,'\0',sizeof(TZEnvVar)); //Fill null character first to avoid overflow
          int tzLength=String(getenv("TZ")).length();
          if (tzLength>sizeof(TZEnvVar)) tzLength=sizeof(TZEnvVar); //Make sure not to overflow TZEnvVar
          memcpy(TZEnvVar,getenv("TZ"),tzLength); //Back Time Zone up to restore it after wakeup
        }
      }
    }//end For() loop
  }

  //This point is reached if either the while() loop timed out or successful NTP sync
  // (Button Pressed or Display Refresh force the function to return from the while() loop, 
  //  so this point is not reached in those cases)
  
  //if (CloudClockCurrentStatus==CloudClockOffStatus && whileFlagOn) {
  if (CloudClockCurrentStatus==CloudClockOffStatus) {
    //Case for while loop timeout (no successfull NTP sync)
    errorsNTPCnt++; //Something went wrong. Update error counter for stats   
    if (auxLoopCounter!=nullptr) *auxLoopCounter=0;                 //To avoid resuming connection the next loop interacion
    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    return(ERROR_NTP_SERVER); //not NTP server connection
  }

  //Case for successfull NTP sync
  if (debugModeOn) {
      Serial.println("    - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck)+", errorsNTPCnt="+String(errorsNTPCnt));
      Serial.print(String(loopStartTime+millis())+"  - [setupNTPConfig] - Exit - Time:");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo,"%d/%m/%Y - %H:%M:%S");
  }

  if (auxLoopCounter!=nullptr) *auxLoopCounter=0;                 //To avoid resuming connection the next loop interacion
  if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;  //To avoid resuming connection the next loop interacion       
  return(NO_ERROR);
}

bool runAPMode() {
  bool activeOK=true,auxWileLoop=true;
  if (debugModeOn) Serial.println("  [runAPMode] - WiFi Enabled but no WiFi SSID is setup. Asking if run AP.");
  tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  tft.drawString("WIFI enabled but no SSID is setup",5,4,TEXT_FONT_BOOT_SCREEN);
  tft.drawString("To setup device WiFi, connect to:",5,28,TEXT_FONT_BOOT_SCREEN);
  tft.drawString("    SSID: "+String(APMODE_SSID),5,45,TEXT_FONT_BOOT_SCREEN);
  tft.drawString("    PSSW: "+String(APMODE_PSSW),5,62,TEXT_FONT_BOOT_SCREEN);
  tft.drawString("    http://"+String(APMODE_LOCAL_IP_ADDRESS),5,79,TEXT_FONT_BOOT_SCREEN);
  //While loop, waiting for pushing any button
  while (auxWileLoop) {
    switch (checkButtonsActions(askAPloop)) {
      case 4: //Value return from checkButtonsActions()
        activeOK=!activeOK;
      break;
      case 5: //Value return from checkButtonsActions()
        auxWileLoop=false;
      break;
      default:  //Value return from checkButtonsActions()
      break;
    }
    if (activeOK) {
      tft.setTextColor(TFT_YELLOW,TFT_BLACK);tft.drawString(" CANCEL ",9,108,TEXT_FONT_BOOT_SCREEN);
      tft.setTextColor(TFT_BLACK,TFT_YELLOW);tft.drawString(" OK ",70,108,TEXT_FONT_BOOT_SCREEN);
      tft.setTextColor(TFT_YELLOW,TFT_BLACK);tft.drawString("  (Buttons to select)",100,108,TEXT_FONT_BOOT_SCREEN);
    }
    else {
      tft.setTextColor(TFT_BLACK,TFT_YELLOW);tft.drawString(" CANCEL ",9,108,TEXT_FONT_BOOT_SCREEN);
      tft.setTextColor(TFT_YELLOW,TFT_BLACK);tft.drawString(" OK ",70,108,TEXT_FONT_BOOT_SCREEN);
      tft.drawString("  (Buttons to select)",100,108,TEXT_FONT_BOOT_SCREEN);
    }
  }
  
  if (activeOK) {
    //Start AP Mode
    IPAddress APLocalIP=stringToIPAddress(String(APMODE_LOCAL_IP_ADDRESS));
    IPAddress APLocalGW=stringToIPAddress(String(APMODE_GATEWAY_ADDRESS));
    IPAddress APLocalSubnetMask=stringToIPAddress(String(APMODE_SUBNETWORK_MASK));
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(APLocalIP, APLocalGW, APLocalSubnetMask);
    WiFi.softAP(APMODE_SSID, APMODE_PSSW);
    if (debugModeOn) Serial.println("  [runAPMode] - AP IP address: ="+WiFi.softAPIP().toString());

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW,TFT_BLACK);
    tft.drawString("AP and Web Server already ENABLED.",5,4,TEXT_FONT_BOOT_SCREEN);
    tft.drawString("Continue from the browser.",5,21,TEXT_FONT_BOOT_SCREEN);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);
    tft.drawString("To setup device WiFi, connect to:",5,45,TEXT_FONT_BOOT_SCREEN);
    tft.drawString("    SSID: "+String(APMODE_SSID),5,62,TEXT_FONT_BOOT_SCREEN);
    tft.drawString("    PSSW: "+String(APMODE_PSSW),5,79,TEXT_FONT_BOOT_SCREEN);
    tft.drawString("    http://"+String(APMODE_LOCAL_IP_ADDRESS),5,96,TEXT_FONT_BOOT_SCREEN);
  }

  return(activeOK);
}