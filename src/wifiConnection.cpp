/* Dealing with WiFi stuff

*/

#include "wifiConnection.h"

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
wifiNetworkInfo * printCurrentWiFi(boolean logsOn=true, int16_t *numberWiFiNetworks=nullptr) {
  // print current network parameters
  WiFiScanClass wifiScan = WiFiScanClass();
  int16_t countWiFiNetworks=wifiScan.scanNetworks();
  int16_t scanReturn=-1;
  while (scanReturn<0){
    scanReturn=wifiScan.scanComplete();
    switch (scanReturn) {
      case WIFI_SCAN_FAILED:
        if (logsOn) Serial.println("  [error] - Failed to scan WiFi networks");
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
      //if (logsOn) {Serial.print(mySSID);Serial.print(" is equal than "); Serial.println(wifiNet.ssid);
      //             Serial.print("networkItem=");Serial.println(i);}
      wifiNetworkNode=i;
      break;
    }
    else {
      //if (logsOn) {Serial.print(mySSID);Serial.print(" is not equal than "); Serial.println(wifiNet.ssid);}
    }
  }

  if (-1==wifiNetworkNode) {
    //WiFi SSID was lost
    if (logsOn) {
      Serial.print("  SSID: ");Serial.print(wifiCred.wifiSSIDs[wifiCred.activeIndex]);Serial.println(" lost");
    }
    return nullptr;
  }

  // print current network parameters
  if (logsOn) {
    Serial.print("  Wifi Network Node: ");Serial.print(wifiNetworkNode);
    Serial.print(", "); Serial.print(scanReturn); Serial.println(" detected");
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
uint8_t wifiConnect(boolean logsOn=true, boolean msgTFT=true, uint8_t* auxLoopCounter=nullptr, uint8_t* auxCounter=nullptr) {
  // Parameters:
  // - logsOn: Print or not log messages in Serial line. Diferent prints out are done base on its value
  //      Value false: no log messages are printed out
  //      Value true:  log messages are printed out
  // - msgTFT: Print or not log messages in the Display during the boot time. Diferent prints out are done base on its value
  //      Value false: no log messages are displayed out
  //      Value true:  log messages are displayed out
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
        if (logsOn) {Serial.print("[setup - wifi] Attempting to connect to WPA SSID: ");Serial.println(wifiCred.wifiSSIDs[loopCounter].c_str());}
        if (msgTFT) {
          #ifdef __TFT_DISPLAY_PRESENT__
            stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.print(".");
            stext1.pushSprite(0,(LINES_PER_TEXT_SCROLL-LINES_PER_TEXT_SPRITE)/2*tft.fontHeight(TEXT_FONT_BOOT_SCREEN));
          #endif
        }
      }
      //Check if buttons are pressed during SSID connection (if not in the first Setup --!msgTFT--)
      if (!msgTFT) {
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
      if (logsOn) {
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
  if (logsOn) {
    int16_t numberWiFiNetworks;
    Serial.println("[setup - wifi] Connected to the network. Waiting for getting WiFi info: "); printCurrentWiFi(true,&numberWiFiNetworks);
    Serial.print("[setup - wifi] Net info: \n");printNetData();
  }

  if (auxLoopCounter!=nullptr) *auxLoopCounter=0;  //To avoid resuming connection the next loop interacion
  if (auxCounter!=nullptr) *auxCounter=0;          //To avoid resuming connection the next loop interacion
  return(NO_ERROR); //WiFi Connection OK
}