#include <Arduino.h>
#include "global_setup.h"
#include <ESPAsyncWebServer.h>
#include <AsyncMqttClient.h>
#include <TFT_eSPI.h>
#include "misc.h"
#include "soc/rtc_wdt.h"

#ifndef _BUTTONSFRAMEWORK_
    enum callingAction {mainloop,askAPloop,ntpcheck,wificheck,webcheck,mqttcheck};
    #define _BUTTONSFRAMEWORK_
#endif

extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus;
extern RTC_DATA_ATTR boolean debugModeOn,wifiEnabled,mqttServerEnabled;
extern RTC_DATA_ATTR uint32_t error_setup;
extern bool isBeaconAdvertising;
extern String mqttServer;
extern TFT_eSprite stext1;
extern uint8_t pixelsPerLine,
    spL,            //Number of Lines in the Sprite
    scL,            //Number of Lines in the Scroll
    pFL,            //Pointer First Line
    pLL,            //pointer Last Line written
    spFL,           //Sprite First Line Window
    spLL,           //Sprite Last Line Window
    scFL,           //Scroll First Line Window
    scLL;           //Scroll Last Line Window

void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttPublish(uint16_t packetId);
uint32_t mqttClientInit(bool fromSetup, bool debugModeOn, bool TFTDisplayLogs);

extern uint8_t checkButtonsActions(enum callingAction fromAction);
