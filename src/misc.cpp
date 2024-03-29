#include "misc.h"
#include <EEPROM.h>

void softReset() {
  //Function to release memory and clean Network clases, etc. up
  //Normally it is called when:
  // 1) Detecting the webServer failed to serve pages
  // 2) Failures to load the BLE module
  // 3) BLE unloads because lack of heap
  // 4) Min heap seen since last boot too low
              
  //Other symthoms about memory leaks are (not checkeck)
  // - WIFI connected, but not ping anser
  // - No NTP sync
  // - No MQTT conneciton
  // - No uploadServerRequest to the cloud
  //The function disconect services and goes to deep sleep for just a few millisenconds
  //The init process is done after waking up

  CloudSyncLastStatus=CloudSyncCurrentStatus;CloudClockLastStatus=CloudClockCurrentStatus;MqttSyncLastStatus=MqttSyncCurrentStatus;
  CloudClockCurrentStatus=CloudClockOffStatus;CloudSyncCurrentStatus=CloudSyncOffStatus;MqttSyncCurrentStatus=MqttSyncOffStatus;
  wifiCurrentStatus=wifiOffStatus;
  softResetOn=true;  //Force complete variable init after wake-up
  previousLastTimeBLECheck=lastTimeBLECheck;lastTimeBLECheck=nowTimeGlobal;lastTimeBLEOnCheck=nowTimeGlobal; //Avoid BLE init next loop cycle after waking-up
  lastTimeWebServerCheck=nowTimeGlobal;
  sleepTimer=300000; //us - 300 millisecons

  //Make sure BLE is deinitialized first
  if (bluetoothEnabled && BLEDevice::getInitialized()) { //BLE initiated with no BLE clients connected
    BLEDevice::stopAdvertising();BLEDevice::deinit(false); delay(750); //Stop Advertisings, release memory and give time to execute everything
  }

  //Reset the counter
  minHeapSeen=0xFFFFFFFF; //v1.5.1
  
  //Close Network services and WiFi connection
  SPIFFS.end();
  webServer.end();
  if (mqttServerEnabled) mqttClient.disconnect(true);
  WiFi.disconnect(true,false);
  
  //Going to sleep
  esp_sleep_enable_timer_wakeup(sleepTimer);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0); //1 = High, 0 = Low        
  loopEndTime=loopStartTime+millis();
  esp_deep_sleep_start();
}

void go_to_hibernate() {
  //Going to hibernate (switch the device off)
  if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - [go_to_hibernate] - Time: ");Serial.println("    - Setting up Power Domains OFF before going into Deep Sleep");}
    
  //Save the lastest known value of batCharge
  uint32_t auxBatCharge=(uint32_t) batCharge;
  for (int i=9; i<(9+sizeof(auxBatCharge)); i++) {EEPROM.write(i,(byte) auxBatCharge); auxBatCharge=auxBatCharge>>8;}
  EEPROM.commit();

  //Set all the power domains OFF
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
  #if SOC_PM_SUPPORT_CPU_PD
    esp_sleep_pd_config(ESP_PD_DOMAIN_CPU,         ESP_PD_OPTION_OFF);
  #endif
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M,         ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,       ESP_PD_OPTION_OFF);

  //Set ext1 trigger to wake up.
  if (debugModeOn) {Serial.println("    - Setup ESP32 to wake up when ext1 GPIO "+String(GPIO_NUM_35)+" is LOW");}
  esp_sleep_enable_ext1_wakeup(0x800000000, ESP_EXT1_WAKEUP_ALL_LOW); //GPIO_NUM_35=2^35 mask //Button1
  
  loopEndTime=loopStartTime+millis();
  if (debugModeOn) {Serial.println(String(loopEndTime)+"    - Going to sleep now");}
  esp_deep_sleep_start();
}

