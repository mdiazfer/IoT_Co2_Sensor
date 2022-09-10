#ifndef WIFIFRAMEWORK
  #define WIFIFRAMEWORK
#endif

#ifndef _DECLAREGLOBALPARAMETERS_
  #define _DECLAREGLOBALPARAMETERS_
#endif

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
//#include <MHZ.h>
#include "global_setup.h"
#include "wifiConnection.h"
#include "display_support.h"
#include "MHZ19.h"
#include <SoftwareSerial.h>


uint8_t error_setup = NO_ERROR;
TFT_eSPI tft = TFT_eSPI();  // Invoke library to manage the display
ulong nowTime=0,previousTime=0,gapTime;
CircularGauge circularGauge=CircularGauge(610,0,2000,80,95,70,30,60,TFT_DARKGREEN,800,TFT_YELLOW,950,TFT_RED,TFT_DARKGREY,TFT_BLACK);
HorizontalBar horizontalBar=HorizontalBar(24.6,-10,50,145,50,95,10,TFT_GREEN,19,TFT_BLUE,27,TFT_RED,TFT_DARKGREY,TFT_BLACK);
float_t valueCO2,valueT,valueHum,lastValueCO2=-1,tempMeasure;
String valueString;
//MHZ co2(MH_Z19_RX, MH_Z19_TX, MHZ19B);

MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(MH_Z19_RX, MH_Z19_TX);

extern const int MHZ19B;

void loadBootImage() {
  //-->>Load the logo image when booting up

  return;
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
      exit;
    }
  }
  else {
    Serial.println("[setup] - Display: OK");
    tft.fillScreen(TFT_BLACK);
  }

  loadBootImage();
  delay(500);
  //Display messages
  tft.setCursor(0,0,TEXT_FONT_BOOT_SCREEN);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.setTextColor(TFT_WHITE,TFT_BLACK); tft.println("IoT boot up...............");
  //tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Serial:  [");
  //tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  //tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Display: [");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");

  //-->>Sensor init
  Serial.print("[setup] - Sensor: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Sensor:  [");
  
  //MHZ co2(MH_Z19_RX, MH_Z19_TX, CO2_IN, MHZ19B);
  /*if (co2.isPreHeating()) {
    Serial.print("Preheating");
    while (co2.isPreHeating()) {
      Serial.print(".");
      delay(5000);
    }
    Serial.println();
  }*/
  /*if (nullptr == ss) {error_setup = ERROR_SENSOR_SETUP; Serial.println("sensor error");}
  else ss->begin(9600);
  */
  mySerial.begin(9600);                               // (Uno example) device to MH-Z19 serial start   
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 

 if (error_setup != ERROR_SENSOR_SETUP ) { 
    Serial.println("OK");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  } else {
    Serial.println("KO"); Serial.println("Can't continue. STOP");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.println("Can't continue. STOP");
    return;
  }
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");

  //-->>Buttons init
  Serial.print("[setup] - Buttons: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - Buttons: [");
  if (error_setup != ERROR_BUTTONS_SETUP ) { 
    Serial.println("OK");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  } else {
    Serial.println("KO"); Serial.println("Can't continue. STOP");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.println("Can't continue. STOP");
    return;
  }
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");

  //-->>WiFi init
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - WiFi: ");
  error_setup=wifiConnect();
  Serial.print("[setup] - WiFi: ");
  if (error_setup != ERROR_WIFI_SETUP ) { 
    Serial.println("OK");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print(" [");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("] - ");
    tft.setTextColor(TFT_GREEN,TFT_BLACK);tft.print(wifiNet.ssid);tft.print(", ");tft.println(WiFi.localIP().toString());
  } else {
    Serial.println("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("   [");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
    tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");
  }
  
  //-->>BLE init
  Serial.print("[setup] - BLE: ");
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.print("[setup] - BLE:     [");
  if (error_setup != ERROR_BLE_SETUP ) { 
    Serial.println("OK");
    tft.setTextColor(TFT_GREEN,TFT_BLACK); tft.print("OK");
  } else {
    Serial.println("KO");
    tft.setTextColor(TFT_RED,TFT_BLACK); tft.print("KO");
  }
  tft.setTextColor(TFT_GOLD,TFT_BLACK); tft.println("]");


  if (error_setup != NO_ERROR) {
    Serial.println("Ready to start with limitations");
    tft.setTextColor(TFT_BROWN,TFT_BLACK); tft.println("Ready to start with limitations");
  } else {
    Serial.println("Ready to start");
    tft.setTextColor(TFT_DARKGREEN,TFT_BLACK); tft.println("Ready to start");
  }

  delay(2000);
  tft.setCursor(0,0,TEXT_FONT);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(TEXT_SIZE);
  randomSeed(analogRead(0));
}

void loop() {
  nowTime=millis();
  gapTime = previousTime!=0 ? nowTime-previousTime: VALUE_REFRESH_PERIOD;
  
  if (gapTime>=VALUE_REFRESH_PERIOD) {
    previousTime=nowTime;gapTime=0;

    tft.setTextSize(TEXT_SIZE);

    /*co2.setDebug(true);
    int ppm_uart = co2.readCO2UART();
    Serial.print("PPMuart: ");Serial.print(ppm_uart); 
    //if (ppm_uart > 0) Serial.print(ppm_uart); 
    //else Serial.print("n/a");

    int temperature = co2.getLastTemperature();
    Serial.print(", Temperature: ");Serial.println(temperature);
    //if (temperature > 0) Serial.println(temperature);
    //else Serial.println("n/a");
    */

    /*int CO2;
    CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
    Serial.print("CO2 (ppm): ");                      
    Serial.print(CO2);                                

    int8_t Temp;
    Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
    Serial.print(", Temperature (C): ");                  
    Serial.println(Temp);                               
    */

    //Cleaning & Drawing circular gauge
    //valueCO2=(float_t)random(0,2000);
    valueCO2=(float_t)myMHZ19.getCO2();
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
        //tft.fillRect(TFT_X_WIDTH-tft.textWidth(String((uint32_t) valueHum)+"%"),90+tft.fontHeight(TEXT_FONT),tft.textWidth(String((uint32_t) valueHum)+"%"),tft.fontHeight(TEXT_FONT),TFT_BLACK);
        circularGauge.cleanAll();
        
        //Drawing the circularGauge
        circularGauge.drawGauge2(valueCO2); lastValueCO2=valueCO2;
      }
    else 
      {
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
    //valueT=(float_t)random(0,600)/10-10.0;
    tempMeasure=myMHZ19.getTemperature(true,true);
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
  
}