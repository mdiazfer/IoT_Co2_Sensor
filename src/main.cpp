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
#include <SHT2x.h>

#ifdef __MHZ19B__
  const ulong co2PreheatingTime=MH_Z19B_CO2_WARMING_TIME;
#endif

//Global variable definitions
const String co2SensorType=String(CO2_SENSOR_TYPE);
const String tempHumSensorType=String(TEMP_HUM_SENSOR_TYPE);
uint8_t error_setup = NO_ERROR;
TFT_eSPI tft = TFT_eSPI();  // Invoke library to manage the display
ulong nowTime=0,previousTime=0,previousTimeDisplay=0,previousTimeDisplayMode=0,
      previousHourSampleTime=0,previousDaySampleTime=0,
      gapTime,lastGapTime,gapTimeDisplay,gapTimeDisplayMode,gapHourSampleTime,gapDaySampleTime;
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
enum displayModes displayMode=sampleValue, lastDisplayMode=bootup; //Will make starting always in sampleValue
enum availableStates stateSelected=displayingSampleFixed,currentState=bootupScreen,lastState=currentState;
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


//Code
void loadBootImage() {
  //-->>Load the logo image when booting up

  return;
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
  auxDec=abs(auxEnt*10-dec);
  if (auxDec>=5) auxEnt++;

  if (decimals==0) myString=String(number).toInt(); 
  else myString=String(ent)+"."+String(auxEnt);

  return myString;
}

