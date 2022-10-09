/* Dealing with WiFi stuff

*/

#include "wifiConnection.h"

#ifdef __TFT_DISPLAY_PRESENT__
  #include <TFT_eSPI.h>
  extern TFT_eSPI tft;
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

  uint8_t wifiNetworkNode=255;
  for (uint8_t i=0; i<countWiFiNetworks; i++) {
    String mySSID=String(WIFI_SSID_CREDENTIALS);
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
uint8_t wifiConnect() {
  int counter=0;
  // attempt to connect to Wifi network:
  WiFi.begin(WIFI_SSID_CREDENTIALS, WIFI_PW_CREDENTIALS);
  while (status != WL_CONNECTED && counter < MAX_CONNECTION_ATTEMPTS) {
    if (logsOn) {Serial.print("[setup - wifi] Attempting to connect to WPA SSID: ");Serial.println(WIFI_SSID_CREDENTIALS);}
    #ifdef __TFT_DISPLAY_PRESENT__
      tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print(".");
    #endif
    // Connect to WPA/WPA2 network:
    status = WiFi.status();
    delay(500);
    counter++;
  }

  if (counter>=MAX_CONNECTION_ATTEMPTS) {
    if (logsOn) {
      Serial.print("[setup - wifi] WiFi network ERROR: ");
      Serial.print("No connection to SSID ");Serial.println(WIFI_SSID_CREDENTIALS);
      Serial.print("[setup - wifi] Number of connection attempts ");
      Serial.print(counter);Serial.print(">");Serial.println(MAX_CONNECTION_ATTEMPTS);
    }

    return(ERROR_WIFI_SETUP); //not wifi connection
  }

  // you're connected now, so print out the data:
  if (logsOn) {
    int16_t numberWiFiNetworks;
    Serial.println("[setup - wifi] Connected to the network. Waiting for getting WiFi info: "); printCurrentWiFi(true,&numberWiFiNetworks);
    Serial.print("[setup - wifi] Net info: \n");printNetData();
  }

  return(NO_ERROR); //WiFi Connection OK
}