#include "global_setup.h"
#include <WiFi.h>
#include "ButtonChecks.h"

extern boolean debugModeOn,webResuming,forceWEBCheck,forceWEBTestCheck;
extern RTC_DATA_ATTR CloudSyncStatus CloudSyncCurrentStatus; //Struct to store WiFi parameters
extern RTC_DATA_ATTR int errorsSampleUpts,webServerError1,webServerError2,webServerError3;
extern struct tm nowTimeInfo;

String IpAddress2String(const IPAddress& ipAddress);
uint8_t sendHttpRequest (boolean debugModeOn, IPAddress server, uint16_t port, String httpRequest);
uint32_t sendAsyncHttpRequest (boolean debugModeOn, boolean fromSetup, uint32_t error_setup, IPAddress server, uint16_t port, String httpRequest, uint64_t* whileLoopTimeLeft);
uint32_t checkURL(boolean debugModeOn,boolean fromSetup,uint32_t error_setup,IPAddress server,uint16_t port,String httpRequest);