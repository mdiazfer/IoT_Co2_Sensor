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
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_RE; //Keeping it for future. In this verstion No BAT checks in Reduce Engergy Mode to save energy
      samplePeriod=SAMPLE_PERIOD_RE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_RE;
    break;
    case lowestEnergy:
      sleepTimer=TIME_TO_SLEEP_SAVE_ENERGY>((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000)?TIME_TO_SLEEP_SAVE_ENERGY-((loopEndTime-loopStartTime+INITIAL_BOOTIME)*1000):TIME_TO_SLEEP_SAVE_ENERGY;
      voltageCheckPeriod=VOLTAGE_CHECK_PERIOD_SE; //Keeping in for future. In this version No BAT checks in Save Engergy Mode to save energy
      samplePeriod=SAMPLE_PERIOD_SE;
      uploadSamplesPeriod=UPLOAD_SAMPLES_PERIOD_SE;
    break;
  }
  
  //Going to sleep
  esp_sleep_enable_timer_wakeup(sleepTimer);
  if (debugModeOn) {
    Serial.print(String(loopEndTime)+"  - [go_to_sleep] - Time: ");getLocalTime(&startTimeInfo);Serial.println(&startTimeInfo, "%d/%m/%Y - %H:%M:%S");
    Serial.println("    - Setup ESP32 to wake up in "+String(sleepTimer/uS_TO_S_FACTOR)+" Seconds");
    }
  
  //We set our ESP32 to wake up for an external ext0 trigger.
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0); //1 = High, 0 = Low
  if (debugModeOn) {Serial.println("    - Setup ESP32 to wake up when ext0 GPIO "+String(GPIO_NUM_35)+" is LOW");}
  
  //Close WiFi connection and go to sleep
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