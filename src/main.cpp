#ifndef WIFIFRAMEWORK
  #define WIFIFRAMEWORK
#endif

#ifndef _DECLAREGLOBALPARAMETERS_
  #define _DECLAREGLOBALPARAMETERS_
#endif

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "global_setup.h"
#include "wifiConnection.h"
#include "display_support.h"
#include "MHZ19.h"
#include <SoftwareSerial.h>
#include "Button.h"
#include "SHT2x.h"
#include "ButtonChecks.h"
#include "time.h"
#include "httpClient.h"
#include "icons.h"
#include "battery.h"

#ifdef __MHZ19B__
  const ulong co2PreheatingTime=MH_Z19B_CO2_WARMING_TIME;
#endif

//Global variable definitions
const String co2SensorType=String(CO2_SENSOR_TYPE);
const String tempHumSensorType=String(TEMP_HUM_SENSOR_TYPE);
char co2SensorVersion[5];
uint8_t error_setup=NO_ERROR,remainingBootupSeconds=0;
int16_t cuX,cuY;
TFT_eSPI tft = TFT_eSPI();  // Invoke library to manage the display
TFT_eSprite stext1 = TFT_eSprite(&tft); // Sprite object stext1
ulong nowTime=0,previousTime=0,previousTimeDisplay=0,previousTimeDisplayMode=0,previousTimeNTPCheck=0,previousTimeVOLTCheck=0,previousTimeBATCheck=0,
      previousHourSampleTime=0,previousDaySampleTime=0,previousUploadSampleTime=0,previousTimeIconStatusRefresh=0,
      gapTime,lastGapTime,gapTimeDisplay,gapTimeDisplayMode,gapHourSampleTime,gapDaySampleTime,gapTimeNTPCheck,gapTimeVOLTCheck,gapTimeBATCheck,
      gapUploadSampleTime=0,gapTimeIconStatusRefresh=0,previousTurnOffBacklightTime=0,gapTurnOffBacklight=0,
      timeUSBPower=0,timePressButton2,timeReleaseButton2,remainingBootupTime=BOOTUP_TIMEOUT*1000;
CircularGauge circularGauge=CircularGauge(0,0,CO2_GAUGE_RANGE,CO2_GAUGE_X,CO2_GAUGE_Y,CO2_GAUGE_R,
                                          CO2_GAUGE_WIDTH,CO2_GAUGE_SECTOR,TFT_DARKGREEN,
                                          CO2_GAUGE_TH1,TFT_YELLOW,CO2_GAUGE_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);
HorizontalBar horizontalBar=HorizontalBar(0,TEMP_BAR_MIN,TEMP_BAR_MAX,TEMP_BAR_X,TEMP_BAR_Y,
                                          TEMP_BAR_LENGTH,TEMP_BAR_HEIGH,TFT_GREEN,TEMP_BAR_TH1,
                                          TFT_BLUE,TEMP_BAR_TH2,TFT_RED,TFT_DARKGREY,TFT_BLACK);
float_t valueCO2,valueT,valueHum=0,lastValueCO2=-1,tempMeasure;
String valueString;
float_t lastHourCo2Samples[3600/SAMPLE_T_LAST_HOUR];   //Buffer to record last-hour C02 values
float_t lastHourTempSamples[3600/SAMPLE_T_LAST_HOUR];  //Buffer to record last-hour Temp values
float_t lastHourHumSamples[3600/SAMPLE_T_LAST_HOUR];   //Buffer to record last-hour Hum values
float_t lastDayCo2Samples[24*3600/SAMPLE_T_LAST_DAY];  //Buffer to record last-day C02 values
float_t lastDayTempSamples[24*3600/SAMPLE_T_LAST_DAY];  //Buffer to record last-day Temp values
float_t lastDayHumSamples[24*3600/SAMPLE_T_LAST_DAY];   //Buffer to record last-day Hum values
boolean showGraph=false,updateHourSample=true,updateDaySample=true,updateHourGraph=true,updateDayGraph=true;
int8_t counterDisplay=-1;
enum displayModes displayMode=bootup, lastDisplayMode=bootup; 
enum availableStates stateSelected=displayingSampleFixed,currentState=bootupScreen,lastState=currentState;
enum CloudClockStatus CloudClockCurrentStatus;
SoftwareSerial co2SensorSerialPort(CO2_SENSOR_RX, CO2_SENSOR_TX);
#ifdef __MHZ19B__
  MHZ19 co2Sensor;
  extern const int MHZ19B;
#endif
#ifdef __SI7021__
  SHT2x tempHumSensor;
#endif
Button  button1(BUTTON1);
Button  button2(BUTTON2);
const long  gmtOffset_sec = GMT_OFFSET_SEC;
const int   daylightOffset_sec = DAYLIGHT_OFFSET_SEC;
struct tm startTimeInfo;
String serverToUploadSamplesString(SERVER_UPLOAD_SAMPLES);
IPAddress serverToUploadSamplesIPAddress;
boolean uploadSamplesToServer=UPLOAD_SAMPLES_TO_SERVER;
String device(DEVICE_NAME);
static const char hex_digits[] = "0123456789ABCDEF";
boolean waitingMessage=true,runningMessage=true;
boolean autoBackLightOff=true;
boolean button2Pressed=false;
uint8_t pixelsPerLine,
    spL,            //Number of Lines in the Sprite
    scL,            //Number of Lines in the Scroll
    pFL,            //Pointer First Line
    pLL,            //pointer Last Line written
    spFL,           //Sprite First Line Window
    spLL,           //Sprite Last Line Window
    scFL,           //Scroll First Line Window
    scLL;           //Scroll Last Line Window
enum powerModes powerState=off,lastPowerState=off;
enum batteryChargingStatus batteryStatus=battery000;

//Code
void loadBootImage() {
  //-->>Load the logo image when booting up
  
  return;
}

void showIcons() {
  //Load the icons

  tft.setSwapBytes(true);
  
  //Drawign wifi icon
  switch (wifiCurrentStatus) {
    case (wifi100Status):
      tft.pushImage(0,0,24,24,wifi100);
    break;
    case (wifi75Status):
      tft.pushImage(0,0,24,24,wifi075);
    break;
    case (wifi50Status):
      tft.pushImage(0,0,24,24,wifi050);
    break;
    case (wifi25Status):
      tft.pushImage(0,0,24,24,wifi025);
    break;
    case (wifi0Status):
      tft.pushImage(0,0,24,24,wifi000);
    break;
    case (wifiOffStatus):
      tft.pushImage(0,0,24,24,wifiOff);
    break;
  }
  
  //-->>Get BLE status
  tft.pushImage(30,0,24,24,bluetoothOff);
  
  //-->>Get NTP status
  switch (CloudClockCurrentStatus) {
    case (CloudClockOnStatus):
      tft.pushImage(95,0,24,24,cloudClockOn);
    break;
    case (CloudClockOffStatus):
      tft.pushImage(95,0,24,24,cloudClockOff);
    break;
  }
  
  //-->>Get Cloud status
  switch (CloudSyncCurrentStatus) {
    case (CloudSyncOnStatus):
      tft.pushImage(125,0,24,24,cloudSyncOn);
    break;
    case (CloudSyncOffStatus):
      tft.pushImage(125,0,24,24,cloudSyncOff);
    break;
   }
  
  //-->>Get Batery status
  switch (batteryStatus) {
    case (batteryCharging000):
      tft.pushImage(215,0,24,24,StatusBatteryCharging000);
    break;
    case (batteryCharging010):
      tft.pushImage(215,0,24,24,StatusBatteryCharging010);
    break;
    case (batteryCharging025):
      tft.pushImage(215,0,24,24,StatusBatteryCharging025);
    break;
    case (batteryCharging050):
      tft.pushImage(215,0,24,24,StatusBatteryCharging050);
    break;
    case (batteryCharging075):
      tft.pushImage(215,0,24,24,StatusBatteryCharging075);
    break;
    case (batteryCharging100):
      tft.pushImage(215,0,24,24,StatusBatteryCharging100);
    break;
    case (battery000):
      tft.pushImage(215,0,24,24,StatusBattery000);
    break;
    case (battery010):
      tft.pushImage(215,0,24,24,StatusBattery010);
    break;
    case (battery025):
      tft.pushImage(215,0,24,24,StatusBattery025);
    break;
    case (battery050):
      tft.pushImage(215,0,24,24,StatusBattery050);
    break;
    case (battery075):
      tft.pushImage(215,0,24,24,StatusBattery075);
    break;
    case (battery100):
      tft.pushImage(215,0,24,24,StatusBattery100);
    break;
  }
  
}