void go_to_sleep() {
  //Timer is a wake up source. We set our ESP32 to wake up periodically
  //Firts let's set the timer based on the Saving Energy Mode
  //and also the period variable to take actions next wakeup period
  loopEndTime=loopStartTime+millis();

  if (debugModeOn) {Serial.println(String(loopEndTime)+"  - [go_to_sleep] - loopStartTime="+String(loopStartTime));}
  
  switch (energyCurrentMode) {
    case fullEnergy:
      sleepTimer=TIME_TO_SLEEP_FULL_ENERGY>(loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000?TIME_TO_SLEEP_FULL_ENERGY-(loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000:TIME_TO_SLEEP_FULL_ENERGY;
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD;
      samplePeriod=SAMPLE_PERIOD;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD;
      wifiReconnectPeriod=WIFI_RECONNECT_PERIOD;
      BLEPeriod=BLE_PERIOD;
      BLEOnTimeout=BLE_ON_TIMEOUT;
      if (BLEPeriod<=BLEOnTimeout) BLEPeriod=BLEOnTimeout+500;
    break;
    case reducedEnergy:
      sleepTimer=TIME_TO_SLEEP_REDUCED_ENERGY>((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000)?TIME_TO_SLEEP_REDUCED_ENERGY-((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000):TIME_TO_SLEEP_REDUCED_ENERGY;
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_RE; //It's supposed display is off, so no checks on display
      samplePeriod=SAMPLE_PERIOD_RE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_RE;
      wifiReconnectPeriod=WIFI_RECONNECT_PERIOD_RE;
      BLEPeriod=BLE_PERIOD_RE;
      BLEOnTimeout=BLE_ON_TIMEOUT_RE;
      if (BLEPeriod<=BLEOnTimeout) BLEPeriod=BLEOnTimeout+500;
    break;
    case lowestEnergy:
      sleepTimer=TIME_TO_SLEEP_SAVE_ENERGY>((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000)?TIME_TO_SLEEP_SAVE_ENERGY-((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000):TIME_TO_SLEEP_SAVE_ENERGY;
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_SE; //It's supposed display is off, so no checks on display
      samplePeriod=SAMPLE_PERIOD_SE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_SE;
      wifiReconnectPeriod=WIFI_RECONNECT_PERIOD_SE;
      BLEPeriod=BLE_PERIOD_SE;
      BLEOnTimeout=BLE_ON_TIMEOUT_SE;
      if (BLEPeriod<=BLEOnTimeout) BLEPeriod=BLEOnTimeout+500;
    break;
  }
  
  //Going to sleep
  esp_sleep_enable_timer_wakeup(sleepTimer);
  if (debugModeOn) {
    Serial.print(String(loopEndTime)+"  - [go_to_sleep] - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");
    Serial.println("    - Setup ESP32 to wake up in "+String(sleepTimer/uS_TO_S_FACTOR)+" Seconds");
    }
  
  //We set our ESP32 to wake up for an external ext0 trigger.
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0); //1 = High, 0 = Low
  if (debugModeOn) {Serial.println("    - Setup ESP32 to wake up when ext0 GPIO "+String(GPIO_NUM_35)+" is LOW");}
  
  //Make sure BLE is deinitialized first
  if (bluetoothEnabled && BLEDevice::getInitialized()) { //BLE initiated with no BLE clients connected
    BLEDevice::stopAdvertising();BLEDevice::deinit(false); delay(750); //Stop Advertisings, release memory and give time to execute everything
  }

  //Close Network services, WiFi connection and go to sleep
  SPIFFS.end();
  webServer.end();
  if (mqttServerEnabled) mqttClient.disconnect(true);
  WiFi.disconnect(true,false);
  esp_deep_sleep_start();
}

String roundFloattoString(float_t number, uint8_t decimals) {
  //Round float to "decimals" decimals in String format
  String myString;  

  int ent,dec,auxEnt,auxDec,aux1,aux2;

  if (decimals==1) {
    //Better precision operating without pow()
    aux1=number*100;
    ent=aux1/100;
    aux2=ent*100;
    dec=aux1-aux2; if (dec<0) dec=-dec;
  }
  else 
    if (decimals==2) {
      //Better precision operating without pow()
      aux1=number*1000;
      ent=aux1/1000;
      aux2=ent*1000;
      dec=aux1-aux2; if (dec<0) dec=-dec;
    }
    else {
      ent=int(number);
      dec=abs(number*pow(10,decimals+1)-ent*pow(10,decimals+1));
    }
  auxEnt=int(float(dec/10));
  if (auxEnt>=10) auxEnt=9; //Need adjustment for wrong rounds in xx.98 or xx.99
  auxDec=abs(auxEnt*10-dec);
  if (auxDec>=5) auxEnt++;
  if (auxEnt>=10) {auxEnt=0; ent++;}

  if (decimals==0) myString=String(number).toInt(); 
  else myString=String(ent)+"."+String(auxEnt);

  return myString;
}

uint16_t checkSum(byte *addr, uint32_t count) {
  //=  Compute Internet Checksum for count bytes beginning at location addr
  // https://cse.usf.edu/~kchriste/tools/checksum.c

  register uint32_t sum = 0;
  byte auxBuff[count];
  memcpy(auxBuff,addr,count);

  // Main summing loop
  while(count > 1)
  {
    //sum = sum + *((uint16_t *) addr)++;
    sum=sum+auxBuff[count-1];
    count = count - 2;
  }

  // Add left-over byte, if any
  if (count > 0)
    sum = sum + *((byte *) addr);

  // Fold 32-bit sum to 16 bits
  while (sum>>16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return(~sum);
}

void factoryConfReset() {
  uint8_t configVariables;

  wifiEnabled=WIFI_ENABLED;
  bluetoothEnabled=BLE_ENABLED;
  uploadSamplesEnabled=UPLOAD_SAMPLES_ENABLED;
  configSavingEnergyMode=reducedEnergy; //Default value
  webServerEnabled=WEBSERVER_ENABLED;
  mqttServerEnabled=MQTTSERVER_ENABLED;
  secureMqttEnabled=SECURE_MQTT_ENABLED;
  
  //Now initialize configVariables
  configVariables=0x01; //Bit 0, notFirstRun=true
  if (reducedEnergy==configSavingEnergyMode) configVariables|=0x02; //Bit 1: configSavingEnergyMode
  if (uploadSamplesEnabled) configVariables|=0x04; //Bit 2: uploadSamplesEnabled
  if (bluetoothEnabled) configVariables|=0x08; //Bit 3: bluetoothEnabled
  if (wifiEnabled) configVariables|=0x10; //Bit 4: wifiEnabled
  if (webServerEnabled) configVariables|=0x20; //Bit 5: webServerEnabled
  if (mqttServerEnabled) configVariables|=0x40; //Bit 6: mqttServerEnabled
  if (secureMqttEnabled) configVariables|=0x80; //Bit 7: secureMqttEnabled

  //Write variables in EEPROM to be available the next boots up
  EEPROM.write(0x08,configVariables);
  
  //Write WiFi Credential-related variables
  char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
        auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
  //Set variables for SSID or null if no config in global_setup.h file
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
  #ifdef WIFI_SSID_CREDENTIALS
    String(WIFI_SSID_CREDENTIALS).toCharArray(auxSSID,String(WIFI_SSID_CREDENTIALS).length()+1);
  #endif
  #ifdef WIFI_PW_CREDENTIALS
    String(WIFI_PW_CREDENTIALS).toCharArray(auxPSSW,String(WIFI_PW_CREDENTIALS).length()+1);
  #endif
  #ifdef WIFI_SITE
    String(WIFI_SITE).toCharArray(auxSITE,String(WIFI_SITE).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x0D,auxSSID);wifiCred.wifiSSIDs[0]=auxSSID;
  EEPROM.put(0x2E,auxPSSW);wifiCred.wifiPSSWs[0]=auxPSSW;
  EEPROM.put(0x6E,auxSITE);wifiCred.wifiSITEs[0]=auxSITE;
  
  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxSSID='"+String(auxSSID)+"', auxPSSW='"+String(auxPSSW)+"', auxSITE='"+String(auxSITE)+"'");}
  
  //Set variables for SSID_BK1 or null if no config in global_setup.h file
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
  #ifdef WIFI_SSID_CREDENTIALS_BK1
    String(WIFI_SSID_CREDENTIALS_BK1).toCharArray(auxSSID,String(WIFI_SSID_CREDENTIALS_BK1).length()+1);
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK1
    String(WIFI_PW_CREDENTIALS_BK1).toCharArray(auxPSSW,String(WIFI_PW_CREDENTIALS_BK1).length()+1);
  #endif
  #ifdef WIFI_SITE_BK1
    String(WIFI_SITE_BK1).toCharArray(auxSITE,String(WIFI_SITE_BK1).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x79,auxSSID);wifiCred.wifiSSIDs[1]=auxSSID;
  EEPROM.put(0x9A,auxPSSW);wifiCred.wifiPSSWs[1]=auxPSSW;
  EEPROM.put(0xDA,auxSITE);wifiCred.wifiSITEs[1]=auxSITE;
  
  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxSSID='"+String(auxSSID)+"', auxPSSW='"+String(auxPSSW)+"', auxSITE='"+String(auxSITE)+"'");}
  
  //Set variables for SSID_BK2 or null if no config in global_setup.h file
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
  #ifdef WIFI_SSID_CREDENTIALS_BK2
    String(WIFI_SSID_CREDENTIALS_BK2).toCharArray(auxSSID,String(WIFI_SSID_CREDENTIALS_BK2).length()+1);
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK2
    String(WIFI_PW_CREDENTIALS_BK2).toCharArray(auxPSSW,String(WIFI_PW_CREDENTIALS_BK2).length()+1);
  #endif
  #ifdef WIFI_SITE_BK2
    String(WIFI_SITE_BK2).toCharArray(auxSITE,String(WIFI_SITE_BK2).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0xE5,auxSSID);wifiCred.wifiSSIDs[2]=auxSSID;
  EEPROM.put(0x106,auxPSSW);wifiCred.wifiPSSWs[2]=auxPSSW;
  EEPROM.put(0x146,auxSITE);wifiCred.wifiSITEs[2]=auxSITE;

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxSSID='"+String(auxSSID)+"', auxPSSW='"+String(auxPSSW)+"', auxSITE='"+String(auxSITE)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER
    String(NTP_SERVER).toCharArray(auxNTP,String(NTP_SERVER).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x151,auxNTP);ntpServers[0]=auxNTP;

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER2
    String(NTP_SERVER2).toCharArray(auxNTP,String(NTP_SERVER2).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x191,auxNTP);ntpServers[1]=auxNTP;

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER3
    String(NTP_SERVER3).toCharArray(auxNTP,String(NTP_SERVER3).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x1D1,auxNTP);ntpServers[2]=auxNTP;

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER4
    String(NTP_SERVER4).toCharArray(auxNTP,String(NTP_SERVER4).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x211,auxNTP);ntpServers[3]=auxNTP;

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
  memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
  #ifdef NTP_TZ_ENV_VARIABLE
    String(NTP_TZ_ENV_VARIABLE).toCharArray(auxTZEnvVar,String(NTP_TZ_ENV_VARIABLE).length()+1);
    String(NTP_TZ_NAME).toCharArray(auxTZName,String(NTP_TZ_NAME).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x251,auxTZEnvVar);TZEnvVariable=String(auxTZEnvVar);
  EEPROM.put(0x28A,auxTZName);TZName=String(auxTZName);

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxTZEnvVar='"+String(auxTZEnvVar)+"', auxTZName='"+String(auxTZName)+"'");}

  //Write Web User Credential-related variables
  char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
  //Set variables for Web User Credential or null if no config in global_setup.h file
  memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);
  memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);
  #ifdef WEB_USER_CREDENTIAL
    String(WEB_USER_CREDENTIAL).toCharArray(auxUserName,String(WEB_USER_CREDENTIAL).length()+1);
  #endif
  #ifdef WEB_PW_CREDENTIAL
    String(WEB_PW_CREDENTIAL).toCharArray(auxUserPssw,String(WEB_PW_CREDENTIAL).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x2A8,auxUserName);userName=auxUserName;
  EEPROM.put(0x2B3,auxUserPssw);userPssw=auxUserPssw;
  
  if (debugModeOn) {Serial.println(" [initVariable] - Wrote auxUserName='"+String(auxUserName)+"', auxUserPssw='"+String(auxUserPssw)+"'");}
  
  //Now initialize wifiCred.SiteAllow variables
  configVariables=0x0; //Bit 0, notFirstRun=true
  EEPROM.write(0x2BE,configVariables); //All unset the first time EEPROM is written
  wifiCred.SiteAllow[0]=0;wifiCred.SiteAllow[1]=0;wifiCred.SiteAllow[2]=0;

  //Write MQTT server variable
  //Set variables for MQTT Server or null if no config in global_setup.h file
  char auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH];
  memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);
  #ifdef MQTT_SERVER
    String(MQTT_SERVER).toCharArray(auxMQTT,String(MQTT_SERVER).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x2BF,auxMQTT);mqttServer=auxMQTT;
  
  //Write MQTT User Credential-related variables
  //Set variables for MQTT User Credentials or null if no config in global_setup.h file
  memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);
  memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);
  #ifdef MQTT_USER_CREDENTIAL
    String(MQTT_USER_CREDENTIAL).toCharArray(auxUserName,String(MQTT_USER_CREDENTIAL).length()+1);
  #endif
  #ifdef MQTT_PW_CREDENTIAL
    String(MQTT_PW_CREDENTIAL).toCharArray(auxUserPssw,String(MQTT_PW_CREDENTIAL).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x2FF,auxUserName);mqttUserName=auxUserName;
  EEPROM.put(0x30A,auxUserPssw);mqttUserPssw=auxUserPssw;

  //Write MQTT Topic Name variable
  //Set variables for MQTT Topic or null if no config in global_setup.h file
  char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
  memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
  #ifdef MQTT_TOPIC_PREFIX
    mqttTopicPrefix=MQTT_TOPIC_PREFIX;
    if (mqttTopicPrefix.charAt(mqttTopicPrefix.length()-1)!='/') mqttTopicPrefix+="/"; //Adding slash at the end if needed
    uint8_t auxLength=mqttTopicPrefix.length()+1;
    if (auxLength>MQTT_TOPIC_NAME_MAX_LENGTH-1) { //Substring if greater that max length
      auxLength=MQTT_TOPIC_NAME_MAX_LENGTH-1;
      mqttTopicPrefix=mqttTopicPrefix.substring(0,auxLength);
    }
    mqttTopicPrefix.toCharArray(auxMqttTopicPrefix,mqttTopicPrefix.length()+1);
  #endif
  mqttTopicName=mqttTopicPrefix+device; //Adding the device name to the MQTT Topic name
   //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x315,auxMqttTopicPrefix);

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote mqttServer='"+String(auxMQTT)+"', mqttTopicPrefix='"+String(auxMqttTopicPrefix)+"', mqttUserName='"+String(auxUserName)+"', mqttUserPssw='"+String(auxUserPssw)+"'");}

  //Set iBeacon Proximity
  char auxBLEProximityUUID[BLE_BEACON_UUID_LENGH];
  memset(auxBLEProximityUUID,'\0',BLE_BEACON_UUID_LENGH);
  #ifdef BLE_BEACON_UUID
    String(BLE_BEACON_UUID).toCharArray(auxBLEProximityUUID,String(BLE_BEACON_UUID).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x3E0,auxBLEProximityUUID);

  if (debugModeOn) {Serial.println(" [initVariable] - Wrote BLEProximityUUID='"+String(auxBLEProximityUUID)+"'");}

  //Set iBeacon Major and Minor
  byte mac[6];WiFi.macAddress(mac);
  uint16_t aux=(mac[2]<<8)|mac[3];
  EEPROM.writeUShort(0x417,aux);
  if (debugModeOn) {Serial.print(" [initVariable] - Wrote Major=0x"+String(aux,HEX));}
  aux=(mac[4]<<8)|mac[5];
  EEPROM.writeUShort(0x419,aux);

  if (debugModeOn) {Serial.println(" and Minor=0x"+String(aux,HEX));}

  //Initialize bootCount variable
  bootCount=0;
  EEPROM.write(0x3DE,bootCount); 

  //Initialize resetCount variable
  resetCount=0;
  EEPROM.write(0x3DF,resetCount); 
}