void setup() {
  static uint32_t wr = 1;
  static uint32_t rd = 0xFFFFFFFF;
  currentState=displayingSequential;lastState=currentState;

  if (logsOn) {Serial.begin(115200);Serial.print("\nCO2 bootup v");Serial.print(VERSION);Serial.println(" ..........");Serial.println("[setup] - Serial: OK");}

  //Display init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

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

  loadBootImage();
  delay(500);
  //Display messages
  tft.setCursor(0,0,TEXT_FONT_BOOT_SCREEN);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.setTextColor(TFT_WHITE,TFT_BLACK); tft.print("CO2 bootup v");tft.print(VERSION);tft.println(" ..........");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Display: [");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");

  //Some Temp & Hum sensor checks and init
  if (logsOn) Serial.print("[setup] - Sensor Temp/HUM: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Tp/Hu:  [");
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
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("] ");
    tft.println(tempHumSensorType);
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
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  Tp/Hm Sns. type: "); if (0==tempHumSensorType.compareTo("UNKNOW")) tft.setTextColor(TFT_RED,TFT_BLACK); tft.println(tempHumSensorType);
    //tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  Tp/Hm Sen. version: "); tft.println(tempHumSensor.getFirmwareVersion());
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  Tp/Hm Sen. status: "); tft.println(statSns,HEX);
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  Tp/Hm Sen.  error: "); if (0 != errorSns) tft.setTextColor(TFT_RED,TFT_BLACK); tft.println(errorSns,HEX);
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  Tp/Hm Sen. resolu.: "); tft.println(tempHumSensor.getResolution());
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.println("\n  Can't continue. STOP"); 
    return;
  }

  //Sensor CO2 init
  if (logsOn) Serial.print("[setup] - Sensor: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Sens.:  [");
  co2SensorSerialPort.begin(9600);      // (Uno example) device to MH-Z19 serial start   
  co2Sensor.begin(co2SensorSerialPort); // *Serial(Stream) refence must be passed to library begin(). 
  co2Sensor.setRange(CO2_SENSOR_CO2_RANGE);             // It's aviced to setup range to 2000. Better accuracy

  //Some Co2 sensor checks
  char co2SensorVersion[5];memset(co2SensorVersion, '\0', 5);
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
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("] ");
    tft.print(co2SensorType);tft.print(",V");tft.println(co2SensorVersion);
  } else {
    if (logsOn) {
      Serial.println("KO");
      Serial.print("  CO2 Sensor type: "); Serial.print(co2SensorType);Serial.println(" - Shouldn't be UNKNOWN");
      Serial.print("  CO2 Sensor version: "); Serial.println(co2SensorVersion);
      Serial.print("  CO2 Sensor Accuracy: "); Serial.print(co2Sensor.getAccuracy(false)); Serial.println(" - Should be 0");
      Serial.print("  CO2 Sensor Range: "); Serial.print(co2Sensor.getRange()); Serial.print(" - Should be ");Serial.println(CO2_SENSOR_CO2_RANGE);
      Serial.println("  Can't continue. STOP");
    }
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  CO2 Sensor type: "); if (0==co2SensorType.compareTo("UNKNOW")) tft.setTextColor(TFT_RED,TFT_BLACK); tft.println(co2SensorType);
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  CO2 Sensor version: "); tft.println(co2SensorVersion);
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  CO2 Sensor Accuracy: "); if ((byte) 0 != co2Sensor.getAccuracy(false)) tft.setTextColor(TFT_RED,TFT_BLACK); tft.println(co2Sensor.getAccuracy(false));
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("  CO2 Sensor Range: "); if  (CO2_SENSOR_CO2_RANGE!=co2Sensor.getRange()) tft.setTextColor(TFT_RED,TFT_BLACK); tft.println(co2Sensor.getRange());
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.println("\n  Can't continue. STOP");
    return;
  }
 
  //Initiating buffers to draw the Co2/Temp/Hum graphs
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)  {lastHourCo2Samples[i]=0;lastHourTempSamples[i]=0;lastHourHumSamples[i]=0;}
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++) {lastDayCo2Samples[i]=0;lastDayTempSamples[i]=0;lastDayHumSamples[i]=0;}

  //-->>Buttons init
  if (logsOn) Serial.print("[setup] - Buttons: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Buttons: [");
  button1.begin();
  button2.begin();
  if (error_setup != ERROR_BUTTONS_SETUP ) { 
    if (logsOn) Serial.println("OK");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  } else {
    if (logsOn) {Serial.println("KO"); Serial.println("Can't continue. STOP");}
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.println("Can't continue. STOP");
    return;
  }
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");

  //WiFi init
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - WiFi: ");  
  int16_t cuX=tft.getCursorX(); int16_t cuY=tft.getCursorY();
  tft.setCursor(cuX,cuY);
  error_setup=wifiConnect();
  //Clean-up dots displayed after trying to get connected
  for (int counter2=0; counter2<MAX_CONNECTION_ATTEMPTS; counter2++) tft.print(" ");
  tft.setCursor(cuX,cuY);
  //print Logs
  if (logsOn) Serial.print("[setup] - WiFi: ");
  if (error_setup != ERROR_WIFI_SETUP ) { 
    if (logsOn) Serial.println("OK");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print(" [");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("] - ");
    tft.setTextColor(TFT_GREEN,TFT_BLACK);tft.print(wifiNet.ssid);tft.print(", ");tft.println(WiFi.localIP().toString());
  } else {
    if (logsOn) Serial.println("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("   [");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
  }
  
  //-->>BLE init
  if (logsOn) Serial.print("[setup] - BLE: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - BLE:     [");
  if (error_setup != ERROR_BLE_SETUP ) { 
    if (logsOn) Serial.println("OK");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  } else {
    if (logsOn) Serial.println("KO");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
  }
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");


  if (error_setup != NO_ERROR) {
    if (logsOn) Serial.println("Ready to start with limitations");
    tft.setTextColor(TFT_BROWN,TFT_BLACK); tft.println("Ready to start with limitations");
  } else {
    if (logsOn) Serial.println("Ready to start");
    tft.setTextColor(TFT_DARKGREEN,TFT_BLACK); tft.println("Ready to start");
  }

  delay(5000);
  tft.setCursor(0,0,TEXT_FONT);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(TEXT_SIZE);
  /*-->*/randomSeed(analogRead(37));
  circularGauge.drawGauge2(0);
  currentState=displayingSequential;lastState=currentState; //Transition to the next state
}

void loop() {
  if (ERROR_DISPLAY_SETUP==error_setup || ERROR_SENSOR_CO2_SETUP==error_setup || 
      ERROR_SENSOR_TEMP_HUM_SETUP==error_setup)
    return;

  nowTime=millis();

  while (nowTime<co2PreheatingTime) {
    //Waiting for the sensor to warmup before displaying value
    if (nowTime<10000) if (logsOn) Serial.println("Waiting for the warmup to finish");
    circularGauge.cleanValueTextGauge();
    circularGauge.cleanUnitsTextGauge();
    circularGauge.setValue((int)(co2PreheatingTime-nowTime)/1000);
    circularGauge.drawTextGauge("warmup",TEXT_SIZE,true,TEXT_SIZE_UNITS_CO2,TEXT_FONT,TEXT_FONT_UNITS_CO2,TFT_GREENYELLOW);
    delay(1000);
    nowTime=millis();
  }

  //Taking time since last updates
  gapTime = previousTime!=0 ? nowTime-previousTime: SAMPLE_PERIOD;
  gapTimeDisplay = previousTimeDisplay!=0 ? nowTime-previousTimeDisplay: DISPLAY_REFRESH_PERIOD;
  gapTimeDisplayMode = previousTimeDisplayMode!=0 ? nowTime-previousTimeDisplayMode: DISPLAY_MODE_REFRESH_PERIOD;
  gapHourSampleTime = previousHourSampleTime!=0 ? nowTime-previousHourSampleTime: SAMPLE_T_LAST_HOUR*1000;
  gapDaySampleTime = previousDaySampleTime!=0 ? nowTime-previousDaySampleTime: SAMPLE_T_LAST_DAY*1000;
  
  //Checking if sample buffers update is needed
  if (gapHourSampleTime>=SAMPLE_T_LAST_HOUR*1000) {previousHourSampleTime=nowTime;gapHourSampleTime=0;updateHourSample=true;updateHourGraph=true;}
  else updateHourSample=false;

  if (gapDaySampleTime>=SAMPLE_T_LAST_DAY*1000) {previousDaySampleTime=nowTime;gapDaySampleTime=0;updateDaySample=true;updateDayGraph=true;}
  else updateDaySample=false;

  //Actions if button1 is pushed. It depens on the current state
  if (button1.pressed())
  {
    //Actions are different based on the current state
    switch(currentState) {
      case bootupScreen:
      break;
      case menuGlobal:
      //Changing Menus: Global Menu -> What to display Menu -> Display Gra. Info -> back
        switch(stateSelected) {
          case menuWhatToDisplay:
            stateSelected=displayInfo;
          break;
          case displayInfo:
            stateSelected=lastState;
          break;
          default:
            stateSelected=menuWhatToDisplay;
          break;
        }
        printGlobalMenu();
      break;
      case menuWhatToDisplay:
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
        printMenuWhatToDisplay();
      break;
      case displayInfo:
      break;
      default:
        lastState=currentState;
        stateSelected=menuWhatToDisplay;
        currentState=menuGlobal;
        printGlobalMenu();
      break;
    }
  }

  //Actions if button2 is pushed. It depens on the current state
  if (button2.pressed()) {
    //Actions are different based on the current state
    switch(currentState) {
      case menuGlobal:
        currentState=stateSelected;
        if (currentState==menuWhatToDisplay) {stateSelected=lastState; printMenuWhatToDisplay();}
        else if (currentState==displayInfo) printInfoScreen();
      break;
      case menuWhatToDisplay:
        currentState=stateSelected;
        gapTimeDisplay=DISPLAY_REFRESH_PERIOD;
        gapTimeDisplayMode=DISPLAY_MODE_REFRESH_PERIOD;
        lastDisplayMode=menu;
        lastGapTime=SAMPLE_PERIOD;
        //tft.fillScreen(MENU_BACK_COLOR);
      break;
      case displayInfo:
        currentState=menuGlobal;
        stateSelected=displayInfo;
        printGlobalMenu();
      break;
      default:
      break;
    }
  }

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
    tempMeasure=tempHumSensor.getTemperature();
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
          drawText(valueT, String(valueString),TEXT_SIZE,TEXT_FONT,TFT_GREEN,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),25,20,TFT_BLUE,27,TFT_RED);
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