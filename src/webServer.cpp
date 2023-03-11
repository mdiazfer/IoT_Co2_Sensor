/* Dealing with Web Server stuff

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
  } else if (var == "BLUETOOTH_ON_CHECKED") {
    if (bluetoothEnabled) return String("checked");
    else return String();
  } else if (var == "BLUETOOTH_OFF_CHECKED") {
    if (!bluetoothEnabled) return String("checked");
    else return String();
  } else if (var == "CONTAINER_HEAD") {
    if (deviceReset) return String("Device Reset");
    else if (factoryReset) return String("Factory Reset");
    else return String("Other");
  } else if (var == "CONTAINER_TEXT") {
    if (deviceReset) return String("Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=80;var timer = maxTime;var auxTimeout=false;var interval = setInterval(function() {if (timer > 0) document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';if (timer === maxTime) {var request = new XMLHttpRequest();request.open('GET', 'http://"+WiFi.localIP().toString()+"', true);request.ontimeout = function(){auxTimeout=true;};request.onloadend = function(){if (auxTimeout === true) document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';else {document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');},1500)};timer=0;};request.onloadstart = function(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};request.send();};if (timer === 0) clearInterval(interval);else timer--;},1000);</script>");
    else if (factoryReset) return String("The configuration of the device has been reset to factory values.<p></p>The WiFi SSIDs need to be setup again as the "+WiFi.SSID()+" SSID has been deleted.<br><br>Web access to the device is not available till the WiFi is setup again.");
    else return String("Other");
  } else if (var == "SETUPERRORS") {
    return String("0x")+String(error_setup,HEX);
  } else if (var == "WIFIERRORS") {
    return String(errorsWiFiCnt);
  } else if (var == "SAMPLEUPDTERRORS") {
    return String(errorsSampleUpts);
  } else if (var == "NTPERRORS") {
    return String(errorsNTPCnt);
  }
  else {
    return String();
  }
}

uint32_t initWebServer() {
  
  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
  });

  // Route for root index.html web page
  webServer.on(WEBSERVER_INDEX_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
  });
  
  // Route to load style.css file
  webServer.on(WEBSERVER_CSSSTYLES_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_CSSSTYLES_PAGE, "text/css");
  });

  // Route to load tswnavbar.css file
  webServer.on(WEBSERVER_CSSNAVBAR_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_CSSNAVBAR_PAGE, "text/css");
  });

  // Route to load The_IoT_Factory.png file
  webServer.on(WEBSERVER_LOGO_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
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
  });
  //webServer.serveStatic(WEBSERVER_LOGO_ICON,SPIFFS,WEBSERVER_LOGO_ICON);

  // Route for  basic.html web page
  webServer.on(WEBSERVER_BASICCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_BASICCONFIG_PAGE, String(), false, processor);
  });

  // Route for cloud.html web page
  webServer.on(WEBSERVER_CLOUDCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_CLOUDCONFIG_PAGE, String(), false, processor);
  });

  // Route for bluetooth.html web page
  webServer.on(WEBSERVER_BLUETOOTHCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_BLUETOOTHCONFIG_PAGE, String(), false, processor);
  });

  // Route for maintenance.html web page
  webServer.on(WEBSERVER_MAINTENANCE_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false, processor);
  });

  // Route for container.html web page
  webServer.on(WEBSERVER_CONTAINER_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
  });

  webServer.on("/basic1", HTTP_POST, [](AsyncWebServerRequest *request) {
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
  });

  webServer.on("/basic2", HTTP_POST, [](AsyncWebServerRequest *request) {
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
  });

  webServer.on("/basic3", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
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
            lastDisplayMode=displayMode;
            displayMode=sampleValue;
          }
          else if (String(p->value().c_str()).compareTo("LastHour")==0) {
            currentState=displayingCo2LastHourGraphFixed;
            lastDisplayMode=displayMode;
            displayMode=co2LastHourGraph;
          }
          else if (String(p->value().c_str()).compareTo("LastDay")==0) {
            currentState=displayingCo2LastDayGraphFixed;
            lastDisplayMode=displayMode;
            displayMode=co2LastDayGraph;
          }
          else if (String(p->value().c_str()).compareTo("Sequential")==0) {
            currentState=displayingSequential;
            lastDisplayMode=displayMode;
            displayMode=sampleValue;
          }
          forceDisplayRefresh=true;
          forceDisplayModeRefresh=true;
          previousLastTimeSampleCheck=nowTimeGlobal-SAMPLE_PERIOD;
        }
      }
    }
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
  });

  webServer.on("/cloud", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    uint8_t currentConfigVariables,configVariables=0;
    bool updateEEPROM=false;

    currentConfigVariables=EEPROM.read(0x2A8);
    wifiCred.SiteAllow[0]=false;wifiCred.SiteAllow[1]=false;wifiCred.SiteAllow[2]=false;
    
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
      EEPROM.write(0x2A8,configVariables);
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
      }
    }
    
    if (updateEEPROM) EEPROM.commit();
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
  });
  
  webServer.on("/bluetooth", HTTP_POST, [](AsyncWebServerRequest *request) {
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
  });

  webServer.on("/maintenance_default_values", HTTP_POST, [](AsyncWebServerRequest *request) {
    reconnectWifiAndRestartWebServer=false;
    resyncNTPServer=false;
    factoryReset=false;
    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST Bluetooth_enabled value
        if (p->name().compareTo("Maintenance_Default_Send")==0) {
          factoryReset=true;
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
      }
    }
    //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"0; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
    request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
  });

  webServer.on("/maintenance_device_reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    deviceReset=false;
    int params = request->params();
    for(int i=0;i<params;i++) {
      AsyncWebParameter* p = request->getParam(i);

      if(p->isPost()){
        // HTTP POST Bluetooth_enabled value
        if (p->name().compareTo("Maintenance_Reset_Send")==0) {
          deviceReset=true;
          //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"30; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
          //Device Reset Configuration
          //ESP.restart();
        }
      }
    }
  });

  // Start server
  webServer.begin();

  return NO_ERROR;
}