bool initTZVariables() {
  //Init TZVariables - Called from firstInit() and aftwer waking up from sleep (button or USB plugged)
  char auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
  bool updateEEPROM=false;

  memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);EEPROM.get(0x251,auxTZEnvVar);
  memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);EEPROM.get(0x28A,auxTZName);
  if (String(auxTZEnvVar).compareTo("")==0) {
    //Take the value from global_setup.h
    #ifdef NTP_TZ_ENV_VARIABLE
      TZEnvVariable=String(NTP_TZ_ENV_VARIABLE);
      //Check if TZEnvVariable must be updated in EEPROM
      if (TZEnvVariable.compareTo(String(auxTZEnvVar))!=0) {
        uint8_t auxLength=TZEnvVariable.length()+1;
        if (auxLength>TZ_ENV_VARIABLE_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=TZ_ENV_VARIABLE_MAX_LENGTH-1;
          TZEnvVariable=TZEnvVariable.substring(0,auxLength);
        }
        memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
        memcpy(auxTZEnvVar,TZEnvVariable.c_str(),auxLength);
        EEPROM.put(0x251,auxTZEnvVar);
        updateEEPROM=true;
      }

      TZName=String(NTP_TZ_NAME);
      //Check if TZName must be updated in EEPROM
      if (TZName.compareTo(String(auxTZName))!=0) {
        uint8_t auxLength=TZName.length()+1;
        if (auxLength>TZ_ENV_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=TZ_ENV_NAME_MAX_LENGTH-1;
          TZName=TZName.substring(0,auxLength);
        }
        memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
        memcpy(auxTZName,TZName.c_str(),auxLength);
        EEPROM.put(0x28A,auxTZEnvVar);
        updateEEPROM=true;
      }
    #else
      //TZEnvVariable=String(auxTZEnvVar);
      TZEnvVariable=String("CET-1CEST,M3.5.0,M10.5.0/3");
      TZName=String("Europe/Madrid");
    #endif
  }
  else {
    TZEnvVariable=String(auxTZEnvVar);
    TZName=String(auxTZName);
  }

  return updateEEPROM;
}

