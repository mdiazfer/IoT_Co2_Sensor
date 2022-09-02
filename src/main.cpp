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


uint8_t error_setup = NO_ERROR;

TFT_eSPI tft = TFT_eSPI();  // Invoke library to manage the display

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
  tft.setCursor(0,0,2);
  tft.setTextSize(1);
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
}

void loop() {
  // put your main code here, to run repeatedly:
}