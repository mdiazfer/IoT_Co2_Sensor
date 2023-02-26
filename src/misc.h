#include <Arduino.h>
#include <WiFi.h>
#include "global_setup.h"
#include "battery.h"

extern RTC_DATA_ATTR boolean debugModeOn;
extern RTC_DATA_ATTR uint64_t loopEndTime,loopStartTime,sleepTimer;
extern RTC_DATA_ATTR float_t batCharge;
extern RTC_DATA_ATTR struct tm startTimeInfo;

void go_to_hibernate();
void go_to_sleep();
String roundFloattoString(float_t number, uint8_t decimals);
uint16_t checkSum(byte *addr, uint32_t count);