#include "mqttClient.h"

void onMqttConnect(bool sessionPresent) {
  if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttConnect] - MQTT connected to "+mqttServer+". Session present: "+String(sessionPresent)+
                        "\n                         Subscribing on topic "+MQTT_TOPIC_SUBSCRIPTION+", QoS 0, packetId="+String(mqttClient.subscribe(MQTT_TOPIC_SUBSCRIPTION,0)));
  MqttSyncCurrentStatus=MqttSyncOnStatus;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttDisconnect] - MQTT disconnected.");
  MqttSyncCurrentStatus=MqttSyncOffStatus;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttSubscribe] - MQTT subscribe acknowledged. packetId="+String(packetId)+", qos="+String(qos));
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttMessage] - MQTT publish received. topic="+String(topic)+", mesage: "+String(payload)+", index: "+String(index));
  if (String(topic)!=String(MQTT_TOPIC_SUBSCRIPTION)) {
    if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttMessage] - Topic received ("+String(topic)+") and expected ("+String(MQTT_TOPIC_SUBSCRIPTION)+" ) differs. Return");
    return;
  }

  //Pubish device name
  mqttClient.publish(String(mqttTopicPrefix+"device").c_str(), 0, true, String("co2-sensor").c_str());
  mqttClient.publish(String(mqttTopicPrefix+"device-name").c_str(), 0, true, device.c_str());
}

void onMqttUnsubscribe(uint16_t packetId) {
  if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttUnsubscribe] - MQTT unsubscribe acknowledged. packetId="+String(packetId));
}

void onMqttPublish(uint16_t packetId) {
  if (debugModeOn) Serial.println(String(loopStartTime+millis())+"  [onMqttPublish] - MQTT publish acknowledged. packetId="+String(packetId));
}

uint32_t mqttClientInit(bool fromSetup, bool debugModeOn, bool TFTDisplayLogs) {
  uint32_t errorMqtt=NO_ERROR;
  MqttSyncCurrentStatus=MqttSyncOffStatus;

  if (debugModeOn && fromSetup) {Serial.print("[setup] - MQTT: ");}
  if (TFTDisplayLogs) {stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[setup] - MQTT: ");}
  
  //Connect to MQTT Server
  uint8_t ip[4];
  if (sscanf(mqttServer.c_str(), "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]) ==4) {
    //FQDN
    mqttClient.setServer(mqttServer.c_str(), MQTTSERVER_PORT); //init variable 
  }
  else {
    //IP Address
    mqttClient.setServer(stringToIPAddress(mqttServer), MQTTSERVER_PORT); //init variable
  }

  // If secure MQTT is enabled
  if (secureMqttEnabled) {
    mqttClient.setCredentials(mqttUserName.c_str(),mqttUserPssw.c_str()); //init variable
  }

  if ((error_setup & ERROR_WIFI_SETUP)==0 && wifiEnabled && mqttServerEnabled) { 
    //Connect to MQTT broker
    mqttClient.connect();
    
    //Wait for the MQTT client to get connected or timeout (5sg), whatever happens first
    ulong now=millis();
    bool buttonPressed=false;
    //while (!mqttClient.connected() && (millis()<=now+5000) && !buttonPressed && !isBeaconAdvertising) {
    while (!mqttClient.connected() && (millis()<=now+5000) && !buttonPressed) { //v1.5.1 - No confict with BLE Advertising
      if (!fromSetup) { //v1.4.1 - Abort if buttons are pressed. This avoids config menu gets frozen
        switch (checkButtonsActions(mqttcheck)) { 
          case 1:
          case 2:
          case 3:
            //Button1 or Button2 pressed or released. MQTT Connect process aborted
            if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - checkButtonsActions() returns 1, 2 or 3 - Stop MQTT connection");}
            buttonPressed=true;
          break;
          case 0:
          default:
            //Regular exit. Do nothing else
          break;
        }
      }
    }
    if (mqttClient.connected()) MqttSyncCurrentStatus=MqttSyncOnStatus;

    if (MqttSyncCurrentStatus==MqttSyncOnStatus) {
      if (debugModeOn && fromSetup) {Serial.println("[OK]");}
      if (TFTDisplayLogs) {
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
        stext1.setTextColor(TFT_GREEN_4_BITS_PALETTE,TFT_BLACK); stext1.print("OK");
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
        stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SRV: ");
        stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);
      }
    }
    else
    {
      if (debugModeOn && fromSetup) {Serial.println("[KO]");}
      if (TFTDisplayLogs) {
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
        stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("KO");
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
        stext1.setCursor(0,(pLL-1)*pixelsPerLine);stext1.setTextColor(TFT_DARKGREY_4_BITS_PALETTE,TFT_BLACK);stext1.print("  SRV: ");
        stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK);
      }
      errorMqtt=ERROR_MQTT_SERVER;
    }

    if (debugModeOn && fromSetup) {
      Serial.println("  - SRV: "+mqttServer);
      Serial.println("  - Root Topic Name="+mqttTopicName);
    }

    if (TFTDisplayLogs) {stext1.print(mqttServer);if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine); }
  }
  else 
  {
    if ((error_setup & ERROR_WIFI_SETUP)!=0 || !wifiEnabled) {
      if (debugModeOn && fromSetup) {Serial.println("No WiFi");}
      if (TFTDisplayLogs) {
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
        stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("No WiFi");
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
    }
    else { 
      if (debugModeOn && fromSetup) {Serial.println("N/E");}
      if (TFTDisplayLogs) {
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK);stext1.print("[");
        stext1.setTextColor(TFT_RED_4_BITS_PALETTE,TFT_BLACK); stext1.print("N/E");
        stext1.setTextColor(TFT_YELLOW_4_BITS_PALETTE,TFT_BLACK); stext1.println("] ");if (pLL-1<scLL) pLL++; else {stext1.scroll(0,-pixelsPerLine);if (pFL>spFL) pFL--;}stext1.pushSprite(0, (scL-spL)/2*pixelsPerLine);
      }
    }
  }

  return errorMqtt;
}