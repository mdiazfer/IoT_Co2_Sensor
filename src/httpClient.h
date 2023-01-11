#include "global_setup.h"
#include <WiFi.h>

extern boolean logsOn;

#ifndef _WIFINETWORKINFO_
  enum CloudSyncStatus {CloudSyncOnStatus,CloudSyncOffStatus} ;
#endif
extern RTC_DATA_ATTR CloudSyncStatus CloudSyncCurrentStatus; //Struct to store WiFi parameters
extern RTC_DATA_ATTR int errorsSampleUpts;

String IpAddress2String(const IPAddress& ipAddress);
uint8_t sendHttpRequest (boolean logsOn, IPAddress server, uint16_t port, String httpRequest);