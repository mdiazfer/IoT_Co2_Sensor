#include "global_setup.h"
#include <WiFi.h>

extern boolean logsOn;

#ifndef _WIFINETWORKINFO_
  enum CloudSyncStatus {CloudSyncOnStatus,CloudSyncOffStatus} ;
#endif
extern RTC_DATA_ATTR CloudSyncStatus CloudSyncCurrentStatus; //Struct to store WiFi parameters

String IpAddress2String(const IPAddress& ipAddress);
uint8_t sendHttpRequest (IPAddress server, uint16_t port, String httpRequest);