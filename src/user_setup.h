/* 
  Definition of user configuration parameters
*/

#if BUILD_ENV_NAME==BUILD_TYPE_SENSOR_CASE_OFI
  #define WIFI_SSID_CREDENTIALS "ALE"
  #define WIFI_PW_CREDENTIALS "ALE@Madrid-1"
#endif
#ifndef WIFI_SSID_CREDENTIALS
  #define WIFI_SSID_CREDENTIALS "madnetot"
  #define WIFI_PW_CREDENTIALS "FC4B4C9F6E601E56451B8"
  #define WIFI_SSID_CREDENTIALS_BK1 "ALE"
  #define WIFI_PW_CREDENTIALS_BK1 "ALE@Madrid-1"
  #define WIFI_SSID_CREDENTIALS_BK2 "wlansix"
  #define WIFI_PW_CREDENTIALS_BK2 "madwep6wepmad"
#endif
#define MQTT_USER_CREDENTIAL "admin"
#define MQTT_PW_CREDENTIAL "adminCO2"