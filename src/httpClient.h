#include "global_setup.h"
#include <WiFi.h>

extern boolean logsOn;

String IpAddress2String(const IPAddress& ipAddress);
uint8_t sendHttpRequest (IPAddress server, uint16_t port, String httpRequest);