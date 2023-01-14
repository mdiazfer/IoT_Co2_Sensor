#include "global_setup.h"
#include <WiFi.h>
#include "ButtonChecks.h"

extern boolean logsOn,webResuming,forceWEBCheck,forceWEBTestCheck;
extern RTC_DATA_ATTR CloudSyncStatus CloudSyncCurrentStatus; //Struct to store WiFi parameters
extern RTC_DATA_ATTR int errorsSampleUpts;

String IpAddress2String(const IPAddress& ipAddress);
uint8_t sendHttpRequest (boolean logsOn, IPAddress server, uint16_t port, String httpRequest);
uint8_t sendAsyncHttpRequest (boolean logsOn, boolean fromSetup, uint8_t error_setup, IPAddress server, uint16_t port, String httpRequest, uint64_t* whileLoopTimeLeft);