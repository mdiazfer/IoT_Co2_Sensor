/* Dealing with Web Server stuff
   version: 0.9.3
    - 2 error variables: errorOnActiveCookie and errorOnWrongCookie
    - in upload, the right error variable is set
    - no error responses are sent during chunck process
    - only final POST sends response. error depends on the cookie (active or wrong one)
*/

#include "webServer.h"

String processor(const String& var){
  if(var == "CO2") {
    return roundFloattoString(valueCO2,0)+" ppm";
  } else if (var == "TEMPERATURE") {
    return roundFloattoString(valueT,1)+" ºC";
  } else if (var == "HUMIDITY") {
    return roundFloattoString(valueHum,1)+" %";
  } else if (var == "DEVICENAME") {
    return device;
  } else if (var == "FIRMWAREVERSION") {
    return String(VERSION);
  } else if (var == "USB") {
    switch (powerState) {
      case(off):
        return String();
      break;
      case(chargingUSB):
        return String ("USB charging, ")+String (batADCVolt/1000)+String(" v, ")+String(batCharge)+" %";
      break;
      case(onlyBattery):
        return String ("No USB. Battery, ")+String (batADCVolt/1000)+String(" v, ")+String(batCharge)+" %";
      break;
      case(noChargingUSB):
        return String ("USB no charging, ")+String (batADCVolt/1000)+String(" v, ")+String(batCharge)+" %";
      break;
    }
    return String();
  } else if (var == "DATE") {
    struct tm timeinfo;

    if (getLocalTime(&timeinfo)) {
      char s[100];
      strftime(s,sizeof(s),"%d/%m/%Y - %H:%M:%S",&timeinfo);
      return String(s);
    }
    else return String("NTP server down");
  } else if (var == "UPTIMEDATE") {
    char s[100];
    strftime(s,sizeof(s),"%d/%m/%Y - %H:%M:%S",&startTimeInfo);
    return String(s);
  } else if (var == "TIMEZONE") {
    return TZEnvVariable;
    //return String(TZEnvVar);
  } else if (var == "TIMEZONENAME") {
    return TZName;
  } else if (var == "SENSORTEMPHUM") {
    return tempHumSensorType;
  } else if (var == "SENSORCO2") {
    return co2SensorType;
  } else if (var == "SENSORCO2VERSION") {
    return String(co2SensorVersion);
  } else if (var == "SENSORCO2TEMP") {
    return roundFloattoString(co2Sensor.getTemperature(true,true),1)+" ºC";
  } else if (var == "SSID") {
    if (wifiEnabled) return WiFi.SSID(); else return String("WiFi Disabled");
  } else if (var == "BSSID") {
    if (wifiEnabled) return WiFi.BSSIDstr(); else return String("WiFi Disabled");
  } else if (var == "RSSI") {
    if (wifiEnabled) return String(WiFi.RSSI()); else return String("WiFi Disabled");
  } else if (var == "WIFICHANNEL") {
    if (wifiEnabled) return String(WiFi.channel()); else return String("WiFi Disabled");
  } else if (var == "MACADDRESS") {
    if (wifiEnabled) return String(WiFi.macAddress()); else return String("WiFi Disabled");
  } else if (var == "IPADDRESS") {
    if (wifiEnabled) return String(WiFi.localIP().toString()); else return String("WiFi Disabled");
  } else if (var == "MASK") {
    if (wifiEnabled) return String(WiFi.subnetMask().toString()); else return String("WiFi Disabled");
  } else if (var == "DEFAULTGW") {
    if (wifiEnabled) return String(WiFi.gatewayIP().toString()); else return String("WiFi Disabled");
  } else if (var == "NTPSERVER") {
    if (wifiEnabled && CloudClockCurrentStatus==CloudClockOnStatus) return ntpServers[ntpServerIndex]; else return String("Not Available");
  } else if (var == "NTPSTATUS") {
    if (wifiEnabled && CloudClockCurrentStatus==CloudClockOnStatus) return String("Synced"); else return String("Not Available");
  } else if (var == "WIFISTATUS") {
    if (wifiEnabled) return String("Enabled"); else return String("Disabled");
  } else if (var == "BLUETOOTHSTATUS") {
    if (bluetoothEnabled) return String("Enabled"); else return String("Disabled");
  } else if (var == "CLOUDSERVICESSTATUS") {
    if (uploadSamplesEnabled) return String("Enabled"); else return String("Disabled");
  } else if (var == "MQTTSERVICESSTATUS") {
    if (mqttServerEnabled) return String("Enabled"); else return String("Disabled");
  } else if (var == "Secure_MQTTSERVICESSTATUS") {
    if (secureMqttEnabled) return String("Enabled"); else return String("Disabled");
  } else if (var == "MQTTTOPICNAME") {
    return mqttTopicName;
  } else if (var == "CLOUDSERVICESURL") {
    return String("http://"+serverToUploadSamplesIPAddress.toString())+String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1);
  } else if (var == "CURRENTENERGYMODE") {
    switch (energyCurrentMode)
    {
      case (fullEnergy): return String("Full Energy"); break;
      case(reducedEnergy): return String("Reduced Energy"); break;
      case(lowestEnergy): return String("Lowest Energy"); break;
    }
    return String();
  } else if (var == "BATTERYMODE") {
    switch (configSavingEnergyMode)
    {
      case (fullEnergy): return String("Full Energy"); break;
      case(reducedEnergy): return String("Reduced Energy"); break;
      case(lowestEnergy): return String("Lowest Energy"); break;
    }
    return String();
  } else if (var == "DISPLAYSTATUS") {
    if (digitalRead(PIN_TFT_BACKLIGHT)) return String("On");
    else return String("Off");
  } else if (var == "DISPLAYMODE") {
    switch (currentState)
    {
      case(bootupScreen): return String("bootupScreen"); break;
      case(mainMenu): return String("mainMenu"); break;
      case(showOptMenu): return String("showOptMenu"); break;
      case(infoMenu): return String("infoMenu"); break;
      case(infoMenu1): return String("infoMenu1"); break;
      case(infoMenu2): return String("infoMenu2"); break;
      case(infoMenu3): return String("infoMenu3"); break;
      case(infoMenu4): return String("infoMenu4"); break;
      case(displayingSampleFixed): return String("displayingSampleFixed"); break;
      case(displayingCo2LastHourGraphFixed): return String("displayingCo2LastHourGraphFixed"); break;
      case(displayingCo2LastDayGraphFixed): return String("displayingCo2LastDayGraphFixed"); break;
      case(displayingSequential): return String("displayingSequential"); break;
      case(configMenu): return String("configMenu"); break;
      case(confMenuWifi): return String("confMenuWifi"); break;
      case(confMenuBLE): return String("confMenuBLE"); break;
      case(confMenuUpMeas): return String("confMenuUpMeas"); break;
      case(confMenuSavBatMode): return String("confMenuSavBatMode"); break;
    }
    return String();
  } else if (var == "AUTOOFF") {
    if (autoBackLightOff) return String("On"); else return String("Off");
  } else if (var == "WIFICHECKED_ON") {
    if (wifiEnabled) return String ("checked");
    else return String();
  } else if (var == "WIFICHECKED_OFF") {
    if (!wifiEnabled) return String ("checked");
    else return String();
  } else if (var == "UserName") {
    return userName;
  } else if (var == "UserName_VALUE") {
    return userName;  
  } else if (var == "UserPssw") {
    return "User password";
  } else if (var == "UserPssw_VALUE") {
    return "**********";  
  } else if (var == "MQTTUserName") {
    return mqttUserName;
  } else if (var == "MQTTUserName_VALUE") {
    return mqttUserName;  
  } else if (var == "MQTTUserPssw") {
    return "User password";
  } else if (var == "MQTTUserPssw_VALUE") {
    return "**********";  
  } else if (var == "SSID") {
    if (wifiCred.wifiSSIDs[0].compareTo(String(""))==0) {if (wifiEnabled) return "Mandatory if WiFi enabled"; else return String();}
    else return wifiCred.wifiSSIDs[0];
  } else if (var == "SSID_VALUE") {
    if (wifiCred.wifiSSIDs[0].compareTo(String(""))==0) return String();
    else return wifiCred.wifiSSIDs[0];
  } else if (var == "SSID_REQUIRED") {
    if (wifiCred.wifiSSIDs[0].compareTo(String(""))==0) {if (wifiEnabled) return "required"; else return String();}
    else return String();
  } else if (var == "PSSW") {
    if (wifiCred.wifiPSSWs[0].compareTo(String(""))==0) return "Mandatory PSSW";
    else return "**********";
  } else if (var == "PSSW_VALUE") {
    return wifiCred.wifiPSSWs[0];
  } else if (var == "SITE") {
    return wifiCred.wifiSITEs[0];
  } else if (var == "SITE_VALUE") {
    return wifiCred.wifiSITEs[0];
  } else if (var == "SSID_BK1") {
    return wifiCred.wifiSSIDs[1]; 
  } else if (var == "SSID_BK1_VALUE") {
    return wifiCred.wifiSSIDs[1];
  } else if (var == "PSSW_BK1") {
    if (wifiCred.wifiPSSWs[1].compareTo(String(""))==0) return wifiCred.wifiPSSWs[1];
    else return "**********";
  } else if (var == "PSSW_BK1_VALUE") {
    return wifiCred.wifiPSSWs[1];
  } else if (var == "SITE_BK1") {
    return wifiCred.wifiSITEs[1];
  } else if (var == "SITE_BK1_VALUE") {
    return wifiCred.wifiSITEs[1];
  } else if (var == "SSID_BK2") {
    return wifiCred.wifiSSIDs[2];
  } else if (var == "SSID_BK2_VALUE") {
    return wifiCred.wifiSSIDs[2];
  } else if (var == "PSSW_BK2") {
    if (wifiCred.wifiPSSWs[2].compareTo(String(""))==0) return wifiCred.wifiPSSWs[2];
    else return "**********";
  } else if (var == "PSSW_BK2_VALUE") {
    return wifiCred.wifiPSSWs[2];
  } else if (var == "SITE_BK2") {
    return wifiCred.wifiSITEs[2];
  } else if (var == "SITE_BK2_VALUE") {
    return wifiCred.wifiSITEs[2];
  } else if (var == "NTP1") {
    if (ntpServers[0].compareTo(String(""))==0) {if (wifiEnabled) return "Mandatory if WiFi enabled"; else return String();}
    else return ntpServers[0];
  } else if (var == "NTP1_VALUE") {
    if (ntpServers[0].compareTo(String(""))==0) return String();
    else return ntpServers[0];
  } else if (var == "NTP1_REQUIRED") {
    if (ntpServers[0].compareTo(String(""))==0) {if (wifiEnabled) return "required"; else return String();}
    else return String();
  } else if (var == "NTP2") {
    return ntpServers[1]; 
  } else if (var == "NTP2_VALUE") {
    return ntpServers[1];
  } else if (var == "NTP3") {
    return ntpServers[2]; 
  } else if (var == "NTP3_VALUE") {
    return ntpServers[2];
  } else if (var == "NTP4") {
    return ntpServers[3]; 
  } else if (var == "NTP4_VALUE") {
    return ntpServers[3];
  } else if (var == "DISPLAYONCHECKED") {
    if (digitalRead(PIN_TFT_BACKLIGHT)) return String("checked");
    else return String("");
  } else if (var == "DISPLAYOFFCHECKED") {
    if (!digitalRead(PIN_TFT_BACKLIGHT)) return String("checked");
    else return String("");
  } else if (var == "DISPLAYSWONCHECKED") {
    if (autoBackLightOff) return String("checked");
    else return String("");
  } else if (var == "DISPLAYSWOFFCHECKED") {
      if (!autoBackLightOff) return String("checked");
      else return String("");
  } else if (var == "BATTERYMODEREDUCEDSELECTED") {
      if (energyCurrentMode == reducedEnergy) return String("checked");
      else return String("");
  } else if (var == "BATTERYMODELOWESTSELECTED") {
      if (energyCurrentMode == lowestEnergy) return String("checked");
      else return String("");
  } else if (var == "DISPLAYMODESAMPLESELECTED") {
      if (currentState == displayingSampleFixed) return String("selected");
      else return String("");
  } else if (var == "DISPLAYMODELASTHOURSELECTED") {
      if (currentState == displayingCo2LastHourGraphFixed) return String("selected");
      else return String("");
  } else if (var == "DISPLAYMODELASTDAYSELECTED") {
      if (currentState == displayingCo2LastDayGraphFixed) return String("selected");
      else return String("");
  } else if (var == "DISPLAYMODESEQUENTIALSELECTED") {
    if (currentState == displayingSequential) return String("selected");
    else return String("");
  } else if (var == "CLOUD_ON_CHECKED") {
    if (uploadSamplesEnabled) return String("checked");
    else return String();
  } else if (var == "CLOUD_OFF_CHECKED") {
    if (!uploadSamplesEnabled) return String("checked");
    else return String();
  } else if (var == "SITE_ALLOW_NAME") {
    if (wifiCred.wifiSITEs[0].compareTo(String(""))==0) return String("NO SITE DEFINED");
    else return wifiCred.wifiSITEs[0];
  } else if (var == "SITE_BK1_ALLOW_NAME") {
    if (wifiCred.wifiSITEs[1].compareTo(String(""))==0) return String("NO SITE DEFINED");
    else return wifiCred.wifiSITEs[1];
  } else if (var == "SITE_BK2_ALLOW_NAME") {
    if (wifiCred.wifiSITEs[2].compareTo(String(""))==0) return String("NO SITE DEFINED");
    else return wifiCred.wifiSITEs[2];
  } else if (var == "SITE_ALLOW_CHECKED") {
    if (wifiCred.wifiSITEs[0].compareTo(String(""))==0) return String(" disabled ");
    else if (wifiCred.SiteAllow[0]) return String("checked");
    else return String();
  } else if (var == "SITE_BK1_ALLOW_CHECKED") {
    if (wifiCred.wifiSITEs[1].compareTo(String(""))==0) return String(" disabled ");
    else if (wifiCred.SiteAllow[1]) return String("checked");
    else return String();
  } else if (var == "SITE_BK2_ALLOW_CHECKED") {
    if (wifiCred.wifiSITEs[2].compareTo(String(""))==0) return String(" disabled ");
    else if (wifiCred.SiteAllow[2]) return String("checked");
    else return String();
  } else if (var == "MQTTSERVER") {
    return mqttServer;
  } else if (var == "MQTTTOPIC") {
    return mqttTopicPrefix;
  } else if (var == "MQTT_ON_CHECKED") {
    if (mqttServerEnabled) return String("checked");
    else return String();
  } else if (var == "MQTT_OFF_CHECKED") {
    if (!mqttServerEnabled) return String("checked");
    else return String();
  } else if (var == "MQTTREQUIRED") {
    if (mqttServerEnabled) return String("required");
    else return String();
  } else if (var == "Secure_MQTT_ON_CHECKED") {
    if (secureMqttEnabled) return String("checked");
    else return String();
  } else if (var == "Secure_MQTT_OFF_CHECKED") {
    if (!secureMqttEnabled) return String("checked");
    else return String();
  } else if (var == "BLUETOOTH_ON_CHECKED") {
    if (bluetoothEnabled) return String("checked");
    else return String();
  } else if (var == "BLUETOOTH_OFF_CHECKED") {
    if (!bluetoothEnabled) return String("checked");
    else return String();
  } else if (var == "CONTAINER_HEAD") {
    if (deviceReset) return String("Device Reset");
    else if (factoryReset) return String("Factory Reset");
    else if (fileUpdateError) return String("File Upload");
    else return String("Other");
  } else if (var == "CONTAINER_TEXT") {
    if (deviceReset && updateCommand!=U_FLASH && updateCommand!=U_SPIFFS) {
      if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) {
        deviceReset=false;
        return String("Authentication is required to reboot the device (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://)"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>");
      }
      else return String("Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=80;var timer = maxTime;var auxTimeout=false;var interval = setInterval(function() {if (timer > 0) document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';if (timer === maxTime) {var request = new XMLHttpRequest();request.open('GET', 'http://"+WiFi.localIP().toString()+"', true);request.ontimeout = function(){auxTimeout=true;};request.onloadend = function(){if (auxTimeout === true) document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';else {document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');},1500)};timer=0;};request.onloadstart = function(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};request.send();};if (timer === 0) clearInterval(interval);else timer--;},1000);</script>");
    }
    else if (deviceReset && (updateCommand==U_FLASH || updateCommand==U_SPIFFS)) {
      String auxString=(updateCommand==U_FLASH) ? String("Binary file has been successfully written in FLASH. Size="):String("System file has been successfully written in SPIFFS. Size=");
      return String(auxString+fileUpdateSize+" B.<p></p>Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=80;var timer = maxTime;var auxTimeout=false;var interval = setInterval(function() {if (timer > 0) document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';if (timer === maxTime) {var request = new XMLHttpRequest();request.open('GET', 'http://"+WiFi.localIP().toString()+"', true);request.ontimeout = function(){auxTimeout=true;};request.onloadend = function(){if (auxTimeout === true) document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';else {document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');},1500)};timer=0;};request.onloadstart = function(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};request.send();};if (timer === 0) clearInterval(interval);else timer--;},1000);</script>");
    }
    else if (factoryReset) {
      if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) {
        factoryReset=false;
        return String("Authentication is required to factory reset the device (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\"\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>");
      }
      else return String("The configuration of the device has been successfully reset to factory values.<p></p>The WiFi SSIDs need to be setup again as the "+WiFi.SSID()+" SSID has been deleted.<br><br>Web access to the device is not available till the WiFi is setup again.");
    }
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) return (String("Authentication is required to upload the file (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOBIN_FILE) return (String("File is not a valid Binary file (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOSPIFFS_FILE) return (String("File is not a valid SPIFFS partition (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE) return (String("File is not a valid SPIFFS partition (0x")+String(fileUpdateError,HEX)+String(").<p></p>Partion in Flash might be corrupted so the built-in web server might not work....<p></p>Reboot and try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError!=ERROR_UPLOAD_FILE_NOERROR) return (String("Something went wrong while uploading the file (0x")+String(fileUpdateError,HEX)+String(").<p></p>You may need to reboot the device before trying again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\"\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    //else if (fileUpdateError!=0) return String("Something went wrong in the file upload process.<p></p>Try again.");
    else return String("Other");
  } else if (var == "OTADISABLED") { //v1.2.0 - Not allow OTA upgrade if not two OTA partitions
    if (OTAUpgradeBinAllowed) return String();
    else return String("disabled");
  } else if (var == "SPIFFSDISABLED") { //v1.2.0 - Not allow SPIFFS upgrade it's wrong
    if (SPIFFSUpgradeBinAllowed) return String();
    else return String("disabled");
  } else if (var == "SPIFFSCHECKED") { //v1.2.0 - Not allow SPIFFS upgrade it's wrong
    if (!SPIFFSUpgradeBinAllowed) return String();
    else return String("checked");
  } else if (var == "SELECTFILEBUTTONDISABLED") { //v1.2.0 - Not allow upgrade if OTA or SPIFFS are wrong
    if (OTAUpgradeBinAllowed || SPIFFSUpgradeBinAllowed) return String();
    else return String("disabled"); 
  } else if (var == "SETUPERRORS") {
    return String("0x")+String(error_setup,HEX);
  } else if (var == "WIFIERRORS") {
    return String(errorsWiFiCnt);
  } else if (var == "SAMPLEUPDTERRORS") {
    return String(errorsSampleUpts);
  } else if (var == "NTPERRORS") {
    return String(errorsNTPCnt);
  } else if (var == "SPIFFSERRORS") {
    return String(SPIFFSErrors);
  } else if (var == "BOOTCOUNT") {
    return String(bootCount);
  } else if (var == "RESETCOUNT") {
    return String(resetCount);
  } else if (var == "MINHEAPSIZE") {
    return String(minHeapSeen);
  } else if (var == "CURRENTHEAPSIZE") {
    return String(esp_get_free_heap_size());
  } else if (var == "MAINTENLEFTMEM") {
    return String(OTAAvailableSize)+String(",")+String(SPIFFSAvailableSize);
  } else if (var == "MAINTENLEFTBINMEM") {
    return String(OTAAvailableSize);
  } else if (var == "MAINTENLEFTSPIFFSMEM") {  
    return String(SPIFFSAvailableSize);
  } else {
    return String();
  }
}

uint32_t initWebServer() {
  
  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for root index.html web page
  webServer.on(WEBSERVER_INDEX_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route to load style.css file
  webServer.on(WEBSERVER_CSSSTYLES_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CSSSTYLES_PAGE, "text/css");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to load tswnavbar.css file
  webServer.on(WEBSERVER_CSSNAVBAR_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CSSNAVBAR_PAGE, "text/css");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to load maintenance_upload_firmware/identity file
  webServer.on("/maintenance_upload_firmware/identity", HTTP_GET, [](AsyncWebServerRequest *request) {
    String id = String((uint32_t)ESP.getEfuseMac(), HEX);
    id.toUpperCase();
    
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(200, "application/json", "{\"id\": \""+id+"\", \"hardware\": \"ESP32\"}");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route to load The_IoT_Factory.png file
  webServer.on(WEBSERVER_LOGO_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_LOGO_ICON, "image/png");

    //Wait to disconnect the connection
    request->onDisconnect([]{
      //Excute wen the request->send(SPIFFS, WEBSERVER_LOGO_ICON, "image/png"); is finished
      
      //SSID change - Reconnect WiFi
      if (wifiEnabled && reconnectWifiAndRestartWebServer) {
        wifiCurrentStatus=wifiOffStatus;
        forceWifiReconnect=true; //Don't wait next WIFI_RECONNECT_PERIOD interaction. Reconnect in this loop() interaction
        WiFi.disconnect(true);
        delay(1000);
        WiFi.disconnect(false);
        reconnectWifiAndRestartWebServer=false;
      }

      //NTP Server - Resync NTP Server
      if(wifiEnabled && resyncNTPServer) {
        CloudClockCurrentStatus=CloudClockOffStatus;
        forceNTPCheck=true;
        resyncNTPServer=false;
      }

      //Device reset
      if (deviceReset) ESP.restart();
    });

    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  //webServer.serveStatic(WEBSERVER_LOGO_ICON,SPIFFS,WEBSERVER_LOGO_ICON);

  webServer.on(WEBSERVER_INFO_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INFO_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route for  basic.html web page
  webServer.on(WEBSERVER_BASICCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_BASICCONFIG_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for cloud.html web page
  webServer.on(WEBSERVER_CLOUDCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CLOUDCONFIG_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for bluetooth.html web page
  webServer.on(WEBSERVER_BLUETOOTHCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_BLUETOOTHCONFIG_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for maintenance.html web page
  webServer.on(WEBSERVER_MAINTENANCE_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    fileUpdateError=0; //To check POST parameters & File Upload process
    //request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false, processor);

    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow

    //Setup a new response to send Set Cookie headers in the maintenance.html answer
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false, processor);
    auxResp->setCode(200);

    //If comes with a valid cookie and there is an ongoing upload, return the same cookie
    //This way, a broser-based abort action can be detected
    if (Update.progress()!=0 && request->hasHeader("Cookie") &&
        (request->getHeader("Cookie")->value().length() <= COOKIE_SIZE-1) &&
        memcmp(device.c_str(),request->getHeader("Cookie")->value().c_str()+3,device.length())==0 ) {

          memcpy(currentSetCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length()); //End null not included
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance GET] - Valid cookie deteced="+String(currentSetCookie)+" kept");
          auxResp->addHeader("Set-Cookie", String(currentSetCookie));
    } 
    else {
      //Create a new cookie if the condition is not met
      long auxRandom1=random(0,2147483647);long auxRandom2=random(0,2147483647);long auxRandom3=random(0,2147483647);long auxRandom4=random(0,2147483647);
      String setCookie=String("id-"+device+"="+String(auxRandom1,HEX)+String(auxRandom2,HEX)+String(auxRandom3,HEX)+String(auxRandom4,HEX));      
      memcpy(currentSetCookie,setCookie.c_str(),setCookie.length()); //End null not included
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance GET] - New cookie="+String(currentSetCookie)+" created");
      auxResp->addHeader("Set-Cookie", setCookie);
    }
    request->send(auxResp);

    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for container.html web page
  webServer.on(WEBSERVER_CONTAINER_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    updateCommand=-1;
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for /maintenance_default_values web page
  webServer.on(WEBSERVER_DEFAULTCONF_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR; //To check POST parameters & File Upload process
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for /maintenance_device_reset web page
  webServer.on(WEBSERVER_DEVICERESET_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR; //To check POST parameters & File Upload process
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route for updatefile.html web page
  webServer.on(WEBSERVER_UPLOADFILE_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    fileUpdateError=0; //To check POST parameters & File Upload process
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to load favicon.ico file
  webServer.on(WEBSERVER_FAVICON_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_FAVICON_ICON, "image/x-icon");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route to jquery.min.js file
  webServer.on(WEBSERVER_JQUERY_JS, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_JQUERY_JS, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to WEBSERVER_SAMPLES_PAGE file
  webServer.on(WEBSERVER_SAMPLES_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(200, "application/json", JSON.stringify(samples));
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to gauge.min.js file
  webServer.on(WEBSERVER_GAUGESCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_GAUGESCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route to resutl_script.js file
  webServer.on(WEBSERVER_RESULTSCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_RESULTSCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on("/basic1", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST WiFi_enabled value
        if (String(p->name()).compareTo("WiFi_enabled")==0) {
          if ((String(p->value().c_str()).compareTo("on")==0) && !wifiEnabled) {
            //Never enter here as wifi is disabled. Anyway the code should work
            wifiEnabled=true;
            if(WiFi.status()!=WL_CONNECTED) {
              forceWifiReconnect=true; //Next loop interaction the WiFi connection is done
              forceNTPCheck=true; //Let's force NTP sync
            }
            uint8_t configVariables=EEPROM.read(0x08);
            configVariables|=0x10; //Set wifiEnabled bit to true (enabled)
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
          }
          if ((String(p->value().c_str()).compareTo("off")==0) && wifiEnabled) {
            wifiEnabled=false;
            if(WiFi.status()!=WL_DISCONNECTED) {
              WiFi.disconnect();
              wifiCurrentStatus=wifiOffStatus;
              forceWifiReconnect=false; //To avoid deadlock in WIFI_RECONNECT_PERIOD chck if a previous WiFi reconnection was ongoing
              uploadSamplesEnabled=false; //To avoid uploading samples tries
              CloudClockCurrentStatus=CloudClockOffStatus;
            }
            uint8_t configVariables=EEPROM.read(0x08);
            configVariables&=0xEF; //Set wifiEnabled bit to false (disabled)
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
          }
        }
      }
    }
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on("/basic4", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    int params = request->params();
    bool updateEEPROM=false;
    char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_BASICCONFIG_PAGE, String(), false, processor);

    //Authentication is required
    if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
      //Setup a new response to send AuthenticationRequest headers in the container.html answer
      auxResp->setCode(401);
      auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
      request->send(auxResp);
    }
    else
    {
      for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST AdminUser_Form value
          if (String(p->name()).compareTo("UserName")==0) {
            memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);
            memcpy(auxUserName,p->value().c_str(),p->value().length()); //End null not included
            if (userName.compareTo(auxUserName)!=0) {
              userName=p->value();
              EEPROM.put(0x2A8,auxUserName);
              updateEEPROM=true;
            }
          }
          if (String(p->name()).compareTo("UserPssw")==0) {
            memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);
            memcpy(auxUserPssw,p->value().c_str(),p->value().length()); //End null not included
            if (userPssw.compareTo(auxUserPssw)!=0) {
              userPssw=p->value();
              EEPROM.put(0x2B3,auxUserPssw);
              updateEEPROM=true;
            }
          }
        }
      }
      if (updateEEPROM) EEPROM.commit();
      request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    }

    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on("/basic2", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    

    reconnectWifiAndRestartWebServer=false;
    resyncNTPServer=false;
    int params = request->params();
    bool updateEEPROM=false;
    char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
         auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
    
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);
    
      if(p->isPost()){
        // HTTP POST SSID value
        if (String(p->name()).compareTo("SSID")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[0].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[0]=String(p->value().c_str());
            EEPROM.put(0x0D,auxSSID);
            reconnectWifiAndRestartWebServer=true; //Always reconnect
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW value
        if (String(p->name()).compareTo("PSSW")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[0].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[0]=String(p->value().c_str());
            EEPROM.put(0x2E,auxPSSW);
            reconnectWifiAndRestartWebServer=true;  //Always reconnect
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE value
        if (String(p->name()).compareTo("SITE")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[0].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[0]=String(p->value().c_str());
            EEPROM.put(0x6E,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST SSID_BK1 value
        if (String(p->name()).compareTo("SSID_BK1")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[1].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[1]=String(p->value().c_str());
            EEPROM.put(0x79,auxSSID);
            if (wifiCred.activeIndex>=1) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK1 or SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW_BK1 value
        if (String(p->name()).compareTo("PSSW_BK1")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[1].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[1]=String(p->value().c_str());
            EEPROM.put(0x9A,auxPSSW);
            if (wifiCred.activeIndex>=1) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK1 or SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE_BK1 value
        if (String(p->name()).compareTo("SITE_BK1")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[1].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[1]=String(p->value().c_str());
            EEPROM.put(0xDA,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST SSID_BK2 value
        if (String(p->name()).compareTo("SSID_BK2")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[2].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[2]=String(p->value().c_str());
            EEPROM.put(0xE5,auxSSID);
            if (wifiCred.activeIndex==2) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW_BK2 value
        if (String(p->name()).compareTo("PSSW_BK2")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[2].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[2]=String(p->value().c_str());
            EEPROM.put(0x106,auxPSSW);
            if (wifiCred.activeIndex==2) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE_BK2 value
        if (String(p->name()).compareTo("SITE_BK2")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[2].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[2]=String(p->value().c_str());
            EEPROM.put(0x146,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP1 value
        if (String(p->name()).compareTo("NTP1")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[0].compareTo(auxNTP)!=0) {
            ntpServers[0]=String(p->value().c_str());
            EEPROM.put(0x151,auxNTP);
            resyncNTPServer=true; //Always resync
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP2 value
        if (String(p->name()).compareTo("NTP2")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[1].compareTo(auxNTP)!=0) {
            ntpServers[1]=String(p->value().c_str());
            EEPROM.put(0x191,auxNTP);
            if (ntpServerIndex>=1) resyncNTPServer=true; //Only resync if connected to NTP with lower precedence
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP3 value
        if (String(p->name()).compareTo("NTP3")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[2].compareTo(auxNTP)!=0) {
            ntpServers[2]=String(p->value().c_str());
            EEPROM.put(0x1D1,auxNTP);
            if (ntpServerIndex>=2) resyncNTPServer=true; //Only resync if connected to NTP with lower precedence
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP4 value
        if (String(p->name()).compareTo("NTP4")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[3].compareTo(auxNTP)!=0) {
            ntpServers[3]=String(p->value().c_str());
            EEPROM.put(0x211,auxNTP);
            if (ntpServerIndex>=3) resyncNTPServer=true; //Only resync if connected to NTP with lower precedence
            updateEEPROM=true;
          }
        }
        // HTTP POST TimeZone value
        if (String(p->name()).compareTo("TimeZone")==0) {
          memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
          //memcpy(auxTZEnvVar,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          memcpy(auxTZEnvVar,
                 p->value().substring(p->value().indexOf('$')+1).c_str(),
                 p->value().substring(p->value().indexOf('$')+1).length()); //End null not included
          if (TZEnvVariable.compareTo(auxTZEnvVar)!=0) {
            //TZEnvVariable=p->value();
            TZEnvVariable=p->value().substring(p->value().indexOf('$')+1);
            EEPROM.put(0x251,auxTZEnvVar);
            resyncNTPServer=true; //Always resync NTP
            updateEEPROM=true;
          }

          memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
          memcpy(auxTZName,p->value().substring(0,p->value().indexOf('$')).c_str(),p->value().indexOf('$')); //End null not included
          if (TZName.compareTo(auxTZName)!=0) {
            TZName=p->value().substring(0,p->value().indexOf('$'));
            EEPROM.put(0x28A,auxTZName);
            resyncNTPServer=true; //Always resync NTP
            updateEEPROM=true;
          }
        }
      }
    }
    
    if (updateEEPROM) EEPROM.commit();
    //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"0.5; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);

    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on("/basic3", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    
    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST Display ON/OFF value
        if (p->name().compareTo("Display_enabled")==0) {
          if (p->value().compareTo("off")==0) {
            autoBackLightOff=false;
            //Turn off back light
            digitalWrite(PIN_TFT_BACKLIGHT,LOW);
            lastTimeTurnOffBacklightCheck=loopStartTime+millis();
          }
          if (p->value().compareTo("on")==0) {
            autoBackLightOff=true;
            tft.fillScreen(MENU_BACK_COLOR); //clean the screen before turning the display on
            digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
          }
        }
        // HTTP POST DisplaySw_enabled value
        if (p->name().compareTo("DisplaySw_enabled")==0) {
          if ((p->value().compareTo("on")==0) && !autoBackLightOff) {
            autoBackLightOff=true;
            //Turn off back light
            digitalWrite(PIN_TFT_BACKLIGHT,LOW);
            lastTimeTurnOffBacklightCheck=loopStartTime+millis();
          }
          if ((p->value().compareTo("off")==0) && autoBackLightOff) {
            autoBackLightOff=false;
            tft.fillScreen(MENU_BACK_COLOR); //clean the screen before turning the display on
            digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
          }
        }
        // HTTP POST BatteryMode value
        if (String(p->name()).compareTo("BatteryMode")==0) {
          if (String(p->value().c_str()).compareTo("Reduced")==0) {
            if (lowestEnergy==energyCurrentMode || fullEnergy==energyCurrentMode) {/*do nothing as charge <= Threshold*/ }; 
            // Save new value if configSavingEnergyMode changed
            if (lowestEnergy==configSavingEnergyMode) {
              configSavingEnergyMode=reducedEnergy;
              uint8_t configVariables=EEPROM.read(0x08);
              configVariables&=0xFD; //Set configSavingEnergyMode bit to false (lowestEnergy)
              EEPROM.write(0x08,configVariables);
              EEPROM.commit();
            }
          } else if (String(p->value().c_str()).compareTo("Lowest")==0) {
            //if currently fullEnergy, don't change it as USB is plugged
            if (reducedEnergy==energyCurrentMode) energyCurrentMode=lowestEnergy; 
            // Save new value if configSavingEnergyMode changed
            if (reducedEnergy==configSavingEnergyMode) {
              configSavingEnergyMode=lowestEnergy;
              uint8_t configVariables=EEPROM.read(0x08);
              configVariables|=0x02; //Set configSavingEnergyMode bit to true (reducedEnergy)
              EEPROM.write(0x08,configVariables);
              EEPROM.commit();
            }
          }
        }
        // HTTP POST DisplayMode value
        if (String(p->name()).compareTo("DisplayMode")==0) {
          if (String(p->value().c_str()).compareTo("CO2")==0) {
            currentState=displayingSampleFixed;
            lastDisplayMode=co2LastDayGraph; //Force re-rendering CO2 values in the main screen
            displayMode=sampleValue;
          }
          else if (String(p->value().c_str()).compareTo("LastHour")==0) {
            currentState=displayingCo2LastHourGraphFixed;
            lastDisplayMode=sampleValue;
            displayMode=co2LastHourGraph;
          }
          else if (String(p->value().c_str()).compareTo("LastDay")==0) {
            currentState=displayingCo2LastDayGraphFixed;
            lastDisplayMode=co2LastHourGraph;
            displayMode=co2LastDayGraph;
          }
          else if (String(p->value().c_str()).compareTo("Sequential")==0) {
            currentState=displayingSequential;
            lastDisplayMode=co2LastDayGraph; //Force re-rendering CO2 values in the main screen
            displayMode=sampleValue;
          }
          forceDisplayRefresh=true;
          forceDisplayModeRefresh=true;
          previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD;
        }
      }
    }
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on("/cloud", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    int params = request->params();
    uint8_t currentConfigVariables,configVariables=0;
    bool updateEEPROM=false,connectMqtt=false,disconnectMqtt=false;
    char auxUserName[MQTT_USER_CREDENTIAL_LENGTH],auxUserPssw[MQTT_PW_CREDENTIAL_LENGTH];
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CLOUDCONFIG_PAGE, String(), false, processor);
    byte auxCounter=0;

    currentConfigVariables=EEPROM.read(0x2BE);
    wifiCred.SiteAllow[0]=false;wifiCred.SiteAllow[1]=false;wifiCred.SiteAllow[2]=false;
    
    //Authentication is required
    if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
      //Setup a new response to send AuthenticationRequest headers in the container.html answer
      auxResp->setCode(401);
      auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
      request->send(auxResp);
    }
    else 
    {
      //Checking checkbox. Name is sent only if checked. 
      for(int i=0;i<params;i++) {
        AsyncWebParameter* p = request->getParam(i);

        if(p->isPost()){
          // HTTP POST SITE_ALLOW_NAME value
          if (p->name().compareTo("SITE_ALLOW_NAME")==0) { //Check-box checked
            wifiCred.SiteAllow[0]=true;
            configVariables|= 0x01;
          }
          // HTTP POST SITE_BK1_ALLOW_NAME value
          else if (p->name().compareTo("SITE_BK1_ALLOW_NAME")==0) { //Check-box checked
            wifiCred.SiteAllow[1]=true;
            configVariables|=0x02;
          }
          // HTTP POST SITE_BK2_ALLOW_NAME value
          else if (p->name().compareTo("SITE_BK2_ALLOW_NAME")==0) { //Check-box checked
            wifiCred.SiteAllow[2]=true;
            configVariables|=0x04;
          }
        }
      }

      if (currentConfigVariables != configVariables) {
        EEPROM.write(0x2BE,configVariables);
        updateEEPROM=true;
        if (uploadSamplesEnabled) lastTimeUploadSampleCheck=nowTimeGlobal-uploadSamplesPeriod; //Force to update status of the cloud server (CloudSyncCurrentStatus) in the next loop round
      }
      
      //Checking the rest of parameters different to checkbox
      for(int i=0;i<params;i++) {
        AsyncWebParameter* p = request->getParam(i);

        if(p->isPost()){
          // HTTP POST Cloud_enabled value
          if (p->name().compareTo("Cloud_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !uploadSamplesEnabled) {
              uploadSamplesEnabled=true;
              if (uploadSamplesEnabled) lastTimeUploadSampleCheck=nowTimeGlobal-uploadSamplesPeriod; //Force to update status of the cloud server (CloudSyncCurrentStatus) in the next loop round
              configVariables=EEPROM.read(0x08) | 0x04; //Set uploadSamplesEnabled bit to true (enabled)
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
            if ((p->value().compareTo("off")==0) && uploadSamplesEnabled) {
              uploadSamplesEnabled=false;
              if (CloudSyncCurrentStatus==CloudSyncOnStatus) CloudSyncCurrentStatus=CloudSyncOffStatus;
              configVariables=EEPROM.read(0x08) & 0xFB;
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
          }
          // HTTP POST MQTT_enabled value
          if (p->name().compareTo("MQTT_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !mqttServerEnabled) {
              mqttServerEnabled=true;
              connectMqtt=true;auxCounter|=0x01;
              configVariables=EEPROM.read(0x08) | 0x40; //Set mqttServerEnabled bit to true (enabled)
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
            if ((p->value().compareTo("off")==0) && mqttServerEnabled) {
              mqttServerEnabled=false;
              disconnectMqtt=true;
              configVariables=EEPROM.read(0x08) & 0xBF;
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
          }
          // HTTP POST Secure_MQTT_enabled value
          if (p->name().compareTo("Secure_MQTT_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !secureMqttEnabled) {
              secureMqttEnabled=true;
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x02;}  //Connect with authentication
              configVariables=EEPROM.read(0x08) | 0x80; //Set secureMqttEnabled bit to true (enabled)
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
            if ((p->value().compareTo("off")==0) && secureMqttEnabled) {
              secureMqttEnabled=false;
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x04;} //Connect without authentication
              configVariables=EEPROM.read(0x08) & 0x7F;
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
          }
          // HTTP POST MQTTSERVER value
          if (p->name().compareTo("MQTTSERVER")==0) {
            char auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH];
            memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);
            memcpy(auxMQTT,p->value().c_str(),p->value().length()); //End null not included
            if (mqttServer.compareTo(auxMQTT)!=0) {
              mqttServer=p->value();
              EEPROM.put(0x2BF,auxMQTT);
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x08;}
              updateEEPROM=true;
            }
          }
          // HTTP POST MQTTTOPIC value
          if (p->name().compareTo("MQTTTOPIC")==0) {
            char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
            memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
            memcpy(auxMqttTopicPrefix,p->value().c_str(),p->value().length()); //End null not included
            if (mqttTopicPrefix.compareTo(auxMqttTopicPrefix)!=0) {
              mqttTopicPrefix=p->value();
              if (mqttTopicPrefix.charAt(mqttTopicPrefix.length()-1)!='/') mqttTopicPrefix+="/"; //Adding slash at the end if needed
              mqttTopicPrefix.toCharArray(auxMqttTopicPrefix,mqttTopicPrefix.length()+1);
              EEPROM.put(0x315,auxMqttTopicPrefix);
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x10;}
              updateEEPROM=true;
              mqttTopicName=mqttTopicPrefix+device; //Adding the device name to the MQTT Topic name
            }
          }
          // HTTP POST MQTTUserName value
          if (String(p->name()).compareTo("MQTTUserName")==0) {
            memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);
            memcpy(auxUserName,p->value().c_str(),p->value().length()); //End null not included
            if (mqttUserName.compareTo(auxUserName)!=0) {
              mqttUserName=p->value();
              EEPROM.put(0x2FF,auxUserName);
              updateEEPROM=true;
              if (mqttServerEnabled && secureMqttEnabled) {connectMqtt=true;auxCounter|=0x20;}
            }
          }
          // HTTP POST MQTTUserPssw value
          if (String(p->name()).compareTo("MQTTUserPssw")==0) {
            memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);
            memcpy(auxUserPssw,p->value().c_str(),p->value().length()); //End null not included
            if (mqttUserPssw.compareTo(auxUserPssw)!=0) {
              mqttUserPssw=p->value();
              EEPROM.put(0x30A,auxUserPssw);
              updateEEPROM=true;
              if (mqttServerEnabled && secureMqttEnabled) {connectMqtt=true;auxCounter|=0x40;}
            }
          }
        }
      }
    }
    if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [webServer cloud] - connectMqtt="+String(connectMqtt)+", disconnectMqtt="+String(disconnectMqtt)+", auxCounter=0x"+String(auxCounter,HEX));
    if (connectMqtt) {
      if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [webServer cloud] - mqttClient.connected()="+String(mqttClient.connected()));
      //Disconnect first from the current MQTT server
      if (mqttClient.connected()) mqttClient.disconnect(true);
      while (mqttClient.connected()) {}; //Wait till get disconnected
      
      //Connect to the MQTT broker
      if (WiFi.status()==WL_CONNECTED && !mqttClient.connected() && mqttServerEnabled) { //Connect to MQTT broker again
        if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [webServer cloud] - about to init Mqtt client");
        mqttClientInit(false,true,false);
      }
      if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [webServer cloud] - about to exit");
    }
    if (disconnectMqtt) {
      /*Code here to unsuscribe from the MQTT broker*/
      
      //Disconnect to the MQTT broker
      mqttClient.disconnect(true);
    }
    if (updateEEPROM) EEPROM.commit();
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);

    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  webServer.on("/bluetooth", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST Bluetooth_enabled value
        if (String(p->name()).compareTo("Bluetooth_enabled")==0) {
          if ((String(p->value().c_str()).compareTo("on")==0) && !bluetoothEnabled) {
            bluetoothEnabled=true;
            if (BLEClurrentStatus==BLEOffStatus) BLEClurrentStatus=BLEOnStatus;
            uint8_t configVariables=EEPROM.read(0x08);
            configVariables|=0x08; //Set bluetoothEnabled bit to true (enabled)
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
          }
          if ((String(p->value().c_str()).compareTo("off")==0) && bluetoothEnabled) {
            bluetoothEnabled=false;
            if (BLEClurrentStatus==BLEOnStatus) BLEClurrentStatus=BLEOffStatus;
            uint8_t configVariables=EEPROM.read(0x08);
            configVariables&=0xF7; //Set bluetoothEnabled bit to false (disabled)
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
          }
        }
      }
    }
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on(WEBSERVER_DEFAULTCONF_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    reconnectWifiAndRestartWebServer=false;
    resyncNTPServer=false;
    factoryReset=false;
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    updateCommand=-1;
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);

    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST Bluetooth_enabled value
        if (p->name().compareTo("Maintenance_Default_Send")==0) {
          factoryReset=true;

          //Authentication is required
          if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
            fileUpdateError=ERROR_UPLOAD_FILE_NOAUTH;
            //Setup a new response to send AuthenticationRequest headers in the container.html answer
            auxResp->setCode(401);
            auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          } 
          else {   
            reconnectWifiAndRestartWebServer=true;
            resyncNTPServer=true;
            factoryConfReset();
            EEPROM.commit();

            //Setting other staff non EEPROM-related
            autoBackLightOff=false;
            tft.fillScreen(MENU_BACK_COLOR); //clean the screen before turning the display on
            digitalWrite(PIN_TFT_BACKLIGHT,HIGH);
            currentState=displayingSequential;
            lastDisplayMode=displayMode;
            displayMode=sampleValue;
            forceDisplayRefresh=true;
            forceDisplayModeRefresh=true;
            previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD;
          }
          //Factory Reset is done after serving WEBSERVER_CONTAINER_PAGE
        }
      }
    }
    //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"0; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
    //request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on(WEBSERVER_DEVICERESET_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    //This code is run after uploading the file
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    
    deviceReset=false;
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    updateCommand=-1;
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);

    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST Maintenance_Reset_Send value
        if (p->name().compareTo("Maintenance_Reset_Send")==0) {
          deviceReset=true;
          
          //Authentication is required
          if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
            fileUpdateError=ERROR_UPLOAD_FILE_NOAUTH;
            //Setup a new response to send AuthenticationRequest headers in the container.html answer
            auxResp->setCode(401);
            auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          }
          //Device Reset is done after serving WEBSERVER_CONTAINER_PAGE
        }
      }
    }
    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  webServer.on(WEBSERVER_UPLOADFILE_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    //This code is run after uploading the file
      int params = request->params();

      //Checking if the POST comes from the browser with the active cookie
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //Wrong cookie
        //It's supposed that errorOnWrongCookie was updated propery during the upload handler
        deviceReset=false; //This flags prevents from serving wrong content in container.html when there's a valid ongoing upload in pararell
        fileUpdateError=errorOnWrongCookie;
        if (errorOnWrongCookie==ERROR_UPLOAD_FILE_NOERROR) if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - Coming from Wrong cookie upload but there is no ERROR. It's strange (meditation required). Serving container.html.");

        if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) { 
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - ERROR - Still no active cookie: Authentication required but not detected.");
          //Setup a new response to send AuthenticationRequest headers in the container.html answer
          AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
          auxResp->setCode(401);
          auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          request->send(auxResp);
          //fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
        }
        else {
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - Coming from Wrong cookie upload with ERROR code 0x"+String(fileUpdateError,HEX)+". Serving container.html.");
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
        }

         errorOnWrongCookie=ERROR_UPLOAD_FILE_NOERROR;
      }
      else {
        //Active cookie
        //Let's check the errors and then send the response
        fileUpdateError=errorOnActiveCookie;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - Coming from Active cookie upload with ERROR code 0x"+String(fileUpdateError,HEX)+". Serving container.html.");

        if (fileUpdateError==ERROR_UPLOAD_FILE_NOERROR) //Check Form parameters only if no errors while uploading the file 
        { 
          int i;
          for(i=0;i<params;i++) {
            AsyncWebParameter* p = request->getParam(i);

            if(p->isPost()){
              // HTTP POST Maintenance_Reset_Send value
              if (p->name().compareTo("select_file")==0 && !p->isFile())
                fileUpdateError=ERROR_UPLOAD_FILE_NOFILE;
              if (p->name().compareTo("select_file")!=0 && p->name().compareTo("file-upload")!=0) 
                fileUpdateError=ERROR_UPLOAD_FILE_POST_PRM_NAME;
              else if (p->name().compareTo("select_file")==0) {          
                //Everything OK  
                String fileName=p->value();
                if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - fileName="+fileName);
              }
            }
          }

          if (i!=2) fileUpdateError=ERROR_UPLOAD_FILE_POST_PRM_NUMBER;
        }

        if (fileUpdateError==ERROR_UPLOAD_FILE_NOERROR) {
          //No error. Let's restart the device
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - Upload finished and Post treated with no errors. Should reboot after the file is uploaded if no additional errros.");
          deviceReset=true;
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
        }
        else if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) { 
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST] - ERROR: Authentication required but not detected.");
          //Setup a new response to send AuthenticationRequest headers in the container.html answer
          AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
          auxResp->setCode(401);
          auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          request->send(auxResp);
          //fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
        }
        else {
          //Something was wrong with the file update process. Inform about it.
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware POST - 0] - ERROR with File update. Serving container.html.");
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
        }

        errorOnActiveCookie=ERROR_UPLOAD_FILE_NOERROR;
      }
      
      //Init variable to let them ready for the next upload
      updateCommand=-1;
      fileUpdateSize=0;
      fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    //This code is run first
    fileUpdateName=filename;

    //Doing some checks first before allowing the file upload

    //Cookie is needed to allow to upload the file
    if (!request->hasHeader("Cookie")) {
      errorOnWrongCookie=ERROR_UPLOAD_FILE_NOCOOKIE;
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - No Cookie detected. Close socket and Return.");  
      return;
    }

    //Check cookie length and cookie name
    if ((request->getHeader("Cookie")->value().length() > COOKIE_SIZE-1) ||
        0!=String("id-"+device).compareTo(request->getHeader("Cookie")->value().substring(0,request->getHeader("Cookie")->value().indexOf('='))) )
    {
      errorOnWrongCookie=ERROR_UPLOAD_FILE_BADCOOKIE_FORMAT;
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Bad cookie format: "+request->getHeader("Cookie")->value()+", size "+String(request->getHeader("Cookie")->value().length())+" greater than "+String(COOKIE_SIZE-1)+" or wrong name. Close socket and return. Comparing "+String("id-"+device)+" with "+request->getHeader("Cookie")->value().substring(0,request->getHeader("Cookie")->value().indexOf('=')));
      return;
    }

    if (Update.progress()!=0) {
      //There's one ongoing file upload. Let's check if this is the right cookie
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //No right cookie. Bye
        errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - There is an ongoing file upload. Return.");
        return;
      }
    }

    //Authentication is needed
    if(!request->authenticate(userName.c_str(), userPssw.c_str())){
      errorOnWrongCookie=ERROR_UPLOAD_FILE_NOAUTH;
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Authentication required, index="+String(index)+"\n   - Faulty cookie="+request->getHeader("Cookie")->value());
      return;
    }

    //No further process if there are previous errors
    //No checking errors in this point prevents from stoping right ongloing uploads from
    // someone using a different browser. Cookies conditions are checked below.
    if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
      if (errorOnWrongCookie!=ERROR_UPLOAD_FILE_NOERROR) return;
    }
    else {
      if (errorOnActiveCookie!=ERROR_UPLOAD_FILE_NOERROR) return;
    }

    //Upload handler chunks in data
    if (!index) {
      if (Update.progress()==0) {
        if (memcmp(currentSetCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())==0) {
          //It's the same cookie than the one created when serving the maintenance.html page
          //Saving current cookie
          fileUpdateSize=0;
          memcpy(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length()); //End null not included
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Cookie="+String(activeCookie));
        }
        else {
          //Not the same cookie than the one created when serving the maintenance.html page
          //Probably the POST comes from a wrong request. Bye
          errorOnWrongCookie=ERROR_UPLOAD_FILE_BADCOOKIE;
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Bad Cookie received="+request->getHeader("Cookie")->value()+" instead of "+String(currentSetCookie));
          return;
        }
      }
      else {
        //Checking if it is the current cookie
        if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())==0) {
          //Coming from abort action from the same broswer. Abort previous upload and start again
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Detected upload abortion from the same broswer. Reset and starting again");
          Update.abort(); 
          fileUpdateSize=0;
        }
        else {
          //Coming from different browser. But there is an ongoing upload. Error. Bye
          //It's supposed that this point is never reached
          errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - There is an ongoing file upload. Return.");
          return;
        }
      }

      //Beyond this point the browser has the active cookie

      if (filename.compareTo("firmware.bin")==0) updateCommand=U_FLASH;
      else if (filename.compareTo("filesystem.bin")==0) updateCommand=U_SPIFFS;
      else {
        errorOnActiveCookie=ERROR_UPLOAD_FILE_WRONG_FILE_NAME;
        updateCommand=-1;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error Wrong file name, filename="+filename+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize+len));
        return;
      }

      if (!Update.begin(UPDATE_SIZE_UNKNOWN, updateCommand)) { // Start with max available size
        Update.printError(Serial);
        errorOnActiveCookie=ERROR_UPLOAD_FILE_UPDATE_BEGIN;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error Update.begin()"+String(Serial)+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize+len));
        return;
      }
    }

    // Write chunked data to the free sketch space if it's the same cookie (same upload session)
    if (len) {
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //No same cookie than the one used for starting the file upload
        //It's supposed that this point is never reached
        errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Bad cookie detected as there is an ongoing file upload. Cookie detected: "+request->getHeader("Cookie")->value()+" rather than "+String(activeCookie)+" in index="+String(index)+" len="+String(len));
        return;
      }
      
      fileUpdateSize+=len;

      //Check FLASH memory left always (both firmware.bin and spiffs.bin must fit into the remaining space)
      if (filename.compareTo("firmware.bin")==0) {
        //OTA partition for firmware.bin
        if ((int)(OTAAvailableSize-fileUpdateSize)<0) {
          errorOnActiveCookie=ERROR_UPLOAD_FILE_NOFLASHSPACE_LEFT;
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error No OTA space left in FLASH memory. OTAAvailableSize ("+String(OTAAvailableSize)+") - fileUpdateSize ("+String(fileUpdateSize)+") = "+String((int)(OTAAvailableSize-fileUpdateSize))+" B"+", index="+String(index)+", len="+String(len));
          Update.abort();
          return;
        }

        //Check header if it's really a binary file
        if (index==0) {
          //Checking the Magic Byte
          uint8_t *dataPtr=data;
          if (*dataPtr != BINARY_HD_MAGIC_NUMBER || len<(BINARY_HD_MAGIC_WORD_ADDR+3)) {
            errorOnActiveCookie=ERROR_UPLOAD_FILE_NOBIN_FILE;
            if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error Binary file. Magic Number=0x"+String(*dataPtr,HEX)+" rather than 0x"+String(BINARY_HD_MAGIC_NUMBER,HEX)+" or len("+String(len)+") < BINARY_HD_MAGIC_WORD_ADDR+3("+String(BINARY_HD_MAGIC_WORD_ADDR+3)+")");
            Update.abort();
            return;
          }
          //Checking the Magic WORD
          for (int i=0; i<BINARY_HD_MAGIC_WORD_ADDR; i++) dataPtr++;
          uint8_t auxByte1=*dataPtr;
          uint32_t receivedMagic=auxByte1;
          dataPtr++;auxByte1=*dataPtr;receivedMagic|=(auxByte1<<8);
          dataPtr++;auxByte1=*dataPtr;receivedMagic|=(auxByte1<<16);
          dataPtr++;auxByte1=*dataPtr;receivedMagic|=(auxByte1<<24);
          if (receivedMagic != ((uint32_t) BINARY_HD_MAGIC_WORD)) {
            errorOnActiveCookie=ERROR_UPLOAD_FILE_NOBIN_FILE;
            if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error Binary file. Magic Word=0x"+String((uint32_t) receivedMagic,HEX)+" rather than 0x"+String(BINARY_HD_MAGIC_WORD,HEX));
            Update.abort();
            return;
          }
        }
      }
      else {
        //SPIFFS partition for spiffs.bin
        if ((int)(SPIFFSAvailableSize-fileUpdateSize)<0) {
          errorOnActiveCookie=ERROR_UPLOAD_FILE_NOSPIFFSPACE_LEFT;
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error No SPIFFS space left in FLASH memory. SPIFFSAvailableSize ("+String(SPIFFSAvailableSize)+") - fileUpdateSize ("+String(fileUpdateSize)+") = "+String((int)(SPIFFSAvailableSize-fileUpdateSize))+" B"+", index="+String(index)+", len="+String(len));
          Update.abort();
          return;
        }

        //Check if it's really a SPIFFS partition
        u16_t bix=(u16_t)(fileUpdateSize/SPIFFS_CFG_LOG_BLOCK_SZ); //Current block index
        size_t magicAddr=(bix*SPIFFS_CFG_LOG_BLOCK_SZ)+SPIFFS_MAGIC_ADDR; //Magic address for the current block
        if ((fileUpdateSize-len)<=magicAddr && fileUpdateSize>=(magicAddr+2)) {
          //Magic number received in this chunk
          u16_t calculatedMagic=(0x20140529 ^ SPIFFS_CFG_LOG_PAGE_SZ ^ ((SPIFFSAvailableSize/SPIFFS_CFG_LOG_BLOCK_SZ) - (bix)));
          uint8_t *dataPtr=data;
          for (int i=0; i<magicAddr-(fileUpdateSize-len); i++) dataPtr++;
          uint8_t auxByte1=*dataPtr;
          dataPtr++;
          uint8_t auxByte2=*dataPtr;
          u16_t receivedMagic=(auxByte2<<8) | auxByte1;
          if (receivedMagic!=calculatedMagic) {
            //No SPIFFS partition
            errorOnActiveCookie=bix==0?ERROR_UPLOAD_FILE_NOSPIFFS_FILE:ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE;
            if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error No SPIFFS file. Index="+String(index)+", block="+String(bix)+", magic addr=0x"+String(magicAddr,HEX)+", calculatedMagic=0x"+String(calculatedMagic,HEX)+", receivedMagic=0x"+String(receivedMagic,HEX));
            Update.abort();
            return;
          }
        }

        if(!index && len<magicAddr) {
          //File size lower than SPIFFS header, so No SPIFFS partition
          errorOnActiveCookie=bix==0?ERROR_UPLOAD_FILE_NOSPIFFS_FILE:ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE;
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error No SPIFFS file.");
          Update.abort();
          return;
        }
      }
      
      //Check SPIFFS memory left to upload files into SPIFFS
      //This code should be used if files are uploaded to SPIFFS (no filesystem.bin)
      /*if ((int)(fileSystemSize-fileSystemUsed-fileUpdateSize)<=0) {
        fileUpdateError=ERROR_UPLOAD_FILE_NOSPIFFSPACE_LEFT;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error No SPIFFS memory left. fileSystemSize ("+String(fileSystemSize)+") - fileSystemUsed ("+String(fileSystemUsed)+") - fileUpdateSize ("+String(fileUpdateSize)+") = "+String((int)(fileSystemSize-fileSystemUsed-fileUpdateSize))+" B");
        Update.abort();
        return;
      }*/

      if (Update.write(data, len) != len) {
          Update.printError(Serial);
          errorOnActiveCookie=ERROR_UPLOAD_FILE_UPDATE_WRITE;
          if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error Update.write(), Serial="+String(Serial)+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize));
          Update.abort();
          return;
      }
    }

    if (final) { // if the final flag is set then this is the last frame of data
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //No same cookie than the one used for starting the file upload. Send error message
        //It's supposed that this point is never reached
        errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - In final block, bad cookie detected as there is an ongoing file upload. Cookie detected: "+request->getHeader("Cookie")->value()+" rather than "+String(activeCookie)+" in index="+String(index)+" len="+String(len));
        return;
      }

      if (errorOnActiveCookie!=ERROR_UPLOAD_FILE_NOERROR) return;

      if (Update.end(true)) { //true to set the size to the current progress
        //Inform Reset will be done and then reset the device
        errorOnActiveCookie=ERROR_UPLOAD_FILE_NOERROR;
        if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Update Success - Should reboot now"+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize));
        return;
      } 
      else {
        Update.printError(Serial);
        errorOnActiveCookie=ERROR_UPLOAD_FILE_UPDATE_END;
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+" [maintenance_upload_firmware upload] - Error Update.end()"+String(Serial)+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize));}
        Update.abort();
        return;
      }
    }

    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Handle Web Server Events
  webEvents.onConnect([](AsyncEventSourceClient *client){
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [webEvents.onConnect] - Client reconnected! Last message ID that it got is: "+String(client->lastId()));
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [webEvents.onConnect] - Client reconnected! client->client()->remoteIP()="+client->client()->remoteIP().toString());
    if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [webEvents.onConnect] - Client reconnected! client->client()->remotePort()="+String(client->client()->remotePort()));

    if(client->lastId()){
      if (debugModeOn) Serial.println(String(nowTimeGlobal)+" [webEvents.onConnect] - Client reconnected! Last message ID that it got is:"+String(client->lastId()));
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  webServer.addHandler(&webEvents);
  
  // Start web server
  webServer.begin();

  return NO_ERROR;
}

String processorAP(const String& var) {
  if (var == "NTP1") {
    #ifdef NTP_SERVER
      return String(NTP_SERVER);
    #else
      return "Mandatory if WiFi enabled"
    #endif
  } else if (var == "NTP1_VALUE") {
    #ifdef NTP_SERVER
      return String(NTP_SERVER);
    #else
      return String();
    #endif
  } else if (var == "NTP2") {
    #ifdef NTP_SERVER2
      return String(NTP_SERVER2);
    #else
      return String();
    #endif
  } else if (var == "NTP2_VALUE") {
    #ifdef NTP_SERVER2
      return String(NTP_SERVER2);
    #else
      return String();
    #endif
  } else if (var == "NTP3") {
    #ifdef NTP_SERVER3
      return String(NTP_SERVER3);
    #else
      return String();
    #endif
  } else if (var == "NTP3_VALUE") {
    #ifdef NTP_SERVER3
      return String(NTP_SERVER3);
    #else
      return String();
    #endif
  } else if (var == "NTP4") {
  #ifdef NTP_SERVER4
      return String(NTP_SERVER4);
    #else
      return String();
    #endif
  } else if (var == "NTP4_VALUE") {
    #ifdef NTP_SERVER4
      return String(NTP_SERVER4);
    #else
      return String();
    #endif
  } else if (var == "CONTAINER_HEAD") {
    return String("AP Mode");
  } else if (var == "CONTAINER_TEXT") {
    if (deviceReset) return String("Rebooting the device....<p></p><br>The device is rebooting and will try to connect to the WiFi as per the saved configuration.<br><br>The remainder configuration can be done through the web browser. The device's IP is shown in the Device Configuration Menu.<br><br>If there is no WiFi connection and you need to setup a new WiFi configuration, then set the Factory Reset Option in the Device Configuration Menu.");
    else return String("Other");
  }
  else {
    return String();
  }
}

uint32_t initAPWebServer() {

   // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_APINDEX_PAGE, String(), false, processorAP);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route for root index.html web page
  webServer.on(WEBSERVER_INDEX_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_APINDEX_PAGE, String(), false, processorAP);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });
  
  // Route to load style.css file
  webServer.on(WEBSERVER_CSSSTYLES_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CSSSTYLES_PAGE, "text/css");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to load tswnavbar.css file
  webServer.on(WEBSERVER_CSSNAVBAR_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CSSNAVBAR_PAGE, "text/css");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to load The_IoT_Factory.png file
  webServer.on(WEBSERVER_LOGO_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    request->send(SPIFFS, WEBSERVER_LOGO_ICON, "image/png");

    //Wait to disconnect the connection
    request->onDisconnect([]{
      //Device reset
      if (deviceReset) ESP.restart();
    });
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Route to load favicon.ico file
  webServer.on(WEBSERVER_FAVICON_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_FAVICON_ICON, "image/x-icon");
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  webServer.on("/wifibasic", HTTP_POST, [](AsyncWebServerRequest *request) {
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (isBeaconAdvertising) delay(200); //Wait to iBeacon stops to prevent heap overflow
    
    int params = request->params();
    bool updateEEPROM=false;
    char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
         auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
    
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST SSID value
        if (String(p->name()).compareTo("SSID")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[0].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[0]=String(p->value().c_str());
            EEPROM.put(0x0D,auxSSID);
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW value
        if (String(p->name()).compareTo("PSSW")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[0].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[0]=String(p->value().c_str());
            EEPROM.put(0x2E,auxPSSW);
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE value
        if (String(p->name()).compareTo("SITE")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[0].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[0]=String(p->value().c_str());
            EEPROM.put(0x6E,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST SSID_BK1 value
        if (String(p->name()).compareTo("SSID_BK1")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[1].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[1]=String(p->value().c_str());
            EEPROM.put(0x79,auxSSID);
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW_BK1 value
        if (String(p->name()).compareTo("PSSW_BK1")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[1].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[1]=String(p->value().c_str());
            EEPROM.put(0x9A,auxPSSW);
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE_BK1 value
        if (String(p->name()).compareTo("SITE_BK1")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[1].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[1]=String(p->value().c_str());
            EEPROM.put(0xDA,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST SSID_BK2 value
        if (String(p->name()).compareTo("SSID_BK2")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[2].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[2]=String(p->value().c_str());
            EEPROM.put(0xE5,auxSSID);
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW_BK2 value
        if (String(p->name()).compareTo("PSSW_BK2")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[2].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[2]=String(p->value().c_str());
            EEPROM.put(0x106,auxPSSW);
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE_BK2 value
        if (String(p->name()).compareTo("SITE_BK2")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[2].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[2]=String(p->value().c_str());
            EEPROM.put(0x146,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP1 value
        if (String(p->name()).compareTo("NTP1")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[0].compareTo(auxNTP)!=0) {
            ntpServers[0]=String(p->value().c_str());
            EEPROM.put(0x151,auxNTP);
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP2 value
        if (String(p->name()).compareTo("NTP2")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[1].compareTo(auxNTP)!=0) {
            ntpServers[1]=String(p->value().c_str());
            EEPROM.put(0x191,auxNTP);
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP3 value
        if (String(p->name()).compareTo("NTP3")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[2].compareTo(auxNTP)!=0) {
            ntpServers[2]=String(p->value().c_str());
            EEPROM.put(0x1D1,auxNTP);
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP4 value
        if (String(p->name()).compareTo("NTP4")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[3].compareTo(auxNTP)!=0) {
            ntpServers[3]=String(p->value().c_str());
            EEPROM.put(0x211,auxNTP);
            updateEEPROM=true;
          }
        }
        // HTTP POST TimeZone value
        if (String(p->name()).compareTo("TimeZone")==0) {
          memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
          memcpy(auxTZEnvVar,
                 p->value().substring(p->value().indexOf('$')+1).c_str(),
                 p->value().substring(p->value().indexOf('$')+1).length()); //End null not included
          if (TZEnvVariable.compareTo(auxTZEnvVar)!=0) {
            TZEnvVariable=p->value().substring(p->value().indexOf('$')+1);
            EEPROM.put(0x251,auxTZEnvVar);
            updateEEPROM=true;
          }

          memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
          memcpy(auxTZName,p->value().substring(0,p->value().indexOf('$')).c_str(),p->value().indexOf('$')); //End null not included
          if (TZName.compareTo(auxTZName)!=0) {
            TZName=p->value().substring(0,p->value().indexOf('$'));
            EEPROM.put(0x28A,auxTZName);
            updateEEPROM=true;
          }
        }
      }
    }

    if (updateEEPROM) EEPROM.commit();
    deviceReset=true;
    request->send(SPIFFS, WEBSERVER_APCONTAINER_PAGE, String(), false, processorAP);
    webServerResponding=false;   //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed agin
  });

  // Start server
  webServer.begin();

  return NO_ERROR;
}