void loadAllIcons() {
  tft.setSwapBytes(true);

  tft.pushImage(0,0,24,24,StatusBattery000);
  tft.pushImage(30,0,24,24,StatusBattery010);
  tft.pushImage(60,0,24,24,StatusBattery025);
  tft.pushImage(90,0,24,24,StatusBattery050);
  tft.pushImage(120,0,24,24,StatusBattery075);
  tft.pushImage(150,0,24,24,StatusBattery100);
    
  tft.pushImage(0,30,24,24,StatusBatteryCharging000);
  tft.pushImage(30,30,24,24,StatusBatteryCharging010);
  tft.pushImage(60,30,24,24,StatusBatteryCharging025);
  tft.pushImage(90,30,24,24,StatusBatteryCharging050);
  tft.pushImage(120,30,24,24,StatusBatteryCharging075);
  tft.pushImage(150,30,24,24,StatusBatteryCharging100);

  tft.pushImage(0,60,24,24,wifi000);
  tft.pushImage(30,60,24,24,wifi025);
  tft.pushImage(60,60,24,24,wifi050);
  tft.pushImage(90,60,24,24,wifi075);
  tft.pushImage(120,60,24,24,wifi100);
  tft.pushImage(150,60,24,24,wifiOff);

  tft.pushImage(0,90,24,24,bluetooth);
  tft.pushImage(30,90,24,24,bluetoothConnected);
  tft.pushImage(60,90,24,24,bluetoothOff);

  tft.pushImage(180,0,24,24,cloudClockOn);
  tft.pushImage(180,30,24,24,cloudClockOff);

  tft.pushImage(210,0,24,24,cloudSyncOn);
  tft.pushImage(210,30,24,24,cloudSyncOff);
  
  while(true);
}

void loadAllWiFiIcons() {
  tft.setSwapBytes(true);

  /*tft.pushImage(0,0,24,24,wifi000_blue);
  tft.pushImage(30,0,24,24,wifi025_blue);
  tft.pushImage(60,0,24,24,wifi050_blue);
  tft.pushImage(90,0,24,24,wifi075_blue);
  tft.pushImage(120,0,24,24,wifi100_blue);
  tft.pushImage(150,0,24,24,wifiOff_blue);

  tft.pushImage(0,30,24,24,wifi000_blue_bis);
  tft.pushImage(30,30,24,24,wifi025_blue_bis);
  tft.pushImage(60,30,24,24,wifi050_blue_bis);
  tft.pushImage(90,30,24,24,wifi075_blue_bis);
  tft.pushImage(120,30,24,24,wifi100_blue);
  tft.pushImage(150,30,24,24,wifiOff_blue_bis);

  tft.pushImage(0,70,24,24,wifi000_white);
  tft.pushImage(30,70,24,24,wifi025_white);
  tft.pushImage(60,70,24,24,wifi050_white);
  tft.pushImage(90,70,24,24,wifi075_white);
  tft.pushImage(120,70,24,24,wifi100_white);
  tft.pushImage(150,70,24,24,wifiOff_white);

  tft.pushImage(0,100,24,24,wifi000_bis3);
  tft.pushImage(30,100,24,24,wifi025_bis2);
  tft.pushImage(60,100,24,24,wifi050_bis2);
  tft.pushImage(90,100,24,24,wifi075_bis2);
  tft.pushImage(120,100,24,24,wifi100_bis2);
  tft.pushImage(150,100,24,24,wifiOff);
  */
  
  /*tft.pushImage(0,100,24,24,wifi000_white_bis);
  tft.pushImage(30,100,24,24,wifi025_white_bis);
  tft.pushImage(60,100,24,24,wifi050_white_bis);
  tft.pushImage(90,100,24,24,wifi075_white_bis);
  tft.pushImage(120,100,24,24,wifi100_white);
  tft.pushImage(150,100,24,24,wifiOff_white_bis);

  tft.pushImage(0,100,24,24,wifi000_bis);
  tft.pushImage(30,100,24,24,wifi025_bis);
  tft.pushImage(60,100,24,24,wifi050_bis);
  tft.pushImage(90,100,24,24,wifi075_bis);
  tft.pushImage(120,100,24,24,wifi100);
  tft.pushImage(150,100,24,24,wifiOff);*/
  
  while(true);
}