IPAddress stringToIPAddress(String stringIPAddress) {
  char charToTest;
  uint lastBegin=0,indexArray=0;
  int IPAddressOctectArray[4];
  for (uint i=0; i<=stringIPAddress.length(); i++) {
    charToTest=stringIPAddress.charAt(i);
    if (charToTest=='.') {    
      IPAddressOctectArray[indexArray]=stringIPAddress.substring(lastBegin,i).toInt();
      lastBegin=i+1;
      if (indexArray==2) {
        indexArray++;
        IPAddressOctectArray[indexArray]=stringIPAddress.substring(lastBegin,stringIPAddress.length()).toInt();
      }
      else indexArray++;
    }
  }
  
  return IPAddress(IPAddressOctectArray[0],IPAddressOctectArray[1],IPAddressOctectArray[2],IPAddressOctectArray[3]);
}

String getFileExt(const String& s) {

   size_t i = s.lastIndexOf('.');
   if (i != -1) {
      return(s.substring(i));
   }

   return("");
}

size_t getAppOTAPartitionSize(uint8_t type, uint8_t subtype) {
  //Getting the partition size available for OTA. Should be the maximum binary file size to upload via OTA.
  esp_partition_iterator_t iter;
  const esp_partition_t *partition=nullptr;

  switch (type) {
    case ESP_PARTITION_TYPE_APP:
    { const esp_partition_t* nonRunningPartition=nullptr;
      const esp_partition_t* runningPartition=esp_ota_get_running_partition();
      OTAUpgradeBinAllowed=false; //v1.2.0 To block OTA upgrade if there is only one partition
      if (runningPartition==nullptr) {
        //Something wetn wrong
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Couldn't get the running partition");}
        return 0;
      }
      
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Got the running partition");}
      
      //Getting all the APP-type partitions
      iter = esp_partition_find((esp_partition_type_t) type, (esp_partition_subtype_t) subtype, NULL);
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Name, type, subtype, offset, length");}
      uint8_t appPartitionNumber=0;
      while (iter != nullptr)
      {
        partition = esp_partition_get(iter);
        //if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B)");}
        iter = esp_partition_next(iter);
        appPartitionNumber++;
      }
      esp_partition_iterator_release(iter);

      if (appPartitionNumber!=2) {
        //Wrong number of app partitions by design of this firmware
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Wrong number of APP partitions. It should be 2 rather than "+String(appPartitionNumber));}
        //OTA upgrade is not allowed
        return 0;
      }

      //Getting the non-running APP-type partition
      iter = esp_partition_find((esp_partition_type_t) type, (esp_partition_subtype_t) subtype, NULL);
      while (iter != nullptr)
      {
        partition = esp_partition_get(iter);
        if (runningPartition!=partition) {
          nonRunningPartition=partition;
          if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B) is the non-running partition");}
          break;
        }
        else {
          if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B) is the running partition");}
        }
        iter = esp_partition_next(iter);
      }
      esp_partition_iterator_release(iter);

      if (nonRunningPartition==nullptr) {
        //Something wetn wrong getting the non-running partion
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Couldn't get the non-running partition");}
        return 0;
      }

      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Got the non-running partition and size "+String(nonRunningPartition->size)+" B");}
      OTAUpgradeBinAllowed=true;  //v1.2.0 To block OTA upgrade if there is only one partition
      return nonRunningPartition->size;
    }  
    break;
    case ESP_PARTITION_TYPE_DATA:
    { SPIFFSUpgradeBinAllowed=false;  //v1.2.0 To block SPIFFS upgrade if there is something wrong with SPIFFS partition
      if (subtype!=0x82) return 0; //Not SPIFFS partition

      iter = esp_partition_find((esp_partition_type_t) type, (esp_partition_subtype_t) subtype, NULL);
      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Name, type, subtype, offset, length");}
      while (iter != nullptr) //Assuming there is only one SPIFFS partition (Getting the first one)
      {
        partition = esp_partition_get(iter);
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B) is the SPIFFS partition");}
        break;  
        iter = esp_partition_next(iter);
      }
      esp_partition_iterator_release(iter);

      if (partition==nullptr) {
        //Something wetn wrong getting the SPIFFS partion
        if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Couldn't get the SPIFFS partition");}
        return 0;
      }

      if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Got the SPIFFS partition and size "+String(partition->size)+" B");}
      SPIFFSUpgradeBinAllowed=true; //v1.2.0 To block SPIFFS upgrade if there is something wrong with SPIFFS partition
      return partition->size;
    }
    break;
    default:
      //It's supposed never get here
      return 0;
    break;
  }
  
  
  
  //Interesting code to get the DATA partition.
  /*iter = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (iter != nullptr)
  {
    const esp_partition_t *partition = esp_partition_get(iter);
    if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - "+String(partition->label)+" data "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B)");}
    iter = esp_partition_next(iter);
  }
  esp_partition_iterator_release(iter);
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Info on running partition");}
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - "+String(partition->label)+" data "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B)");}
  if (debugModeOn) {Serial.println(String(nowTimeGlobal)+"   [getPartitionSize] - Ending partition conde");}
  */
}

