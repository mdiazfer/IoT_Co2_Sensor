/* Dealing with Battery stuff

*/

#include <Arduino.h>
#include "battery.h"


double_t chargeToVoltArray[101];

void initVoltageArray() {
  chargeToVoltArray[0] = 3.200/2; 
  chargeToVoltArray[1] = 3.250/2; chargeToVoltArray[2] = 3.300/2; chargeToVoltArray[3] = 3.350/2; chargeToVoltArray[4] = 3.400/2; chargeToVoltArray[5] = 3.450/2;
  chargeToVoltArray[6] = 3.500/2; chargeToVoltArray[7] = 3.550/2; chargeToVoltArray[8] = 3.600/2; chargeToVoltArray[9] = 3.650/2; chargeToVoltArray[10] = 3.700/2;
  chargeToVoltArray[11] = 3.703/2; chargeToVoltArray[12] = 3.706/2; chargeToVoltArray[13] = 3.710/2; chargeToVoltArray[14] = 3.713/2; chargeToVoltArray[15] = 3.716/2;
  chargeToVoltArray[16] = 3.719/2; chargeToVoltArray[17] = 3.723/2; chargeToVoltArray[18] = 3.726/2; chargeToVoltArray[19] = 3.729/2; chargeToVoltArray[20] = 3.732/2;
  chargeToVoltArray[21] = 3.735/2; chargeToVoltArray[22] = 3.739/2; chargeToVoltArray[23] = 3.742/2; chargeToVoltArray[24] = 3.745/2; chargeToVoltArray[25] = 3.748/2;
  chargeToVoltArray[26] = 3.752/2; chargeToVoltArray[27] = 3.755/2; chargeToVoltArray[28] = 3.758/2; chargeToVoltArray[29] = 3.761/2; chargeToVoltArray[30] = 3.765/2;
  chargeToVoltArray[31] = 3.768/2; chargeToVoltArray[32] = 3.771/2; chargeToVoltArray[33] = 3.774/2; chargeToVoltArray[34] = 3.777/2; chargeToVoltArray[35] = 3.781/2;
  chargeToVoltArray[36] = 3.784/2; chargeToVoltArray[37] = 3.787/2; chargeToVoltArray[38] = 3.790/2; chargeToVoltArray[39] = 3.794/2; chargeToVoltArray[40] = 3.797/2;
  chargeToVoltArray[41] = 3.800/2; chargeToVoltArray[42] = 3.805/2; chargeToVoltArray[43] = 3.811/2; chargeToVoltArray[44] = 3.816/2; chargeToVoltArray[45] = 3.821/2;
  chargeToVoltArray[46] = 3.826/2; chargeToVoltArray[47] = 3.832/2; chargeToVoltArray[48] = 3.837/2; chargeToVoltArray[49] = 3.842/2; chargeToVoltArray[50] = 3.847/2;
  chargeToVoltArray[51] = 3.853/2; chargeToVoltArray[52] = 3.858/2; chargeToVoltArray[53] = 3.863/2; chargeToVoltArray[54] = 3.868/2; chargeToVoltArray[55] = 3.874/2;
  chargeToVoltArray[56] = 3.879/2; chargeToVoltArray[57] = 3.884/2; chargeToVoltArray[58] = 3.889/2; chargeToVoltArray[59] = 3.895/2; chargeToVoltArray[60] = 3.900/2;
  chargeToVoltArray[61] = 3.906/2; chargeToVoltArray[62] = 3.911/2; chargeToVoltArray[63] = 3.917/2; chargeToVoltArray[64] = 3.922/2; chargeToVoltArray[65] = 3.928/2;
  chargeToVoltArray[66] = 3.933/2; chargeToVoltArray[67] = 3.939/2; chargeToVoltArray[68] = 3.944/2; chargeToVoltArray[69] = 3.950/2; chargeToVoltArray[70] = 3.956/2;
  chargeToVoltArray[71] = 3.961/2; chargeToVoltArray[72] = 3.967/2; chargeToVoltArray[73] = 3.972/2; chargeToVoltArray[74] = 3.978/2; chargeToVoltArray[75] = 3.983/2;
  chargeToVoltArray[76] = 3.989/2; chargeToVoltArray[77] = 3.994/2; chargeToVoltArray[78] = 4.000/2; chargeToVoltArray[79] = 4.008/2; chargeToVoltArray[80] = 4.015/2;
  chargeToVoltArray[81] = 4.023/2; chargeToVoltArray[82] = 4.031/2; chargeToVoltArray[83] = 4.038/2; chargeToVoltArray[84] = 4.046/2; chargeToVoltArray[85] = 4.054/2;
  chargeToVoltArray[86] = 4.062/2; chargeToVoltArray[87] = 4.069/2; chargeToVoltArray[88] = 4.077/2; chargeToVoltArray[89] = 4.085/2; chargeToVoltArray[90] = 4.092/2;
  chargeToVoltArray[91] = 4.100/2; chargeToVoltArray[92] = 4.111/2; chargeToVoltArray[93] = 4.122/2; chargeToVoltArray[94] = 4.133/2; chargeToVoltArray[95] = 4.144/2;
  chargeToVoltArray[96] = 4.156/2; chargeToVoltArray[97] = 4.167/2; chargeToVoltArray[98] = 4.178/2; chargeToVoltArray[99] = 4.189/2; chargeToVoltArray[100] = 4.200/2;
}

