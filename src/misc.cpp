#include "misc.h"
#include <EEPROM.h>

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
    break;
    case reducedEnergy:
      sleepTimer=TIME_TO_SLEEP_REDUCED_ENERGY>((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000)?TIME_TO_SLEEP_REDUCED_ENERGY-((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000):TIME_TO_SLEEP_REDUCED_ENERGY;
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_RE; //It's supposed display is off, so no checks on display
      samplePeriod=SAMPLE_PERIOD_RE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_RE;
    break;
    case lowestEnergy:
      sleepTimer=TIME_TO_SLEEP_SAVE_ENERGY>((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000)?TIME_TO_SLEEP_SAVE_ENERGY-((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000):TIME_TO_SLEEP_SAVE_ENERGY;
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_SE; //It's supposed display is off, so no checks on display
      samplePeriod=SAMPLE_PERIOD_SE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_SE;
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
  
  //Close WiFi connection and go to sleep
  webServer.end();
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
  
  //Now initialize configVariables
  configVariables=0x01; //Bit 0, notFirstRun=true
  if (reducedEnergy==configSavingEnergyMode) configVariables|=0x02; //Bit 1: configSavingEnergyMode
  if (uploadSamplesEnabled) configVariables|=0x04; //Bit 2: uploadSamplesEnabled
  if (bluetoothEnabled) configVariables|=0x08; //Bit 3: bluetoothEnabled
  if (wifiEnabled) configVariables|=0x10; //Bit 4: wifiEnabled
  if (webServerEnabled) configVariables|=0x20; //Bit 5: webServerEnabled

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

  //Now initialize wifiCred.SiteAllow variables
  configVariables=0x0; //Bit 0, notFirstRun=true
  EEPROM.write(0x2A8,configVariables); //All unset the first time EEPROM is written
  wifiCred.SiteAllow[0]=0;wifiCred.SiteAllow[1]=0;wifiCred.SiteAllow[2]=0;
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
      return nonRunningPartition->size;
    }  
    break;
    case ESP_PARTITION_TYPE_DATA:
    { if (subtype!=0x82) return 0; //Not SPIFFS partition

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
      return partition->size;
    }
    break;
    default:
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