void logRamStats (const char text[]) {
  Serial.printf("\n----==========-----%s\n",text);
  heap_caps_print_heap_info(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  Serial.printf("----==========-----\n");
}

void detachNetwork(void) {
  //Reinit modules to release memory in the hope to increase heap size
  //Close WiFi connection and go to sleep
  webServer.removeHandler(&webEvents);
  webServer.end();
  SPIFFS.end();
  mqttClient.disconnect(true);
  WiFi.disconnect(true,false);
  wifiCurrentStatus=wifiOffStatus;
  CloudSyncCurrentStatus=CloudSyncOffStatus;
  CloudClockCurrentStatus=CloudClockOffStatus;
  MqttSyncCurrentStatus=MqttSyncOffStatus;
  forceWifiReconnect=true; //Force to reconnect WiFi in the next loop cycle
  forceWebServerInit=true; //Force to reinit the webServer
  //CloudSyncCurrentStatus (External HTTP server) and  CloudClockCurrentStatus (NTP Server) will be init by their own
  
  if (debugModeOn) Serial.println(String(nowTimeGlobal)+"   [detachNetwork] - WiFi related modules stoped. They will be initiated in the next loop cycle");
}

String reverseString(String inputString) {
  //Reverse the string.
  char buff[inputString.length()+1];
  memset(buff,'\0',sizeof(buff));
  const char* ptr=inputString.c_str();

  ptr+=inputString.length()-1;
  for (int i=0; i<=inputString.length()-1; i++) {
    buff[i]=(byte) *ptr;
    ptr--;
  }
  
  return String(buff);
}


String reverseStringUUID(String inputString) {
  //Reverse the string. Useful for BLE iBeacon Proximity
  //  inputString must be in the format:  F7826DA6-4FA2-4E98-8024-BC5B71E0893E  - No checks are done
  //  output will be in the format:       3E89E071-5BBC-2480-984E-A24FA66D82F7

  const char* ptr=inputString.c_str();  //F7826DA6-4FA2-4E98-8024-BC5B71E0893E
  char buff[inputString.length()+1];
  memset(buff,'\0',sizeof(buff));
  memcpy(buff,ptr,inputString.length());

  //Clean all the hyphens
  char buff2[sizeof(buff)];
  memset(buff2,'\0',sizeof(buff2));
  int buff2Index=0;
  for (int i=0; i<sizeof(buff2);i++) if (buff[i]!='-') {buff2[buff2Index]=buff[i];buff2Index++;
  }  //buff2=F7826DA64FA24E988024BC5B71E0893E

  memset(buff,'\0',sizeof(buff));
  uint8_t auxLength2=String(buff2).length();
  ptr=buff2+auxLength2-2;
  for (int i=0; i<=auxLength2-1; i=i+2) {memcpy(&buff[i],ptr,2);ptr=ptr-2;} //buff=3E89E0715BBC2480984EA24FA66D82F7

  memset(buff2,'\0',sizeof(buff2));
  memcpy(&buff2[0],&buff[0],8);buff2[8]='-';       //3E89E071-
  memcpy(&buff2[9],&buff[8],4);buff2[13]='-';      //3E89E071-5BBC-
  memcpy(&buff2[14],&buff[12],4);buff2[18]='-';    //3E89E071-5BBC-2480-
  memcpy(&buff2[19],&buff[16],4);buff2[23]='-';    //3E89E071-5BBC-2480-984E-
  memcpy(&buff2[24],&buff[20],12);                //3E89E071-5BBC-2480-984E-A24FA66D82F7
  
  return String(buff2);
}