void drawGraphLastHourCo2() {
  //Clean-up display
  tft.fillScreen(TFT_BLACK);

  //Draw thresholds, axis & legend
  /*int auxI=0; while (auxI<40) {  
    tft.drawFastHLine(CO2_GRAPH_X+5*auxI,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH1*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),5,TFT_YELLOW);
    tft.drawFastHLine(CO2_GRAPH_X+5*auxI,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH2*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),5,TFT_RED);
    auxI=auxI+2;
  }*/
  tft.fillRect(CO2_GRAPH_X,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH2*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),CO2_GRAPH_WIDTH,8,TFT_NAVY);
  tft.drawFastVLine(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_HEIGH+5,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_X-5,CO2_GRAPH_Y+2,CO2_GRAPH_X+5,CO2_GRAPH_Y+2,TFT_DARKGREY);
  tft.drawFastHLine(CO2_GRAPH_X,CO2_GRAPH_Y+CO2_GRAPH_HEIGH,CO2_GRAPH_WIDTH,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X_END,CO2_GRAPH_Y_END,CO2_GRAPH_X_END-5,CO2_GRAPH_Y_END-3,CO2_GRAPH_X_END-5,CO2_GRAPH_Y_END+3,TFT_DARKGREY);
  for (int i=1;i<=12;i++) if (3*(int)(i/3)==i) tft.drawFastVLine(CO2_GRAPH_X+15*i,CO2_GRAPH_Y_END-5,10,TFT_DARKGREY);
    else tft.drawFastVLine(CO2_GRAPH_X+15*i,CO2_GRAPH_Y_END-2,4,TFT_DARKGREY); 
  for (int i=0;i<=3;i++) tft.drawFastHLine(CO2_GRAPH_X-5,CO2_GRAPH_Y+25*i,10,TFT_DARKGREY);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(CO2_GRAPH_X+35,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-45");
  tft.setCursor(CO2_GRAPH_X+80,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-35");
  tft.setCursor(CO2_GRAPH_X+125,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-15");
  tft.setCursor(CO2_GRAPH_X+170,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("Now");
  tft.setCursor(CO2_GRAPH_X+190,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("t(m)");
  //tft.setCursor(CO2_GRAPH_X+30,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("CO2 in the last 60 min (ppm)");
  tft.setCursor(CO2_GRAPH_X+15,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("Last 60 min   ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREEN,TFT_BLACK);tft.print("CO2 (ppm) [0-2000]");
  tft.setCursor(CO2_GRAPH_X+15,14,TEXT_FONT_BOOT_SCREEN-1);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_CYAN,TFT_BLACK);tft.print("Temp (C) [0-50] ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_MAGENTA,TFT_BLACK);tft.print(" Hum (%) [0-100]");

  //Draw samples
  int32_t co2Sample,tempSample,humSample,auxCo2Color,auxTempColor=TFT_CYAN,auxHumColor=TFT_MAGENTA;
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)
  {
    co2Sample=(int32_t) (CO2_GRAPH_Y_END-lastHourCo2Samples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX);
    tempSample=(int32_t) (CO2_GRAPH_Y_END-lastHourTempSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_TEMP_MAX);
    humSample=(int32_t) (CO2_GRAPH_Y_END-lastHourHumSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_HUM_MAX);
    if (lastHourCo2Samples[i]<=CO2_GAUGE_TH1) auxCo2Color=TFT_GREEN;
    else if (CO2_GAUGE_TH1 < lastHourCo2Samples[i] && lastHourCo2Samples[i] <= CO2_GAUGE_TH2) auxCo2Color=TFT_YELLOW;
    else auxCo2Color=TFT_RED;
    if (co2Sample==CO2_GRAPH_Y_END) auxCo2Color=TFT_DARKGREY;
    if(lastHourTempSamples[i]==0) auxTempColor=TFT_DARKGREY; else auxTempColor=TFT_CYAN;
    if(lastHourHumSamples[i]==0) auxHumColor=TFT_DARKGREY; else auxHumColor=TFT_MAGENTA;
    tft.drawPixel(i+CO2_GRAPH_X,humSample,auxHumColor); //Hum sample
    tft.drawPixel(i+CO2_GRAPH_X,tempSample,auxTempColor); //Temp sample
    tft.drawPixel(i+CO2_GRAPH_X,co2Sample,auxCo2Color); //CO2 sample
  }
}

void drawGraphLastDayCo2() {
  //Clean-up display
  tft.fillScreen(TFT_BLACK);

  //Draw thresholds, axis & legend
  /*int auxI=0; while (auxI<40) {  
    tft.drawFastHLine(CO2_GRAPH_X+5*auxI,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH1*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),5,TFT_YELLOW);
    tft.drawFastHLine(CO2_GRAPH_X+5*auxI,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH2*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),5,TFT_RED);
    auxI=auxI+2;
  }*/
  tft.fillRect(CO2_GRAPH_X,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH2*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),CO2_GRAPH_WIDTH,8,TFT_NAVY);
  tft.drawFastVLine(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_HEIGH+5,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_X-5,CO2_GRAPH_Y+2,CO2_GRAPH_X+5,CO2_GRAPH_Y+2,TFT_DARKGREY);
  tft.drawFastHLine(CO2_GRAPH_X,CO2_GRAPH_Y+CO2_GRAPH_HEIGH,CO2_GRAPH_WIDTH+12,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X_END+12,CO2_GRAPH_Y_END,CO2_GRAPH_X_END+12-5,CO2_GRAPH_Y_END-3,CO2_GRAPH_X_END+12-5,CO2_GRAPH_Y_END+3,TFT_DARKGREY);
  for (int i=1;i<=12;i++) if (3*(int)(i/3)==i) tft.drawFastVLine(CO2_GRAPH_X+16*i,CO2_GRAPH_Y_END-5,10,TFT_DARKGREY);
    else tft.drawFastVLine(CO2_GRAPH_X+16*i,CO2_GRAPH_Y_END-2,4,TFT_DARKGREY);
  for (int i=0;i<=3;i++) tft.drawFastHLine(CO2_GRAPH_X-5,CO2_GRAPH_Y+25*i,10,TFT_DARKGREY);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(CO2_GRAPH_X+38,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-18");
  tft.setCursor(CO2_GRAPH_X+86,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-12");
  tft.setCursor(CO2_GRAPH_X+134,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-6");
  tft.setCursor(CO2_GRAPH_X+165,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("Now t(h)");
  //tft.setCursor(CO2_GRAPH_X+30,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("CO2 in the last 24 h. (ppm)");
  tft.setCursor(CO2_GRAPH_X+15,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("Last 24 h.    ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREEN,TFT_BLACK);tft.print("CO2 (ppm) [0-2000]");
  tft.setCursor(CO2_GRAPH_X+15,14,TEXT_FONT_BOOT_SCREEN-1);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_CYAN,TFT_BLACK);tft.print("Temp (C) [0-50] ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_MAGENTA,TFT_BLACK);tft.print(" Hum (%) [0-100]");
  //Draw samples
  int32_t co2Sample,tempSample,humSample,auxCo2Color,auxTempColor=TFT_CYAN,auxHumColor=TFT_MAGENTA;
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++)
  {
    co2Sample=(int32_t) (CO2_GRAPH_Y_END-lastDayCo2Samples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX);
    tempSample=(int32_t) (CO2_GRAPH_Y_END-lastDayTempSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_TEMP_MAX);
    humSample=(int32_t) (CO2_GRAPH_Y_END-lastDayHumSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_HUM_MAX);
    if (lastDayCo2Samples[i]<=CO2_GAUGE_TH1) auxCo2Color=TFT_GREEN;
    else if (CO2_GAUGE_TH1 < lastDayCo2Samples[i] && lastDayCo2Samples[i] <= CO2_GAUGE_TH2) auxCo2Color=TFT_YELLOW;
    else auxCo2Color=TFT_RED;
    if (co2Sample==CO2_GRAPH_Y_END) auxCo2Color=TFT_DARKGREY;
    if(lastDayTempSamples[i]==0) auxTempColor=TFT_DARKGREY; else auxTempColor=TFT_CYAN;
    if(lastDayHumSamples[i]==0) auxHumColor=TFT_DARKGREY; else auxHumColor=TFT_MAGENTA;
    tft.drawPixel(i+CO2_GRAPH_X,humSample,auxHumColor); //Hum sample
    tft.drawPixel(i+CO2_GRAPH_X,tempSample,auxTempColor); //Temp sample
    tft.drawPixel(i+CO2_GRAPH_X,co2Sample,auxCo2Color);   //CO2 sample
  }
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

void setupNTPConfig(boolean fromSetup) {
  //If function called fron setup(), then logs are displayed

  CloudClockCurrentStatus=CloudClockOffStatus;
  //User credentials definition
  ntpServers[0]="\0";ntpServers[1]="\0";ntpServers[2]="\0";ntpServers[3]="\0";
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

  if (error_setup != ERROR_WIFI_SETUP ) {
    uint8_t previousError_Setup=error_setup;
    for (uint8_t loopCounter=0; loopCounter<(uint8_t)sizeof(ntpServers)/sizeof(String); loopCounter++) {
      error_setup=ERROR_NTP_SERVER;
      if (ntpServers[loopCounter].charAt(0)=='\0') loopCounter++;
      else {
        if (logsOn && fromSetup) {
          Serial.println("[setup - NTP] Connecting to NTP Server: ");
          Serial.print("  NTP Server: ");Serial.println(ntpServers[loopCounter].c_str());
        }
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServers[loopCounter].c_str());

        if (!getLocalTime(&startTimeInfo)) {
          if (logsOn && fromSetup) {
            Serial.println("  Time: Failed to get time");
            Serial.print("[setup] - NTP: ");Serial.println("KO");
          }          
        }
        else {
          if (logsOn && fromSetup) {
            Serial.print("  Time: "); Serial.println(&startTimeInfo,"%d/%m/%Y - %H:%M:%S");
            Serial.print("[setup] - NTP: ");Serial.println("OK");
          }
          CloudClockCurrentStatus=CloudClockOnStatus;
          ntpServerIndex=loopCounter;
          error_setup=previousError_Setup;
          loopCounter=(uint8_t)sizeof(ntpServers)/sizeof(String);
        }
      }
    }
  }
}

void setup() {
  static uint32_t wr = 1;
  static uint32_t rd = 0xFFFFFFFF;
  currentState=bootupScreen;lastState=currentState;
  displayMode=bootup;lastDisplayMode=bootup;

  if (logsOn) {Serial.begin(115200);Serial.print("\nCO2 bootup v");Serial.print(VERSION);Serial.println(" ..........");Serial.println("[setup] - Serial: OK");}

  //Display init
  pinMode(PIN_TFT_BACKLIGHT,OUTPUT); 
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Create a sprite for text and initialize sprite parameters
  stext1.setColorDepth(4);
  stext1.createSprite(TFT_MAX_X, TFT_MAX_Y*SCROLL_PER_SPRITE);
  stext1.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  stext1.setTextColor(TFT_GOLD); // Gold text, no background
  stext1.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  stext1.setTextFont(TEXT_FONT_BOOT_SCREEN);
  pixelsPerLine=tft.fontHeight(TEXT_FONT_BOOT_SCREEN);
  spL=LINES_PER_TEXT_SPRITE;            //Number of Lines in the Sprite
  scL=LINES_PER_TEXT_SCROLL;            //Number of Lines in the Scroll
  spFL=1;                               //Sprite First Line Window
  spLL=spL;                             //Sprite Last Line Window
  scFL=(spL-scL)/2+1;                   //Scroll First Line Window
  scLL=scFL+scL-1;                      //Scroll Last Line Window
  pFL=scFL;                             //Pointer First Line
  pLL=pFL;                              //pointer Last Line written
  
  delay(500);
  //Check out the TFT display
  tft.drawPixel(30,30,wr);
  rd = tft.readPixel(30,30);
  if (rd!=wr) {
    error_setup=ERROR_DISPLAY_SETUP;
    if (logsOn) {
      Serial.println("[setup] - Display: KO");
      Serial.println("          Pixel value wrote = ");Serial.println(wr,HEX);
      Serial.println("          Pixel value read  = ");Serial.println(rd,HEX);
      Serial.println("          Can't continue");
      //-->> Setup LED for error indication
    }
    return;
  }
  else {
    if (logsOn) Serial.println("[setup] - Display: OK");
    tft.fillScreen(TFT_BLACK);
  }
  
  //-->>loadAllWiFiIcons();

  loadBootImage();
  delay(500);
  //Display messages
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_WHITE,TFT_BLACK);stext1.print("CO2 bootup v");stext1.print(VERSION);stext1.println(" ..........");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Display: [");stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  
  //Some Temp & Hum sensor checks and init
  if (logsOn) Serial.print("[setup] - Sensor Temp/HUM: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Tp/Hu:  [");
  pinMode(SI7021_SDA,INPUT_PULLUP); pinMode(SI7021_SCL,INPUT_PULLUP);
  tempHumSensor.begin(SI7021_SDA,SI7021_SCL);

  int errorSns = tempHumSensor.getError();
  uint8_t statSns = tempHumSensor.getStatus();

  if (!tempHumSensor.isConnected() || 0==tempHumSensorType.compareTo("UNKNOW"))
    error_setup=ERROR_SENSOR_TEMP_HUM_SETUP;
  if (error_setup != ERROR_SENSOR_TEMP_HUM_SETUP ) { 
    if (logsOn) {
      Serial.println("OK");
      Serial.print("  Tp/Hm Sensor type: "); Serial.println(tempHumSensorType); 
      //Serial.print("  Tp/Hm Sen. version: "); Serial.println(tempHumSensor.getFirmwareVersion());
      Serial.print("  Tp/Hm Sen. status: "); Serial.println(statSns,HEX);
      Serial.print("  Tp/Hm Sen.  error: "); Serial.println(errorSns,HEX);
      Serial.print("  Tp/Hm Sen. resolu.: "); Serial.println(tempHumSensor.getResolution());
    }
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Sensor: ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(tempHumSensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }
  else {
    if (logsOn) {
      Serial.println("KO");
      Serial.print("  Tp/Hm Sensor type: "); Serial.print(tempHumSensorType);Serial.println(" - Shouldn't be UNKNOWN");
      //Serial.print("  Tp/Hm Sen. version: "); Serial.println(tempHumSensor.getFirmwareVersion());
      Serial.print("  Tp/Hm Sen. status: "); Serial.println(statSns,HEX);
      Serial.print("  Tp/Hm Sen.  error: "); Serial.print(errorSns,HEX);Serial.println(" - Should be 0");
      Serial.print("  Tp/Hm Sen. resolu.: "); Serial.println(tempHumSensor.getResolution());
      Serial.println("  Can't continue. STOP");
    }
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sns. type: ");if (0==tempHumSensorType.compareTo("UNKNOW")) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(tempHumSensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    //tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  Tp/Hm Sen. version: "); tft.println(tempHumSensor.getFirmwareVersion());
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sen. status: ");stext1.print(statSns,HEX);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sen.  error: ");if (0 != errorSns) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print(errorSns,HEX);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Tp/Hm Sen. resolu.: ");stext1.print(tempHumSensor.getResolution());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.print(" ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Can't continue. STOP");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    return;
  }

  //Sensor CO2 init
  if (logsOn) Serial.print("[setup] - Sensor: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Sens.:  [");
  co2SensorSerialPort.begin(9600);      // (Uno example) device to MH-Z19 serial start   
  co2Sensor.begin(co2SensorSerialPort); // *Serial(Stream) refence must be passed to library begin(). 
  co2Sensor.setRange(CO2_SENSOR_CO2_RANGE);             // It's aviced to setup range to 2000. Better accuracy

  //Some Co2 sensor checks
  memset(co2SensorVersion, '\0', 5);
  co2Sensor.getVersion(co2SensorVersion);
  if (CO2_SENSOR_CO2_RANGE!=co2Sensor.getRange() || (byte) 0 != co2Sensor.getAccuracy(false) ||
      0==co2SensorType.compareTo("UNKNOW"))
    error_setup = ERROR_SENSOR_CO2_SETUP;
  if (error_setup != ERROR_SENSOR_CO2_SETUP ) { 
    if (logsOn) {
      Serial.println("OK");
      Serial.print("  CO2 Sensor type: "); Serial.println(co2SensorType); 
      Serial.print("  CO2 Sensor version: "); Serial.println(co2SensorVersion);
      Serial.print("  CO2 Sensor Accuracy: "); Serial.println(co2Sensor.getAccuracy(false));
      Serial.print("  CO2 Sensor Range: "); Serial.println(co2Sensor.getRange());
    }
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor type: ");stext1.print(co2SensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor version: ");stext1.print(co2SensorVersion);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Accuracy: ");stext1.print(co2Sensor.getAccuracy(false));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Range: ");stext1.print(co2Sensor.getRange());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  } else {
    if (logsOn) {
      Serial.println("KO");
      Serial.print("  CO2 Sensor type: "); Serial.print(co2SensorType);Serial.println(" - Shouldn't be UNKNOWN");
      Serial.print("  CO2 Sensor version: "); Serial.println(co2SensorVersion);
      Serial.print("  CO2 Sensor Accuracy: "); Serial.print(co2Sensor.getAccuracy(false)); Serial.println(" - Should be 0");
      Serial.print("  CO2 Sensor Range: "); Serial.print(co2Sensor.getRange()); Serial.print(" - Should be ");Serial.println(CO2_SENSOR_CO2_RANGE);
      Serial.println("  Can't continue. STOP");
    }
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor type: ");if (0==co2SensorType.compareTo("UNKNOW")) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(co2SensorType);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor version: ");stext1.print(co2SensorVersion);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Accuracy: ");if ((byte) 0 != co2Sensor.getAccuracy(false)) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(co2Sensor.getAccuracy(false));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  CO2 Sensor Range: ");if  (CO2_SENSOR_CO2_RANGE!=co2Sensor.getRange()) stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print(co2Sensor.getRange());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.print(" ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Can't continue. STOP");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    #if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE
      return;  //Development doesn't have CO2 sensor
    #endif
  }

  //Initiating buffers to draw the Co2/Temp/Hum graphs
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)  {lastHourCo2Samples[i]=0;lastHourTempSamples[i]=0;lastHourHumSamples[i]=0;}
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++) {lastDayCo2Samples[i]=0;lastDayTempSamples[i]=0;lastDayHumSamples[i]=0;}

  //-->>Buttons init
  if (logsOn) Serial.print("[setup] - Buttons: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Buttons: [");
  button1.begin();
  button2.begin();
  if (error_setup != ERROR_BUTTONS_SETUP ) { 
    if (logsOn) Serial.println("OK");
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");
  } else {
    if (logsOn) {Serial.println("KO"); Serial.println("Can't continue. STOP");}
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.print(" ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Can't continue. STOP");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    return;
  }
  stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);

  //WiFi init
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - WiFi: ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  cuX=stext1.getCursorX(); cuY= pLL>scLL? stext1.getCursorY()-pixelsPerLine : stext1.getCursorY();
  stext1.setCursor(cuX,cuY);

  //User credentials definition
  #ifdef WIFI_SSID_CREDENTIALS
    wifiCred.wifiSSIDs[0]=WIFI_SSID_CREDENTIALS;
  #endif
  #ifdef WIFI_PW_CREDENTIALS
    wifiCred.wifiPSSWs[0]=WIFI_PW_CREDENTIALS;
  #endif
  #ifdef WIFI_SSID_CREDENTIALS_BK1
    wifiCred.wifiSSIDs[1]=WIFI_SSID_CREDENTIALS_BK1;
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK1
    wifiCred.wifiPSSWs[1]=WIFI_PW_CREDENTIALS_BK1;
  #endif
  #ifdef WIFI_SSID_CREDENTIALS_BK2
    wifiCred.wifiSSIDs[2]=WIFI_SSID_CREDENTIALS_BK2;
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK2
    wifiCred.wifiPSSWs[2]=WIFI_PW_CREDENTIALS_BK2;
  #endif

  error_setup=wifiConnect();
  //Clean-up dots displayed after trying to get connected
  stext1.setCursor(cuX,cuY);
  for (int counter2=0; counter2<MAX_CONNECTION_ATTEMPTS*(wifiCred.activeIndex+1); counter2++) stext1.print(" ");
  stext1.setCursor(cuX,cuY);

  //print Logs
  if (logsOn) Serial.print("[setup] - WiFi: ");
  if (error_setup != ERROR_WIFI_SETUP ) { 
    if (logsOn) Serial.println("OK");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("OK");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");//if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SSID:  ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print(wifiNet.ssid);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("    IP:  ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(WiFi.localIP().toString());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  MASK: ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(WiFi.subnetMask().toString());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  DFGW: ");stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.println(WiFi.gatewayIP().toString());if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);

    if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
        else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
        else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
        else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
        else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
  } else {
    if (logsOn) Serial.println("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print(" [");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("]");
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SSID: ");stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print("No SSID Available");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    wifiCurrentStatus=wifiOffStatus;
  }

  //Setting up URL things to upload samples to an external server
  CloudSyncCurrentStatus=CloudSyncOffStatus;
  if (error_setup != ERROR_WIFI_SETUP && uploadSamplesToServer) { 
    //Converting SERVER_UPLOAD_SAMPLES into IPAddress variable
    char charToTest;
    int IPAddressOctectArray[4];
    uint lastBegin=0,indexArray=0;
    for (uint i=0; i<=serverToUploadSamplesString.length(); i++) {
      charToTest=serverToUploadSamplesString.charAt(i);
      if (charToTest=='.') {    
        IPAddressOctectArray[indexArray]=serverToUploadSamplesString.substring(lastBegin,i).toInt();
        lastBegin=i+1;
        if (indexArray==2) {
          indexArray++;
          IPAddressOctectArray[indexArray]=serverToUploadSamplesString.substring(lastBegin,serverToUploadSamplesString.length()).toInt();
        }
        else indexArray++;
      }
    }
    serverToUploadSamplesIPAddress=IPAddress(IPAddressOctectArray[0],IPAddressOctectArray[1],IPAddressOctectArray[2],IPAddressOctectArray[3]);

    //Adding the 3 latest mac bytes to the device name (in Hex format)
    byte mac[6];
    WiFi.macAddress(mac);
    device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
      String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
      String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);
    
    //Send HttpRequest to check the server status
    // The request updates CloudSyncCurrentStatus
    sendHttpRequest(serverToUploadSamplesIPAddress, SERVER_UPLOAD_PORT, String(GET_REQUEST_TO_UPLOAD_SAMPLES)+"test HTTP/1.1");

    Serial.print("[setup] - URL: [");
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - URL: [");

    if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
      Serial.println("OK]");
      
      stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  URL: ");
      stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);
    }
    else {
      Serial.println("KO]");

      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
      stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  URL: ");
      stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);

      error_setup != ERROR_WEB_SERVER;
    }

    Serial.print("  - URL: ");Serial.println("http://"+serverToUploadSamplesIPAddress.toString()+
        String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1));
    Serial.print("  - Device name=");Serial.println(device);

    stext1.print("http://"+serverToUploadSamplesIPAddress.toString()+
        String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine); 
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  Device name: ");
    stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print(device);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  //NTP Server
  setupNTPConfig(true);
  if (error_setup==ERROR_NTP_SERVER) {
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - NTP: [");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  NTP Server: ");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);stext1.print((String)(ntpServers[0]+" "+ntpServers[1]+" "+ntpServers[2]+" "+ntpServers[3]));if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }
  else {
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - NTP: [");
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.print("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK); stext1.print("  Date: ");stext1.print(&startTimeInfo,"%d/%m/%Y - %H:%M:%S");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  NTP Server: ");
    stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print(ntpServers[ntpServerIndex]);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  //-->>BLE init
  if (logsOn) Serial.print("[setup] - BLE: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - BLE:     [");
  if (error_setup != ERROR_BLE_SETUP ) { 
    if (logsOn) Serial.println("OK");
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
  } else {
    if (logsOn) Serial.println("KO");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
  }
  stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  
  //-->>Battery and ADC init
  if (logsOn) Serial.print("[setup] - Bat. ADC: ");
  stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - Bat. ADC [");
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE);
  initVoltageArray(); //Init battery charge array
  powerState=off;
  attenuationDb=ADC_ATTEN_DB_11;
  esp_adc_cal_characterize(ADC_UNIT_1, attenuationDb, (adc_bits_width_t) ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
  if (0!=adc1_config_channel_atten(ADC1_CHANNEL_6, attenuationDb)) error_setup=ERROR_BAT_ADC;
  if (error_setup != ERROR_BAT_ADC ) { 
    if (logsOn) {
      Serial.println("OK");
      Serial.print(" - adc_num=");Serial.println(adc1_chars.adc_num);
      Serial.print(" - atten=");Serial.println(adc1_chars.atten);
      Serial.print(" - bit_width=");Serial.println(adc1_chars.bit_width);
      Serial.print(" - coeff_a=");Serial.println(adc1_chars.coeff_a);
      Serial.print(" - coeff_b=");Serial.println(adc1_chars.coeff_b);
      Serial.print(" - vref=");Serial.println(adc1_chars.vref);
      Serial.print(" - version=");Serial.println(adc1_chars.version);
    }

    //Take BAT ADC for setup powerState
    batCharge=0;lastBatCharge=0;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
    batADCVolt=0; for (u8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE);

    if (batADCVolt >= VOLTAGE_TH_STATE) {
      timeUSBPower=nowTime;
      powerState=chargingUSB; //check later if powerState is noChargingUSB instead
      batteryStatus=batteryCharging000;
    }
    else {
      powerState=onlyBattery;
      batteryStatus=getBatteryStatus(batADCVolt,0);
      timeUSBPower=0;
    }
    lastPowerState=powerState;
    stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
  } else {
    if (logsOn) Serial.println("KO");
    stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
  }
  stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("]");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);

  if (error_setup != NO_ERROR) {
    if (logsOn) Serial.println("Ready to start but with limitations");
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("Buttons to scroll UP/DOWN");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_ORANGE_4_BITS_PALETTE,TFT_BLACK);stext1.print("Ready in ");cuX=stext1.getCursorX();cuY=stext1.getCursorY();
    stext1.print(BOOTUP_TIMEOUT);stext1.print(" sec. but with limitations");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  } else {
    if (logsOn) Serial.println("Ready to start");
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("Buttons to scroll UP/DOWN");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
    stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK);stext1.print("Ready to start in ");cuX=stext1.getCursorX();cuY=stext1.getCursorY();
    stext1.print(BOOTUP_TIMEOUT);stext1.print(" sec.");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
  }

  /*-->*/randomSeed(analogRead(37));

  previousTime=millis();
  remainingBootupTime=BOOTUP_TIMEOUT*1000;
  remainingBootupSeconds=(uint8_t)(remainingBootupTime/1000);
}

