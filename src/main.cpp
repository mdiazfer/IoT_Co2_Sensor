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

#ifdef __MHZ19B__
  const ulong co2PreheatingTime=MH_Z19B_CO2_WARMING_TIME;
#endif

//Global variable definitions
const String co2SensorType=String(CO2_SENSOR_TYPE);
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
float_t valueCO2,valueT,valueHum,lastValueCO2=-1,tempMeasure;
String valueString;
float_t lastHourCo2Samples[3600/SAMPLE_T_LAST_HOUR];   //Circular buffer to record last-hour C02 values
float_t lastDayCo2Samples[24*3600/SAMPLE_T_LAST_DAY]; //Circular buffer to record last-day C02 values
boolean showGraph=false,updateHourSample=true,updateDaySample=true;
int8_t counterDisplay=-1;
enum displayModes {bootup,sampleValue,co2LastHourGraph,co2LastDayGraph};
enum displayModes displayMode=sampleValue, lastDisplayMode=bootup; //Will make starting always in sampleValue

SoftwareSerial co2SensorSerialPort(CO2_SENSOR_RX, CO2_SENSOR_TX);
#ifdef __MHZ19B__
  MHZ19 co2Sensor;
  extern const int MHZ19B;
#endif

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
  for (int i=1;i<=4;i++) tft.drawFastVLine(CO2_GRAPH_X+45*i,CO2_GRAPH_Y_END-5,10,TFT_DARKGREY);
  for (int i=0;i<=3;i++) tft.drawFastHLine(CO2_GRAPH_X-5,CO2_GRAPH_Y+25*i,10,TFT_DARKGREY);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(CO2_GRAPH_X+35,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-45");
  tft.setCursor(CO2_GRAPH_X+80,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-35");
  tft.setCursor(CO2_GRAPH_X+125,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-15");
  tft.setCursor(CO2_GRAPH_X+170,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("Now");
  tft.setCursor(CO2_GRAPH_X+190,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("t(m)");
  tft.setCursor(CO2_GRAPH_X+30,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("CO2 in the last 60 min (ppm)");
  tft.setCursor(CO2_GRAPH_X-12,CO2_GRAPH_Y+5,TEXT_FONT_BOOT_SCREEN-1);tft.print("2k");
  tft.setCursor(CO2_GRAPH_X-20,CO2_GRAPH_Y+25,TEXT_FONT_BOOT_SCREEN-1);tft.print("1k5");
  tft.setCursor(CO2_GRAPH_X-12,CO2_GRAPH_Y+50,TEXT_FONT_BOOT_SCREEN-1);tft.print("1k");
  tft.setCursor(CO2_GRAPH_X-20,CO2_GRAPH_Y+75,TEXT_FONT_BOOT_SCREEN-1);tft.print("500");

  //Draw samples
  int32_t xSample, auxColor;
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)
  {
    xSample=(int32_t) (CO2_GRAPH_Y_END-lastHourCo2Samples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX);
    if (lastHourCo2Samples[i]<=CO2_GAUGE_TH1) auxColor=TFT_GREEN;
    else if (CO2_GAUGE_TH1 < lastHourCo2Samples[i] && lastHourCo2Samples[i] <= CO2_GAUGE_TH2) auxColor=TFT_YELLOW;
    else auxColor=TFT_RED;
    if (xSample==CO2_GRAPH_Y_END) auxColor=TFT_DARKGREY;
    tft.drawPixel(i+CO2_GRAPH_X,xSample,auxColor);
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
  for (int i=1;i<=4;i++) tft.drawFastVLine(CO2_GRAPH_X+48*i,CO2_GRAPH_Y_END-5,10,TFT_DARKGREY);
  for (int i=0;i<=3;i++) tft.drawFastHLine(CO2_GRAPH_X-5,CO2_GRAPH_Y+25*i,10,TFT_DARKGREY);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(CO2_GRAPH_X+38,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-18");
  tft.setCursor(CO2_GRAPH_X+86,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-12");
  tft.setCursor(CO2_GRAPH_X+134,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-6");
  tft.setCursor(CO2_GRAPH_X+165,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("Now t(h)");
  tft.setCursor(CO2_GRAPH_X+30,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("CO2 in the last 24 h. (ppm)");
  tft.setCursor(CO2_GRAPH_X-12,CO2_GRAPH_Y+5,TEXT_FONT_BOOT_SCREEN-1);tft.print("2k");
  tft.setCursor(CO2_GRAPH_X-20,CO2_GRAPH_Y+25,TEXT_FONT_BOOT_SCREEN-1);tft.print("1k5");
  tft.setCursor(CO2_GRAPH_X-12,CO2_GRAPH_Y+50,TEXT_FONT_BOOT_SCREEN-1);tft.print("1k");
  tft.setCursor(CO2_GRAPH_X-20,CO2_GRAPH_Y+75,TEXT_FONT_BOOT_SCREEN-1);tft.print("500");

  //Draw samples
  int32_t xSample, auxColor;
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++)
  {
    xSample=(int32_t) (CO2_GRAPH_Y_END-lastDayCo2Samples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX);
    if (lastDayCo2Samples[i]<=CO2_GAUGE_TH1) auxColor=TFT_GREEN;
    else if (CO2_GAUGE_TH1 < lastDayCo2Samples[i] && lastDayCo2Samples[i] <= CO2_GAUGE_TH2) auxColor=TFT_YELLOW;
    else auxColor=TFT_RED;
    if (xSample==CO2_GRAPH_Y_END) auxColor=TFT_DARKGREY;
    tft.drawPixel(i+CO2_GRAPH_X,xSample,auxColor);
  }
}

void setup() {
  static uint32_t wr = 1;
  static uint32_t rd = 0xFFFFFFFF;

  if (logsOn) {Serial.begin(115200);Serial.println("\n[setup] - Serial: OK");}

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
  tft.setTextColor(TFT_WHITE,TFT_BLACK); tft.println("IoT boot up...............");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Display: [");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");

  //Sensor init
  if (logsOn) Serial.print("[setup] - Sensor: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Sens.:  [");
  co2SensorSerialPort.begin(9600);      // (Uno example) device to MH-Z19 serial start   
  co2Sensor.begin(co2SensorSerialPort); // *Serial(Stream) refence must be passed to library begin(). 
  co2Sensor.setRange(CO2_SENSOR_CO2_RANGE);             // It's aviced to setup range to 2000. Better accuracy

  //Some sensor checks
  char co2SensorVersion[5];memset(co2SensorVersion, '\0', 5);
  co2Sensor.getVersion(co2SensorVersion);
  if (CO2_SENSOR_CO2_RANGE!=co2Sensor.getRange() || (byte) 0 != co2Sensor.getAccuracy(false) ||
      0==co2SensorType.compareTo("UNKNOW"))
    error_setup = ERROR_SENSOR_SETUP;
  if (error_setup != ERROR_SENSOR_SETUP ) { 
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
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.println("  Can't continue. STOP");
    return;
  }
  
  /*-->
  boolean positiveSign;
  float_t markUp;
  randomSeed(analogRead(0));
  <--*/
  
  //Initiating circula buffer to draw the Co2 graph
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++)  lastHourCo2Samples[i]=0;
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++) lastDayCo2Samples[i]=0;
  /*-->{
    //Ramdom filling for testing
    positiveSign=random(0,4)>=3?true:false;markUp=(float_t)random(0,5)/100;
    if (i==0) lastDayCo2Samples[i]=1800;
    else
      if (positiveSign) lastDayCo2Samples[i]=lastDayCo2Samples[i-1]*(1+markUp);
      else lastDayCo2Samples[i]=lastDayCo2Samples[i-1]*(1-markUp);
    Serial.print("\n- i=");Serial.print(i);Serial.print(", positiveSign=");Serial.print(positiveSign);
    Serial.print(", markUp=");Serial.print(markUp);
    Serial.print(", lastDayCo2Samples[");Serial.print(i);Serial.print("]=");Serial.print(lastDayCo2Samples[i]);
  }<--*/

  //-->>Buttons init
  if (logsOn) Serial.print("[setup] - Buttons: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Buttons: [");
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
  /*-->*/randomSeed(analogRead(0));
  circularGauge.drawGauge2(0);
}

void loop() {
  if (ERROR_DISPLAY_SETUP==error_setup || ERROR_SENSOR_SETUP==error_setup)
    return;

  nowTime=millis();

  while (nowTime<co2PreheatingTime) {
    //Waiting for the sensor to warmup before displaying value
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
  
  //Checking if circular buffers update is needed
  if (gapHourSampleTime>=SAMPLE_T_LAST_HOUR*1000) {previousHourSampleTime=nowTime;gapHourSampleTime=0;updateHourSample=true;}
  else updateHourSample=false;

  if (gapDaySampleTime>=SAMPLE_T_LAST_DAY*1000) {previousDaySampleTime=nowTime;gapDaySampleTime=0;updateDaySample=true;}
  else updateDaySample=false;

  //Regular actions every SAMPLE_PERIOD seconds
  if (gapTime>=SAMPLE_PERIOD) {
    previousTime=nowTime;lastGapTime=gapTime;gapTime=0;

    //Getting CO2 value
    /*-->valueCO2=(float_t)random(0,2000);<--*/
    valueCO2=(float_t)co2Sensor.getCO2();

    //Updating the last hour Co2 buffer
    if (updateHourSample) {
      for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR)-1; i++) lastHourCo2Samples[i]=lastHourCo2Samples[i+1];
      lastHourCo2Samples[(int)(3600/SAMPLE_T_LAST_HOUR)-1]=valueCO2;
    }
    
    //Updating the last day Co2 buffer
    if (updateDaySample) {
      for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY)-1; i++) lastDayCo2Samples[i]=lastDayCo2Samples[i+1];
      lastDayCo2Samples[(int)(24*3600/SAMPLE_T_LAST_DAY)-1]=valueCO2;
    }
  }

  //Regular actions every DISPLAY_MODE_REFRESH_PERIOD seconds
  if (gapTimeDisplayMode>=DISPLAY_MODE_REFRESH_PERIOD) {
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
  if (gapTimeDisplay>=DISPLAY_REFRESH_PERIOD) {
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
            tft.fillRect(TFT_X_WIDTH-tft.textWidth(String((uint32_t) valueHum)+"%"),90+tft.fontHeight(TEXT_FONT),tft.textWidth(String((uint32_t) valueHum)+"%"),tft.fontHeight(TEXT_FONT),TFT_BLACK);
            
            //Partial displaying of the circular gauge (only the text value) to don't take too much displaying it
            circularGauge.setValue(valueCO2);
            circularGauge.cleanValueTextGauge();
            circularGauge.cleanUnitsTextGauge();
          }
          circularGauge.drawTextGauge("ppm");

          //Drawing temperature
          /*-->valueT=(float_t)random(0,600)/10-10.0;*/
          tempMeasure=co2Sensor.getTemperature(true,true);
          if (tempMeasure>-50.0) valueT=tempMeasure;  //Discarding potential wrong values
          valueString=String((int) valueT)+"."+String(abs(((int) (valueT*10))-(((int)valueT)*10)))+"C";
          tft.setTextSize(TEXT_SIZE);
          drawText(valueT, String(valueString),TEXT_SIZE,TEXT_FONT,TFT_GREEN,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),25,20,TFT_BLUE,27,TFT_RED);
          horizontalBar.drawHorizontalBar(valueT);

          //Drawing Humidity
          valueHum=(float_t)random(0,100);
          valueString=String("Hum.");
          tft.setTextSize(TEXT_SIZE);
          drawText(valueHum,String(valueString),TEXT_SIZE,TEXT_FONT,TFT_MAGENTA,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),90,30,TFT_MAGENTA,55,TFT_MAGENTA);
          //valueString=String((uint32_t) valueHum)+"%";
          valueString=String("----");
          drawText(valueHum,String(valueString),TEXT_SIZE,TEXT_FONT,TFT_MAGENTA,TFT_BLACK,TFT_X_WIDTH-tft.textWidth(String(valueString)),90+tft.fontHeight(TEXT_FONT),30,TFT_BROWN,55,TFT_RED);
        
        }
        lastDisplayMode=sampleValue;
      break;
      case co2LastHourGraph:
        //Last Hour graph is displayed
        
        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastHourGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastHourCo2();}
        else if (updateHourSample) drawGraphLastHourCo2(); //Draw new graph only if buffer was updated

        lastDisplayMode=co2LastHourGraph;
      break;
      case co2LastDayGraph:
        //Last Day graph is displayed

        //Cleaning the screen always the first time in
        if (lastDisplayMode!=co2LastDayGraph) {tft.fillScreen(TFT_BLACK); drawGraphLastDayCo2();}
        else if (updateDaySample) drawGraphLastDayCo2(); //Draw new graph only if buffer was updated
      
        lastDisplayMode=co2LastDayGraph;
      break;
    }
  }
}