/* Library for displaying graphs in the TFT

*/

#include <Arduino.h>
#include <TFT_eSPI.h>

#ifndef _DECLAREGLOBALPARAMETERS_
  #include "global_setup.h"
  
  #ifndef _DISPLAYSUPPORTINFO_
    enum availableStates {bootupScreen,menuGlobal,menuWhatToDisplay,displayInfo,displayInfo1,displayInfo2,displayInfo3,displayInfo4,displayingSampleFixed,displayingCo2LastHourGraphFixed,
                          displayingCo2LastDayGraphFixed,displayingSequential};
    
    #define _DISPLAYSUPPORTINFO_
  #endif
#endif

extern TFT_eSPI tft;
extern enum availableStates stateSelected;
extern enum availableStates currentState;

class HorizontalBar {
  public:
    int32_t xStart, yStart, xEnd, yEnd, width, high, arrowHigh;
    int32_t xValue, xValueMin, xValueMax, xTh1, xTh2;
    float_t value, valueMin, valueMax, valueRange, th1, th2;
    uint32_t normalColor, coldColor, hotColor, colorBar, colorBackground;

    HorizontalBar(float_t value, float_t valueMin, float_t valueMax, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color,
            float_t th1, uint32_t color1, float_t th2, uint32_t color2, uint32_t colorBar, uint32_t colorBackground);
    void drawHorizontalBar();
    void drawHorizontalBar(float_t value);
    void cleanHorizontalBar();

};

class CircularGauge {
  private:
    //Backwards compatibility variables
    float_t angleValue=value*angleRange/valueRange+angleStart,
            angleTh1=th1*angleRange/valueRange+angleStart,
            angleTh2=th2*angleRange/valueRange+angleStart;
    int32_t x, y;
    
  public:
    int32_t xCenter, yCenter, rExt, rInt, width, xStart, yStart, xEnd, yEnd, rDeltaMark=5;
    float_t value, valueMin, valueMax, valueRange, th1, th2;
    float_t valueAngle, valueMinAngle, valueMaxAngle, th1Angle, th2Angle, segmentAngle;
    float_t angleStart=90+segmentAngle,angleEnd=450-segmentAngle,angleRange=360-2*segmentAngle;
    uint32_t normalColor, warningColor, alarmColor, colorGauge, colorBackground, colorText, colorValue;
    String valueString, unitsString;
    int32_t xStartValueText=0, yStartValueText=0, xEndValueText=0, yEndValueText=0;
    int32_t xStartUnitsText=0, yStartUnitsText=0, xEndUnitsText=0, yEndUnitsText=0;

    CircularGauge(float_t value, float_t valueMin, float_t valueMax, int32_t x, int32_t y, int32_t r, int32_t width, float_t segment, uint32_t color,
                float_t th1, uint32_t color1, float_t th2, uint32_t color2, uint32_t colorGauge, uint32_t colorBackground);
    void setValue(float_t value);
    void drawGauge();
    void drawGauge(float_t value);
    void drawGauge1();
    void drawGauge1(float_t value);
    void drawGauge2();
    void drawGauge2(float_t value);
    void drawTextGauge(String unitsString, uint32_t valueTextSize=TEXT_SIZE, boolean isInteger=true,
                      uint32_t unitsTextSize=TEXT_SIZE_UNITS_CO2, uint32_t valueTextFont=TEXT_FONT,
                      uint32_t unitsTextFont=TEXT_FONT_UNITS_CO2, uint32_t colorText=0);
    void cleanGauge();
    void cleanValueTextGauge();
    void cleanUnitsTextGauge();
    void cleanAll();
};

void drawText(float_t value, String textString, int32_t textSize, int32_t font, uint32_t colorForeground,
              uint32_t colorBackground, int32_t x, int32_t y, 
              float_t th1, uint32_t warningColor, float_t th2, uint32_t alarmColor);