void loop() {
  nowTime=millis();
  if (remainingBootupTime>0) {
    //Only while showing bootup screen
    remainingBootupTime-=(nowTime-previousTime);
    previousTime=nowTime;
  }

  //Keep showing bootup screen for a while
  if (currentState==bootupScreen) {
      if (remainingBootupSeconds!=(uint8_t)(remainingBootupTime/1000)) {
      remainingBootupSeconds=(uint8_t)(remainingBootupTime/1000);
      
      //Update screen if last line is visible in the scroll window
      if (pLL-1>=scFL && pLL-1<=scLL) {
        stext1.setCursor(cuX,(pLL-2)*pixelsPerLine);stext1.print(remainingBootupSeconds);
        stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
    }

    //Scroll DOWN
    if (button1.pressed()) {
      if (pFL<scFL) {
        pFL++;
        if (pLL-1<spLL) pLL++;
        stext1.scroll(0,pixelsPerLine);
        stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
      //Reset timeout to give more time to read while scrolling the screen 
      if (pLL-1>=scFL && pLL-1<=scLL)
        remainingBootupTime=BOOTUP_TIMEOUT*1000;
      else
        remainingBootupTime=BOOTUP_TIMEOUT2*1000; //If last line is not in scroll windows, wait a bit more
    }

    //Scroll UP
    if (button2.pressed()) {
      if (pLL-1>scLL) {
        pLL--;
        if (pFL>spFL) pFL--;
        stext1.scroll(0,-pixelsPerLine);
        stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
      //Reset timeout to give more time to read while scrolling the screen 
      if (pLL-1>=scFL && pLL-1<=scLL)
        remainingBootupTime=BOOTUP_TIMEOUT*1000;
      else
        remainingBootupTime=BOOTUP_TIMEOUT2*1000; //If last line is not in scroll windows, wait a bit more
    }
    
    if (ERROR_DISPLAY_SETUP==error_setup || ERROR_SENSOR_CO2_SETUP==error_setup || 
        ERROR_SENSOR_TEMP_HUM_SETUP==error_setup || ERROR_BUTTONS_SETUP==error_setup)
          remainingBootupTime=BOOTUP_TIMEOUT*1000;
          
    if (remainingBootupTime<=0) {
      //After BOOTUP_TIMEOUT secons with no activity leaves bootupScreen and continue
      tft.setCursor(0,0,TEXT_FONT);
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(TEXT_SIZE);
      displayMode=sampleValue;lastDisplayMode=bootup; //Will make starting in sampleValue      
      lastState=currentState;currentState=displayingSequential;  //Transition to the next state
      previousTime=0;
    }
  
    return;
  }
  
  //Show Warming screen
  while (nowTime<co2PreheatingTime) {
    //Waiting for the sensor to warmup before displaying value
    if (waitingMessage) {if (logsOn) Serial.println("Waiting for the warmup to finish");circularGauge.drawGauge2(0);waitingMessage=false;}
    circularGauge.cleanValueTextGauge();
    circularGauge.cleanUnitsTextGauge();
    circularGauge.setValue((int)(co2PreheatingTime-millis())/1000);
    circularGauge.drawTextGauge("warmup",TEXT_SIZE,true,TEXT_SIZE_UNITS_CO2,TEXT_FONT,TEXT_FONT_UNITS_CO2,TFT_GREENYELLOW);
    
    showIcons();
    delay(1000);
    nowTime=millis();
    previousTurnOffBacklightTime=nowTime;
  }
  if (runningMessage) {if (logsOn) Serial.println("Running.... :-)");runningMessage=false;}

  //Taking time since last updates
  gapTime = previousTime!=0 ? nowTime-previousTime: SAMPLE_PERIOD;
  gapTimeDisplay = previousTimeDisplay!=0 ? nowTime-previousTimeDisplay: DISPLAY_REFRESH_PERIOD;
  gapTimeDisplayMode = previousTimeDisplayMode!=0 ? nowTime-previousTimeDisplayMode: DISPLAY_MODE_REFRESH_PERIOD;
  gapHourSampleTime = previousHourSampleTime!=0 ? nowTime-previousHourSampleTime: SAMPLE_T_LAST_HOUR*1000;
  gapDaySampleTime = previousDaySampleTime!=0 ? nowTime-previousDaySampleTime: SAMPLE_T_LAST_DAY*1000;
  gapUploadSampleTime = previousUploadSampleTime!=0 ? nowTime-previousUploadSampleTime: UPLOAD_SAMPLES_PERIOD;
  gapTimeIconStatusRefresh = previousTimeIconStatusRefresh!=0 ? nowTime-previousTimeIconStatusRefresh: ICON_STATUS_REFRESH_PERIOD;
  gapTurnOffBacklight = previousTurnOffBacklightTime!=0 ? nowTime-previousTurnOffBacklightTime: TIME_TURN_OFF_BACKLIGHT;
  gapTimeNTPCheck = previousTimeNTPCheck!=0 ? nowTime-previousTimeNTPCheck: NTP_KO_CHECK;
  gapTimeVOLTCheck = previousTimeVOLTCheck!=0 ? nowTime-previousTimeVOLTCheck: VOLTAGE_CHECK_PERIOD;
  
  //Cheking if NTP is off or should be checked
  if (gapTimeNTPCheck>=NTP_KO_CHECK && lastDisplayMode!=bootup) {
    previousTimeNTPCheck=nowTime;
    if (wifiCurrentStatus!=wifiOffStatus && 
        (CloudClockCurrentStatus==CloudClockOffStatus ||
         (startTimeInfo.tm_hour==NTP_OK_CHECK_HOUR && startTimeInfo.tm_min==NTP_OK_CHECK_MINUTE)) ) {
          setupNTPConfig(false);
    }      
  }
  
  //Cheking if BackLight should be turned off
  if ( digitalRead(PIN_TFT_BACKLIGHT)!=LOW && autoBackLightOff==true &&
      (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed ||
       currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) &&
      gapTurnOffBacklight >= TIME_TURN_OFF_BACKLIGHT) {
        if (lastDisplayMode==bootup)
          previousTurnOffBacklightTime=nowTime;
        else {
          digitalWrite(PIN_TFT_BACKLIGHT,LOW);
          previousTurnOffBacklightTime=nowTime;
          gapTurnOffBacklight=0;
        }
  }
  
  //Checking if sample buffers update is needed
  if (gapHourSampleTime>=SAMPLE_T_LAST_HOUR*1000) {previousHourSampleTime=nowTime;gapHourSampleTime=0;updateHourSample=true;updateHourGraph=true;}
  if (gapDaySampleTime>=SAMPLE_T_LAST_DAY*1000) {previousDaySampleTime=nowTime;gapDaySampleTime=0;updateDaySample=true;updateDayGraph=true;}

  //Actions if button1 is pushed. It depens on the current state
  if (button1.pressed()) checkButton1();

  //Actions if button2 is pushed. It depens on the current state
  if (button2.pressed()) {
    //Take time to check if it is long press
    button2Pressed=true;
    if (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed ||
        currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential)
      timePressButton2=millis();
    else
      timePressButton2=0;
    checkButton2();
  }

  if (button2.released()) button2Pressed=false;
  //Check if Button2 was long pressed
  //if (button2.released() && timePressButton2!=0) {
  if (button2Pressed && timePressButton2!=0) { 
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
        previousTurnOffBacklightTime=nowTime;
        gapTurnOffBacklight=0;
      }
      else {
        //Display message in the screen
        tft.setTextColor(TFT_RED,TFT_BLACK);
        tft.setCursor(0,tft.height()/2+2,TEXT_FONT_MENU);tft.println("     Disabled");
        delay(2500);
        tft.fillScreen(TFT_BLACK);
      }
    }
  }
  
  //Regular actions every VOLTAGE_CHECK_PERIOD seconds
  //Checking out whether the voltage exceeds thresholds to detect energy source (bat or USB)
  if (gapTimeVOLTCheck>=VOLTAGE_CHECK_PERIOD) {
    previousTimeVOLTCheck=nowTime;gapTimeVOLTCheck=0;
  
    //Power state check
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
    batADCVolt=0; for (u8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume
    
    if (batADCVolt >= VOLTAGE_TH_STATE) {
      //USB is plugged. Assume battery is always plugged and charged after FULL_CHARGE_TIME milliseconds
      if(noChargingUSB!=powerState) {
        powerState=chargingUSB;
        if ((nowTime-timeUSBPower)>=FULL_CHARGE_TIME)
          powerState=noChargingUSB;
      }
      if (0==timeUSBPower) timeUSBPower=nowTime;
    }
    else {
      powerState=onlyBattery;
      timeUSBPower=0;
    }

    if (lastPowerState!=powerState) {
      gapTimeBATCheck=BATTERY_CHECK_PERIOD; //Refresh display with the right battery icon
      lastPowerState=powerState;
    }
  }
  else gapTimeVOLTCheck=nowTime-previousTimeVOLTCheck;

  //Regular actions every SAMPLE_PERIOD seconds
  //  Taking CO2, Temp & Hum samples. Moving buffers at the right time
  if (gapTime>=SAMPLE_PERIOD) {
    previousTime=nowTime;lastGapTime=gapTime;gapTime=0;

    //Getting CO2 & Temp values
    /*-->valueCO2=(float_t)random(0,2000);<--*/
    /*-->valueT=(float_t)random(0,600)/10-10.0;<--*/
    /*-->valueHum=(float_t)random(0,100);<--*/
    valueCO2=(float_t)co2Sensor.getCO2();
    
    //tempMeasure=co2Sensor.getTemperature(true,true);
    tempHumSensor.read();
    //tempMeasure=tempHumSensor.getTemperature();
    tempMeasure=0.9944*tempHumSensor.getTemperature()-0.8073; //Calibrated value
    //valueHum=0;
    valueHum=tempHumSensor.getHumidityCompensated();
    
    if (tempMeasure>-50.0) valueT=tempMeasure;  //Discarding potential wrong values

    //Updating the last hour buffers
    if (updateHourSample) {
      for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR)-1; i++) 
      {lastHourCo2Samples[i]=lastHourCo2Samples[i+1];
      lastHourTempSamples[i]=lastHourTempSamples[i+1];
      lastHourHumSamples[i]=lastHourHumSamples[i+1];
      }
      lastHourCo2Samples[(int)(3600/SAMPLE_T_LAST_HOUR)-1]=valueCO2;
      lastHourTempSamples[(int)(3600/SAMPLE_T_LAST_HOUR)-1]=valueT;
      lastHourHumSamples[(int)(3600/SAMPLE_T_LAST_HOUR)-1]=valueHum;
      updateHourSample=false;
    }
    
    //Updating the last day Co2 buffer
    if (updateDaySample) {
      for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY)-1; i++) {
        lastDayCo2Samples[i]=lastDayCo2Samples[i+1];
        lastDayTempSamples[i]=lastDayTempSamples[i+1];
        lastDayHumSamples[i]=lastDayHumSamples[i+1];
      }
      lastDayCo2Samples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueCO2;
      lastDayTempSamples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueT;
      lastDayHumSamples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueHum;
      updateDaySample=false;
    }
  }

  //Regular actions every BATTERY_CHECK_PERIOD seconds - Update battery charge status
  if (gapTimeBATCheck>=BATTERY_CHECK_PERIOD) {
    previousTimeBATCheck=nowTime;gapTimeBATCheck=0;

    //Getting bat voltage
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
    batADCVolt=0; for (u8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume

    if (onlyBattery==powerState)
      //Take battery charge when the Battery is plugged
      batteryStatus=getBatteryStatus(batADCVolt,0);
    else
      //When USB is plugged, the Battery charge can be only guessed based on
      // the time the USB is being plugged 
      batteryStatus=getBatteryStatus(batADCVolt,nowTime-timeUSBPower);
  }
  else gapTimeBATCheck=nowTime-previousTimeBATCheck;
  
  //Regular actions every UPLOAD_SAMPLES_PERIOD seconds - Upload samples to external server
  if (gapUploadSampleTime>=UPLOAD_SAMPLES_PERIOD && uploadSamplesToServer &&
      lastDisplayMode!=bootup && wifiCurrentStatus!=wifiOffStatus) {
    String httpRequest=String(GET_REQUEST_TO_UPLOAD_SAMPLES);
    previousUploadSampleTime=nowTime;gapUploadSampleTime=0;

    //GET /lar-co2/?device=co2-sensor&local_ip_address=192.168.100.192&co2=543&temp_by_co2_sensor=25.6&hum_by_co2_sensor=55&temp_co2_sensor=28.7
    httpRequest=httpRequest+"device="+device+"&local_ip_address="+
      IpAddress2String(WiFi.localIP())+"&co2="+valueCO2+"&temp_by_co2_sensor="+valueT+"&hum_by_co2_sensor="+
      valueHum+"&temp_co2_sensor="+co2Sensor.getTemperature(true,true)+" HTTP/1.1";

    sendHttpRequest(serverToUploadSamplesIPAddress, SERVER_UPLOAD_PORT, httpRequest);
  }
  else gapUploadSampleTime=nowTime-previousUploadSampleTime;

  //Regular actions every ICON_STATUS_REFRESH_PERIOD seconds
  // Refresh icon status
  if (gapTimeIconStatusRefresh>=ICON_STATUS_REFRESH_PERIOD && lastDisplayMode!=bootup) {
    //Make sure scanning doesn't block display printing
    if ((DISPLAY_REFRESH_PERIOD-gapTimeDisplay) <= 2500) {
      previousTimeIconStatusRefresh=nowTime+DISPLAY_REFRESH_PERIOD+gapTimeDisplay+500-ICON_STATUS_REFRESH_PERIOD;
      gapTimeIconStatusRefresh=nowTime-previousTimeIconStatusRefresh;
    }
    else {
      previousTimeIconStatusRefresh=nowTime;gapTimeIconStatusRefresh=0;

      if ((wifiCurrentStatus != wifiOffStatus) && 
          (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed ||
           currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) ) {
        int16_t numberWiFiNetworks=0;
        printCurrentWiFi(false,&numberWiFiNetworks);
        if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
        else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
        else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
        else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
        else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
        
        /*-->Serial.print("[Refresh Display] - nowTime=");Serial.print(nowTime);Serial.print(", previousTimeIconStatusRefresh=");Serial.print(previousTimeIconStatusRefresh);
        Serial.print(", lasted time=");Serial.print((nowTime-previousTimeIconStatusRefresh)/1000);Serial.print(", wifiNet.ssid=");Serial.print(wifiNet.ssid);
        Serial.print(", wifiCurrentStatus=");Serial.print(wifiCurrentStatus);Serial.print(", wifiNet.RSSI=");Serial.println(wifiNet.RSSI); 
        /<--*/
      }
    }
  }
  
  
  //Regular actions every DISPLAY_MODE_REFRESH_PERIOD seconds
  // Selecting what's the screen to display (active screen)
  if (gapTimeDisplayMode>=DISPLAY_MODE_REFRESH_PERIOD && currentState==displayingSequential) {
    previousTimeDisplayMode=nowTime;gapTimeDisplayMode=0;

    switch (displayMode) {
      case sampleValue:
        if (lastDisplayMode!=bootup) displayMode=co2LastHourGraph; //Next display mode only if coming from bootup screen
      break;
      case co2LastHourGraph:
        displayMode=co2LastDayGraph; //Next display mode
      break;
      case co2LastDayGraph:
        displayMode=sampleValue; //Next display mode
      break;
    }
  }
  
  //Regular actions every DISPLAY_REFRESH_PERIOD seconds
  // Display the active screen
  if (gapTimeDisplay>=DISPLAY_REFRESH_PERIOD && (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || 
                                                 currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) ) {
    previousTimeDisplay=nowTime;gapTimeDisplay=0;

    switch (displayMode) {
      case  sampleValue:
        //Sample values are displayed
        if (lastGapTime>=SAMPLE_PERIOD) {
          //Display refresh only when getting new sample

          //Cleaning the screen always the first time in
          if (lastDisplayMode!=sampleValue) {tft.fillScreen(TFT_BLACK); lastValueCO2=-1;} //Force re-rendering CO2 values in the main screen
          
          //Drawing Icons
          showIcons();

          //Cleaning & Drawing circular gauge
          tft.setTextSize(TEXT_SIZE);
          if ( lastValueCO2<0 || abs(1-lastValueCO2/valueCO2)*100>10 ||
              (lastValueCO2<circularGauge.th1 && valueCO2>=circularGauge.th1) ||
              (lastValueCO2>=circularGauge.th1 && valueCO2<circularGauge.th1) ||
              (lastValueCO2>circularGauge.th1 && lastValueCO2<circularGauge.th2 && valueCO2>circularGauge.th2) ||
              (lastValueCO2>=circularGauge.th2 && valueCO2<circularGauge.th2))
          {
            //First cleaning all the of items to avoid partial deletion of the horizontal bar
            tft.fillRect(horizontalBar.xStart,horizontalBar.yStart-tft.fontHeight(TEXT_FONT)-1,horizontalBar.width,tft.fontHeight(TEXT_FONT),TFT_BLACK);
            horizontalBar.cleanHorizontalBar();
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String("Hum.")),90,tft.textWidth(String("Hum.")),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            valueString=String("----");
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(valueString),90+tft.fontHeight(TEXT_FONT),tft.textWidth(valueString+"%"),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            circularGauge.cleanAll();
            
            //Drawing the circularGauge
            circularGauge.drawGauge2(valueCO2); lastValueCO2=valueCO2;
          }
          else {
            //First cleaning all bar and text items to before re-drawing them
            tft.fillRect(horizontalBar.xStart,horizontalBar.yStart-tft.fontHeight(TEXT_FONT)-1,horizontalBar.width,tft.fontHeight(TEXT_FONT),TFT_BLACK);
            horizontalBar.cleanHorizontalBar();
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String("Hum.")),90,tft.textWidth(String("Hum.")),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String(round(valueHum))+"%"),90+tft.fontHeight(TEXT_FONT),tft.textWidth(String(round(valueHum))+"%"),tft.fontHeight(TEXT_FONT),TFT_BLACK);

            //Partial displaying of the circular gauge (only the text value) to don't take too much displaying it
            circularGauge.setValue(valueCO2);
            circularGauge.cleanValueTextGauge();
            circularGauge.cleanUnitsTextGauge();
          }
          circularGauge.drawTextGauge("ppm");

          //Drawing temperature
          //valueString=String((int) valueT)+"."+String(abs(((int) (valueT*10))-(((int)valueT)*10)))+"C";
          valueString=roundFloattoString(valueT,1)+"C";
          tft.setTextSize(TEXT_SIZE);
          drawText(valueT, String(valueString),TEXT_SIZE,TEXT_FONT,TFT_GREEN,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),25,19.95,TFT_BLUE,27.05,TFT_RED);
          horizontalBar.drawHorizontalBar(valueT);

          //Drawing Humidity
          valueString=String("Hum.");
          tft.setTextSize(TEXT_SIZE);
          drawText(valueHum,String(valueString),TEXT_SIZE,TEXT_FONT,TFT_MAGENTA,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),90,30,TFT_MAGENTA,55,TFT_MAGENTA);
          valueString=String(int(round(valueHum)))+"%";
          //valueString=String("----");
          drawText(valueHum,String(valueString),TEXT_SIZE,TEXT_FONT,TFT_MAGENTA,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),90+tft.fontHeight(TEXT_FONT),30,TFT_BROWN,55,TFT_RED);
        
        }
        lastDisplayMode=sampleValue;
      break;
      case co2LastHourGraph:
        //Last Hour graph is displayed
        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastHourGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastHourCo2(); updateHourGraph=false;}
        else if (updateHourGraph) {drawGraphLastHourCo2(); updateHourGraph=false;}//Draw new graph only if buffer was updated

        lastDisplayMode=co2LastHourGraph;
      break;
      case co2LastDayGraph:
        //Last Day graph is displayed
        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastDayGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastDayCo2(); updateDayGraph=false;}
        else if (updateDayGraph) {drawGraphLastDayCo2(); updateDayGraph=false;}//Draw new graph only if buffer was updated
      
        lastDisplayMode=co2LastDayGraph;
      break;
    }
  }
}