float_t getChargePercentage(double_t milliVolts) {
  uint8_t index=50, prev=0, half=0;
  
  if (milliVolts >= 4.2/2){
    return 100;
  }
  if (milliVolts <= 3.2/2){
    return 0;
  }
  while(true){
    half = abs(index - prev) / 2;
    prev = index;
    if(milliVolts >= chargeToVoltArray[index]){
      index = index + half;
    }else{
      index = index - half;
    }
    if (prev == index){
      break;
    }
  }
  return (float_t) index;
}

enum batteryChargingStatus getBatteryStatus(float_t batADCVolt, ulong timeLast) {
  enum batteryChargingStatus batteryStatus=batteryCharging000;

  //Take batCharge based on powerState
  switch(powerState) {
    case(off):
      //It's suppossed this case is no possible
    break;
    case(chargingUSB):
      batCharge=lastBatCharge+100*float_t (timeLast)/FULL_CHARGE_TIME; //lastBatCharge=last bat charge in onlyBAT mode
      if (batCharge>100) batCharge=100;
      if (batCharge<0) batCharge=0;
      if (batCharge>87) batteryStatus=batteryCharging100;
      else if (batCharge>62) batteryStatus=batteryCharging075;
      else if (batCharge>37) batteryStatus=batteryCharging050;
      else if (batCharge>12) batteryStatus=batteryCharging025;
      else if (batCharge>6) batteryStatus=batteryCharging010;
      else batteryStatus=batteryCharging000;
    break;
    case(onlyBattery):
      //batCharge=((batADCVolt-BAT_ADC_MIN)/(BAT_ADC_MAX-BAT_ADC_MIN))*100;
      batCharge=getChargePercentage((double_t) (batADCVolt/1000));
      if (batCharge>100) batCharge=100;
      if (batCharge<0) batCharge=0;
      lastBatCharge=batCharge;
      if (batCharge>87) batteryStatus=battery100;
      else if (batCharge>62) batteryStatus=battery075;
      else if (batCharge>37) batteryStatus=battery050;
      else if (batCharge>12) batteryStatus=battery025;
      else if (batCharge>6) batteryStatus=battery010;
      else batteryStatus=battery000;
    break;
    case(noChargingUSB):
      batCharge=100;
      batteryStatus=batteryCharging100;
    break;
  }

  return batteryStatus;
}

void updateBatteryVoltageAndStatus(ulong nowTime, ulong *timeUSBPower) {
  //batADCVolt update
  //Power state check and powerState update
  //batteryStatus update
  //If USB is plugged, timeUSBPower is updated with nowTime, to estimate batteryCharge based on time
  //If USB is unplugged, timeUSBPower is set to zero

  //Power state check
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
    batADCVolt=0; for (u_int8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
    digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume
    
    if (batADCVolt >= VOLTAGE_TH_STATE) {
      //USB is plugged. Assume battery is always plugged and charged after FULL_CHARGE_TIME milliseconds
      if(noChargingUSB!=powerState) {
        powerState=chargingUSB;
        if ((nowTime - *timeUSBPower)>=FULL_CHARGE_TIME)
          powerState=noChargingUSB;
      }
      if (0 == *timeUSBPower) *timeUSBPower=nowTime;
    }
    else {
      powerState=onlyBattery;
      *timeUSBPower=0;
    }

    if (onlyBattery==powerState)
      //Take battery charge when the Battery is plugged
      batteryStatus=getBatteryStatus(batADCVolt,0);
    else
      //When USB is plugged, the Battery charge can be only guessed based on
      // the time the USB is being plugged 
      batteryStatus=getBatteryStatus(batADCVolt,nowTime - *timeUSBPower);
}