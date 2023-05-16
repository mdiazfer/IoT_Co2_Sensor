  /* Library for displaying graphs in the TFT

*/

#include "display_support.h"
#include "battery.h"
#include "icons.h"

HorizontalBar::HorizontalBar(float_t value, float_t valueMin, float_t valueMax, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color,
        float_t th1, uint32_t color1, float_t th2, uint32_t color2, uint32_t colorBar, uint32_t colorBackground) {
  /* Constructor of class HorizontalBar */

  HorizontalBar::arrowHigh=10; HorizontalBar::width=w; HorizontalBar::high=h;
  HorizontalBar::xStart=x; HorizontalBar::yStart=y; HorizontalBar::xEnd=x+w; HorizontalBar::yEnd=y+h+arrowHigh;
  HorizontalBar::value=value;
  HorizontalBar::valueMin=valueMin; HorizontalBar::valueMax=valueMax; HorizontalBar::valueRange=valueMax-valueMin;
  HorizontalBar::th1=th1; HorizontalBar::th2=th2;
  HorizontalBar::normalColor=color; HorizontalBar::coldColor=color1; HorizontalBar::hotColor=color2;
  HorizontalBar::colorBar=colorBar; HorizontalBar::colorBackground=colorBackground;
  HorizontalBar::xValueMin=xStart; HorizontalBar::xValueMax=xEnd;
  if (value>=valueMin && value<=valueMax) HorizontalBar::xValue=(value-valueMin)*w/(valueMax-valueMin);
  else if (value<valueMin) xValue=0;
  else if (value>valueMax) xValue=width;
  HorizontalBar::xTh1=(th1-valueMin)*w/(valueMax-valueMin);
  HorizontalBar::xTh2=(th2-valueMin)*w/(valueMax-valueMin);
};

void HorizontalBar::drawHorizontalBar() {
  /****************************************************************** 
    Function: drawHorizontalBar
    Definition: Draw a horizontal bar graph
    Parameters:
      value: value of the magnitude to be draw
      valueMin: value at which the bar graph starts
      valueMax: value at which the bar graph ends
      x: x coordinate of the bar begining
      y: y coordinate of the bar begining
      w: bar grapah width
      h: bar grapah high
      color: color for normal value (no waning, no alarm)
      th1: threshold value to enter in warning zone
      color1: color for warning values
      th2: threshold value to enter in alarm zone
      color2: color for alarm values
      colorBar: bar color (for zone > value)
      colorBackground: background color
  ******************************************************************/

  if (value<=th1) {
    tft.fillRect(xStart,yStart,xValue,high,coldColor);
    tft.fillRect(xStart+xValue,yStart,width-xValue,high,colorBar);
    tft.drawFastVLine(xStart+xTh1,yStart,high+7,normalColor);
    tft.drawFastVLine(xStart+xTh2,yStart,high+7,hotColor);
    tft.fillTriangle(xStart+xValue,yStart+high,xStart+xValue-arrowHigh,yStart+high+arrowHigh,xStart+xValue+arrowHigh,yStart+high+arrowHigh,coldColor);
  }
  else if (th1<value && value<=th2) {
    tft.fillRect(xStart,yStart,xTh1,high,coldColor);
    tft.fillRect(xStart+xTh1,yStart,xValue-xTh1,high,normalColor);
    tft.fillRect(xStart+xValue,yStart,width-xValue,high,colorBar);
    tft.drawFastVLine(xStart+xTh2,yStart,high+7,hotColor);
    tft.fillTriangle(xStart+xValue,yStart+high,xStart+xValue-arrowHigh,yStart+high+arrowHigh,xStart+xValue+arrowHigh,yStart+high+arrowHigh,normalColor);
  }
  else {
    tft.fillRect(xStart,yStart,xTh1,high,coldColor);
    tft.fillRect(xStart+xTh1,yStart,xTh2-xTh1,high,normalColor);  
    tft.fillRect(xStart+xTh2,yStart,xValue-xTh2,high,hotColor);
    tft.fillRect(xStart+xValue,yStart,width-xValue,high,colorBar);
    tft.fillTriangle(xStart+xValue,yStart+high,xStart+xValue-arrowHigh,yStart+high+arrowHigh,xStart+xValue+arrowHigh,yStart+high+arrowHigh,hotColor);
  }
};

void HorizontalBar::drawHorizontalBar(float_t value) {
  this->value=value;
  if (value>=valueMin && value<=valueMax) this->xValue=(value-valueMin)*width/(valueMax-valueMin);
  else if (value<valueMin) xValue=0;
  else if (value>valueMax) xValue=width;
  drawHorizontalBar();
}

void HorizontalBar::cleanHorizontalBar() {
  tft.fillRect(xStart,yStart,width,high+arrowHigh+1,colorBackground);
};

CircularGauge::CircularGauge(float_t value, float_t valueMin, float_t valueMax, int32_t x, int32_t y, int32_t r, int32_t width, float_t segment, uint32_t color,
              float_t th1, uint32_t color1, float_t th2, uint32_t color2, uint32_t colorGauge, uint32_t colorBackground) {
  // Class contructor
  
  if (value>valueMax) valueMax=value;
  
  //Definition of class variables
  CircularGauge::xCenter=x; CircularGauge::yCenter=y; CircularGauge::width=width; CircularGauge::rExt=r; CircularGauge::rInt=r-width; 
  CircularGauge::xStart=x-r; CircularGauge::yStart=y-r; CircularGauge::xEnd=x+r; CircularGauge::yEnd=y+r;
  CircularGauge::value=(float_t)value; CircularGauge::valueMin=(float_t)valueMin; CircularGauge::valueMax=(float_t)valueMax; CircularGauge::valueRange=(float_t)valueMax-(float_t)valueMin; CircularGauge::th1=(float_t)th1; CircularGauge::th2=(float_t)th2;
  CircularGauge::normalColor=color; CircularGauge::warningColor=color1; CircularGauge::alarmColor=color2; CircularGauge::colorGauge=colorGauge; CircularGauge::colorBackground=colorBackground;
  CircularGauge::angleStart=90+segment; CircularGauge::angleEnd=450-segment; CircularGauge::angleRange=360-2*segment;
  CircularGauge::angleTh1=(float_t)th1*angleRange/valueRange+angleStart;
  CircularGauge::angleTh2=(float_t)th2*angleRange/valueRange+angleStart;
  CircularGauge::valueAngle=CircularGauge::angleValue; CircularGauge::valueMinAngle=CircularGauge::angleStart; CircularGauge::valueMaxAngle=CircularGauge::angleEnd; CircularGauge::th1Angle=CircularGauge::angleTh1; CircularGauge::th2Angle=CircularGauge::angleTh2; CircularGauge::segmentAngle=segment;
  CircularGauge::colorText=colorGauge; CircularGauge::colorValue=colorGauge;
  CircularGauge::valueString=String(value);
  CircularGauge::x=x; CircularGauge::y=y;

  if (value>=valueMin && value<=valueMax) CircularGauge::angleValue=(float_t)value*angleRange/valueRange+angleStart;
  else if (value<valueMin) angleValue=angleStart;
  else if (value>valueMax) angleValue=angleEnd;
};

void CircularGauge::setValue(float_t value) {
  if (value>=valueMin && value<=valueMax) CircularGauge::angleValue=(float_t)value*angleRange/valueRange+angleStart;
  else if (value<valueMin) angleValue=angleStart;
  else if (value>valueMax) angleValue=angleEnd;

  this->value=value;
  this->valueString=String (value);
};

void CircularGauge::drawGauge2() {
  //Function to draw the circular gauge

  //Function internal variables
  float_t angle,angleRadian,angleStep=0.01,delta=segmentAngle;
  int32_t xExt,yExt,xInt,yInt,widhtExt=width*0.27, widthGap=width*0.5, widthInt=width*0.77;
  uint32_t colorLine, colorText;
  
  //Drawing the circle gauge
  for (angle=angleStart; angle<angleEnd; angle+=angleStep) {
      if (angle<=angleTh1) colorLine=normalColor;
      else if (angle>angleTh1 && angle<=angleTh2) colorLine=warningColor;
          else colorLine=alarmColor;
    
      angleRadian=(angle/180)*PI;

      xExt=round(cos(angleRadian)*rExt)+x;
      yExt=round(sin(angleRadian)*rExt)+y;
      xInt=round(cos(angleRadian)*(rExt-widhtExt))+x;
      yInt=round(sin(angleRadian)*(rExt-widhtExt))+y;
      tft.drawLine(xInt,yInt,xExt,yExt,colorLine);        

    if (angle<=angleValue) {
      xExt=round(cos(angleRadian)*(rExt-widthGap))+x;
      yExt=round(sin(angleRadian)*(rExt-widthGap))+y;
      xInt=round(cos(angleRadian)*(rExt-widthInt))+x;
      yInt=round(sin(angleRadian)*(rExt-widthInt))+y;
      tft.drawLine(xInt,yInt,xExt,yExt,colorGauge);
    }
  }

  if (value<th1) this->colorValue=normalColor;
  else if (th1<=value && value <th2) this->colorValue=warningColor;
  else this->colorValue=alarmColor;
  
  angleRadian=(angleValue/180)*PI;
  tft.fillTriangle(round(cos(angleRadian)*(rExt-widthGap))+x, round(sin(angleRadian)*(rExt-widthGap))+y,
                    round(cos(angleRadian-0.15)*(rExt-widthInt))+x, round(sin(angleRadian-0.15)*(rExt-widthInt))+y,
                    round(cos(angleRadian+0.15)*(rExt-widthInt))+x, round(sin(angleRadian+0.15)*(rExt-widthInt))+y,colorValue);
  tft.drawLine(round(cos(angleRadian)*(rExt-widthGap))+x, round(sin(angleRadian)*(rExt-widthGap))+y,
                round(cos(angleRadian)*(rExt-widthGap-25))+x,round(sin(angleRadian)*(rExt-widthGap-25))+y,colorValue);
};
    
void CircularGauge::drawGauge2(float_t value) {
  this->value=value;
  if (value>=valueMin && value<=valueMax) this->angleValue=(float_t)value*angleRange/valueRange+angleStart;
  else if (value<valueMin) angleValue=angleStart;
  else if (value>valueMax) angleValue=angleEnd;
  valueString=String(value);
  drawGauge2();
}

void CircularGauge::drawGauge1() {
  //Function to draw the circular gauge

  //Function internal variables
  float_t angle,angleRadian,angleStep=0.01,delta=segmentAngle;
  int32_t xExt,yExt,xInt,yInt,widhtExt=width*0.27, widthGap=width*0.5, widthInt=width*0.77;
  uint32_t colorLine, colorText;

  //Drawing the circle gauge
  for (angle=angleStart; angle<angleEnd; angle+=angleStep) {
    if (angle<=angleValue) {
      if (angle<=angleTh1) colorLine=normalColor;
      else if (angle>angleTh1 && angle<=angleTh2) colorLine=warningColor;
          else colorLine=alarmColor;
    }
    else colorLine=colorGauge;
    
    angleRadian=(angle/180)*PI;

    xExt=round(cos(angleRadian)*rExt)+x;
    yExt=round(sin(angleRadian)*rExt)+y;
    //xInt=round(cos(angleRadian)*(r-(width-22)))+x;
    //yInt=round(sin(angleRadian)*(r-(width-22)))+y;
    xInt=round(cos(angleRadian)*(rExt-widhtExt))+x;
    yInt=round(sin(angleRadian)*(rExt-widhtExt))+y;
    tft.drawLine(xInt,yInt,xExt,yExt,colorGauge);

    if (angle<=angleValue) {
      //xExt=round(cos(angleRadian)*(r-(width-15)))+x;
      //yExt=round(sin(angleRadian)*(r-(width-15)))+y;
      xExt=round(cos(angleRadian)*(rExt-widthGap))+x;
      yExt=round(sin(angleRadian)*(rExt-widthGap))+y;
      //xInt=round(cos(angleRadian)*(r-width+7))+x;
      //yInt=round(sin(angleRadian)*(r-width+7))+y;
      xInt=round(cos(angleRadian)*(rExt-widthInt))+x;
      yInt=round(sin(angleRadian)*(rExt-widthInt))+y;
      tft.drawLine(xInt,yInt,xExt,yExt,colorLine);
    }
  }
  angleRadian=(angleValue/180)*PI;
  /*tft.fillTriangle(round(cos(angleRadian)*(r-(width-15)))+x, round(sin(angleRadian)*(r-(width-15)))+y,
                    round(cos(angleRadian-0.15)*(r-(width-15+12)))+x, round(sin(angleRadian-0.15)*(r-(width-15+12)))+y,
                    round(cos(angleRadian+0.15)*(r-(width-15+12)))+x, round(sin(angleRadian+0.15)*(r-(width-15+12)))+y,TFT_BLUE);
  tft.drawLine(round(cos(angleRadian)*(r-(width-15)))+x, round(sin(angleRadian)*(r-(width-15)))+y,
                round(cos(angleRadian)*(r-(width-15+25)))+x,round(sin(angleRadian)*(r-(width-15+25)))+y,TFT_BLUE);
  */
  tft.fillTriangle(round(cos(angleRadian)*(rExt-widthGap))+x, round(sin(angleRadian)*(rExt-widthGap))+y,
                    round(cos(angleRadian-0.15)*(rExt-widthInt))+x, round(sin(angleRadian-0.15)*(rExt-widthInt))+y,
                    round(cos(angleRadian+0.15)*(rExt-widthInt))+x, round(sin(angleRadian+0.15)*(rExt-widthInt))+y,TFT_BLUE);
  tft.drawLine(round(cos(angleRadian)*(rExt-widthGap))+x, round(sin(angleRadian)*(rExt-widthGap))+y,
                round(cos(angleRadian)*(rExt-widthGap-25))+x,round(sin(angleRadian)*(rExt-widthGap-25))+y,TFT_BLUE);

  this->colorValue=colorLine;

  //Drawing threshold1
  angleRadian=(angleTh1/180)*PI;
  xExt=round(cos(angleRadian)*(rExt+rDeltaMark))+x;
  if ( xExt<xStart) xStart=xExt;
  if (rExt>xEnd) xEnd=rExt;
  yExt=round(sin(angleRadian)*(rExt+rDeltaMark))+y;
  if (yExt<yStart) yStart=yExt;
  if (yExt>yEnd) yEnd=yExt;
  xInt=round(cos(angleRadian)*(rExt-(width-22)))+x;
  yInt=round(sin(angleRadian)*(rExt-(width-22)))+y;
  tft.drawLine(xInt,yInt,xExt,yExt,warningColor);

  //Drawing threshold2
  angleRadian=(angleTh2/180)*PI;
  xExt=round(cos(angleRadian)*(rExt+rDeltaMark))+x;
  if ( xExt<xStart) xStart=xExt;
  if (rExt>xEnd) xEnd=rExt;
  yExt=round(sin(angleRadian)*(rExt+rDeltaMark))+y;
  if (yExt<yStart) yStart=yExt;
  if (yExt>yEnd) yEnd=yExt;
  xInt=round(cos(angleRadian)*(rExt-(width-22)))+x;
  yInt=round(sin(angleRadian)*(rExt-(width-22)))+y;
  tft.drawLine(xInt,yInt,xExt,yExt,alarmColor);
};
    
void CircularGauge::drawGauge1(float_t value) {
  this->value=value;
  if (value>=valueMin && value<=valueMax) this->angleValue=(float_t)value*angleRange/valueRange+angleStart;
  else if (value<valueMin) angleValue=angleStart;
  else if (value>valueMax) angleValue=angleEnd;
  valueString=String(value);
  drawGauge1();
}

void CircularGauge::drawGauge() {
  //Function to draw the circular gauge

  //Function internal variables
  float_t angle,angleRadian,angleStep=0.01,delta=segmentAngle;
  int32_t xExt,yExt,xInt,yInt;
  uint32_t colorLine, colorText;

  //Drawing the circle gauge
  for (angle=angleStart; angle<angleEnd; angle+=angleStep) {
    if (angle<=angleValue) {
      if (angle<=angleTh1) colorLine=normalColor;
      else if (angle>angleTh1 && angle<=angleTh2) colorLine=warningColor;
          else colorLine=alarmColor;
    }
    else colorLine=colorGauge;
    
    angleRadian=(angle/180)*PI;

    xExt=round(cos(angleRadian)*rExt)+x;
    yExt=round(sin(angleRadian)*rExt)+y;
    xInt=round(cos(angleRadian)*(rExt-width))+x;
    yInt=round(sin(angleRadian)*(rExt-width))+y;
    tft.drawLine(xInt,yInt,xExt,yExt,colorLine);
  }

  this->colorValue=colorLine;

  //Drawing threshold1
  angleRadian=(angleTh1/180)*PI;
  xExt=round(cos(angleRadian)*(rExt+rDeltaMark))+x;
  if ( xExt<xStart) xStart=xExt;
  if (rExt>xEnd) xEnd=rExt;
  yExt=round(sin(angleRadian)*(rExt+rDeltaMark))+y;
  if (yExt<yStart) yStart=yExt;
  if (yExt>yEnd) yEnd=yExt;
  xInt=round(cos(angleRadian)*(rExt-width))+x;
  yInt=round(sin(angleRadian)*(rExt-width))+y;
  tft.drawLine(xInt,yInt,xExt,yExt,warningColor);

  //Drawing threshold2
  angleRadian=(angleTh2/180)*PI;
  xExt=round(cos(angleRadian)*(rExt+rDeltaMark))+x;
  if ( xExt<xStart) xStart=xExt;
  if (rExt>xEnd) xEnd=rExt;
  yExt=round(sin(angleRadian)*(rExt+rDeltaMark))+y;
  if (yExt<yStart) yStart=yExt;
  if (yExt>yEnd) yEnd=yExt;
  xInt=round(cos(angleRadian)*(rExt-width))+x;
  yInt=round(sin(angleRadian)*(rExt-width))+y;
  tft.drawLine(xInt,yInt,xExt,yExt,alarmColor);
};

void CircularGauge::drawGauge(float_t value) {
  this->value=value;
  if (value>=valueMin && value<=valueMax) this->angleValue=(float_t)value*angleRange/valueRange+angleStart;
  else if (value<valueMin) angleValue=angleStart;
  else if (value>valueMax) angleValue=angleEnd;
  valueString=String(value);
  drawGauge();
}

void CircularGauge::drawTextGauge(String unitsString, uint32_t valueTextSize, boolean isInteger,
                  uint32_t unitsTextSize,uint32_t valueTextFont,uint32_t unitsTextFont,uint32_t colorText) {
  //Function to draw the text inside the circular gauge

  this->unitsString=unitsString;
  if (isInteger) valueString=valueString.toInt();

  //Drawing text value
  if (0==colorText) {
    //Color text is got depends on the value
    if (angleValue<=angleTh1) colorText=normalColor;
    else if (angleValue>angleTh1 && angleValue<=angleTh2) colorText=warningColor;
      else colorText=alarmColor;
  }
  
  tft.setTextSize(valueTextSize);
  tft.setTextColor(colorText);
  xStartValueText=x-(tft.textWidth(valueString,valueTextFont))/2;
  yStartValueText=y-tft.fontHeight(valueTextFont)/2;
  xEndValueText=x+(tft.textWidth(valueString,valueTextFont))/2;
  yEndValueText=yStartValueText+(int32_t)tft.fontHeight(valueTextFont);
  tft.drawString(valueString,xStartValueText,yStartValueText,valueTextFont);
  
  //Drawing text units
  tft.setTextSize(unitsTextSize);
  xStartUnitsText=x-(tft.textWidth(unitsString,unitsTextFont))/2;
  yStartUnitsText=yEndValueText+3;
  xEndUnitsText=x+(tft.textWidth(unitsString,unitsTextFont))/2;
  yEndUnitsText=yStartUnitsText+(int32_t)tft.fontHeight(unitsTextFont);
  tft.drawString(unitsString,xStartUnitsText,yStartUnitsText,unitsTextFont);
  
  this->colorText=colorText;
};

void CircularGauge::cleanGauge() {
  //Function to clean the value
  tft.fillRect(xStart,yStart,xEnd,yEnd,colorBackground);
};
  
void CircularGauge::cleanValueTextGauge() {
  //Function to clean the value
  tft.fillRect(xStartValueText,yStartValueText,xEndValueText-xStartValueText,yEndValueText-yStartValueText,colorBackground);
};

void CircularGauge::cleanUnitsTextGauge() {
  //Function to clean the untis
  tft.fillRect(xStartUnitsText,yStartUnitsText,xEndUnitsText-xStartUnitsText,yEndUnitsText-yStartUnitsText,colorBackground);
};

void CircularGauge::cleanAll() {
  this->cleanGauge();
  this->cleanValueTextGauge();
  this->cleanUnitsTextGauge();
};

void drawText(float_t value, String textString, int32_t textSize, int32_t font, uint32_t colorForeground,
              uint32_t colorBackground, int32_t x, int32_t y, 
              float_t th1, uint32_t coldColor, float_t th2, uint32_t hotColor) {
  
  //Drawing the text String
  if (value<=th1) colorForeground=coldColor;
  else if (value>th2) colorForeground=hotColor;
  
  tft.setTextSize(textSize);
  tft.setTextColor(colorForeground);
  tft.drawString(String(textString),x,y,font);
};

void printshowOptMenu() {
  //Cleaning and printing menu screen
  tft.fillScreen(MENU_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(25,10,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Options to show");
  if (stateSelected==displayingSampleFixed) {auxColorFore=MENU_BACK_COLOR;auxColorBack=MENU_FORE_COLOR;} else {auxColorFore=MENU_FORE_COLOR;auxColorBack=MENU_BACK_COLOR;}tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,40,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("CO2 measurements");
  if (stateSelected==displayingCo2LastHourGraphFixed) {auxColorFore=MENU_BACK_COLOR;auxColorBack=MENU_FORE_COLOR;} else {auxColorFore=MENU_FORE_COLOR;auxColorBack=MENU_BACK_COLOR;}tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,40+(tft.fontHeight(TEXT_FONT_MENU)+5)*1,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Last hour graph");
  if (stateSelected==displayingCo2LastDayGraphFixed) {auxColorFore=MENU_BACK_COLOR;auxColorBack=MENU_FORE_COLOR;} else {auxColorFore=MENU_FORE_COLOR;auxColorBack=MENU_BACK_COLOR;}tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,40+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Last day graph");
  if (stateSelected==displayingSequential) {auxColorFore=MENU_BACK_COLOR;auxColorBack=MENU_FORE_COLOR;} else {auxColorFore=MENU_FORE_COLOR;auxColorBack=MENU_BACK_COLOR;}tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,40+(tft.fontHeight(TEXT_FONT_MENU)+5)*3,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.print("Sequentially all");
}

void printMainMenu() {
  //Cleaning and printing menu screen
  tft.fillScreen(MENU_GLOBAL_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_GLOBAL_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Main Menu");

  if (stateSelected==showOptMenu) {auxColorFore=MENU_GLOBAL_BACK_COLOR;auxColorBack=MENU_GLOBAL_FORE_COLOR;} else {auxColorFore=MENU_GLOBAL_FORE_COLOR;auxColorBack=MENU_GLOBAL_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Show Options Menu");
  if (stateSelected==infoMenu) {auxColorFore=MENU_GLOBAL_BACK_COLOR;auxColorBack=MENU_GLOBAL_FORE_COLOR;} else {auxColorFore=MENU_GLOBAL_FORE_COLOR;auxColorBack=MENU_GLOBAL_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*1,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("General Info");
  if (stateSelected==configMenu) {auxColorFore=MENU_GLOBAL_BACK_COLOR;auxColorBack=MENU_GLOBAL_FORE_COLOR;} else {auxColorFore=MENU_GLOBAL_FORE_COLOR;auxColorBack=MENU_GLOBAL_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Config Menu");
  if (stateSelected==factResetMenu) {auxColorFore=MENU_GLOBAL_BACK_COLOR;auxColorBack=MENU_GLOBAL_FORE_COLOR;} else {auxColorFore=MENU_GLOBAL_FORE_COLOR;auxColorBack=MENU_GLOBAL_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*3,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Factory Reset");
  //Printing Back option
  if (  (stateSelected==displayingSampleFixed) || (stateSelected==displayingCo2LastHourGraphFixed) ||
        (stateSelected==displayingCo2LastDayGraphFixed) || (stateSelected==displayingSequential) )
        {auxColorFore=MENU_GLOBAL_BACK_COLOR;auxColorBack=MENU_GLOBAL_FORE_COLOR;}
  else {auxColorFore=MENU_GLOBAL_FORE_COLOR;auxColorBack=MENU_GLOBAL_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Back");
}

void printFactoryResetMenu() {
  //Cleaning and printing menu screen
  tft.fillScreen(MENU_FACT_RESET_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_FACT_RESET_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(50,10,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Factory Reset");

  //Printing the TEXT
  auxColorFore=MENU_FACT_RESET_FORE_COLOR;auxColorBack=MENU_FACT_RESET_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(1,44,TEXT_FONT_BOOT_SCREEN); tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.println("  All network services, including web\n  access, won't be longer available.\n  The WiFi will be setup next bootup.");
  
  //Printing first Config Menu Option - WiFi ON/OFF-
  //tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(1,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println(" ");
  tft.setTextSize(TEXT_SIZE_MENU);
  tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(1,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Continue?");
  if ( (stateSelected==factResetMenu || stateSelected==factReset) ) {auxColorFore=MENU_FACT_RESET_FORE_COLOR;auxColorBack=MENU_FACT_RESET_BACK_COLOR;} else {auxColorFore=MENU_FACT_RESET_BACK_COLOR;auxColorBack=MENU_FACT_RESET_FORE_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(110,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println(" CANCEL ");
  tft.setTextColor(auxColorBack,auxColorFore);tft.setCursor(200,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println(" OK ");
}

void printInfoMenu() {
  //Cleaning and printing menu screen
  tft.fillScreen(MENU_INFO_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Info Menu");

  if ( (stateSelected==infoMenu || stateSelected==infoMenu1) ) {auxColorFore=MENU_INFO_BACK_COLOR;auxColorBack=MENU_INFO_FORE_COLOR;} else {auxColorFore=MENU_INFO_FORE_COLOR;auxColorBack=MENU_INFO_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Gral. Info");
  if (stateSelected==infoMenu2) {auxColorFore=MENU_INFO_BACK_COLOR;auxColorBack=MENU_INFO_FORE_COLOR;} else {auxColorFore=MENU_INFO_FORE_COLOR;auxColorBack=MENU_INFO_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*1,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Sensors Info");
  if (stateSelected==infoMenu3) {auxColorFore=MENU_INFO_BACK_COLOR;auxColorBack=MENU_INFO_FORE_COLOR;} else {auxColorFore=MENU_INFO_FORE_COLOR;auxColorBack=MENU_INFO_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("WiFi Info");
  if (stateSelected==infoMenu4) {auxColorFore=MENU_INFO_BACK_COLOR;auxColorBack=MENU_INFO_FORE_COLOR;} else {auxColorFore=MENU_INFO_FORE_COLOR;auxColorBack=MENU_INFO_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*3,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Net Info");
  if (stateSelected==mainMenu) {auxColorFore=MENU_INFO_BACK_COLOR;auxColorBack=MENU_INFO_FORE_COLOR;} else {auxColorFore=MENU_INFO_FORE_COLOR;auxColorBack=MENU_INFO_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(20,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Back");
  
}

void printConfigMenu() {
  //Cleaning and printing menu screen
  tft.fillScreen(MENU_CONFIG_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_CONFIG_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Config Menu");

  //Printing the first Config Menu Option - WiFi ON/OFF-
  if ( (stateSelected==configMenu || stateSelected==confMenuWifi) ) {auxColorFore=MENU_CONFIG_BACK_COLOR;auxColorBack=MENU_CONFIG_FORE_COLOR;} else {auxColorFore=MENU_CONFIG_FORE_COLOR;auxColorBack=MENU_CONFIG_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(10,34,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("WiFi:          ");
  tft.setCursor(10+tft.textWidth("WiFi:          ",TEXT_FONT_MENU),34,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);
  if (wifiEnabled) {tft.println("ON");} else {tft.println("OFF");}

  //Printing the 2nd Config Menu Option - Blueth.: ON/OFF-
  if ( (stateSelected==confMenuBLE) ) {auxColorFore=MENU_CONFIG_BACK_COLOR;auxColorBack=MENU_CONFIG_FORE_COLOR;} else {auxColorFore=MENU_CONFIG_FORE_COLOR;auxColorBack=MENU_CONFIG_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(10,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*1,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Bluetooth:     ");
  tft.setCursor(10+tft.textWidth("Bluetooth:     ",TEXT_FONT_MENU),34+(tft.fontHeight(TEXT_FONT_MENU)+5)*1,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);
  if (bluetoothEnabled) {tft.println("ON");} else {tft.println("OFF");}
  
  //Printing the 3nd Config Menu Option - Upl. Meas.: ON/OFF-
  if (wifiEnabled) { 
    if ( (stateSelected==confMenuUpMeas) ) {auxColorFore=MENU_CONFIG_BACK_COLOR;auxColorBack=MENU_CONFIG_FORE_COLOR;} else {auxColorFore=MENU_CONFIG_FORE_COLOR;auxColorBack=MENU_CONFIG_BACK_COLOR;}
    tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(10,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Upload Measu.: ");
    tft.setCursor(10+tft.textWidth("Upload Measu.: ",TEXT_FONT_MENU),34+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);
    if (uploadSamplesEnabled) {tft.println("ON");} else {tft.println("OFF");}
  }
  else {
    auxColorFore=TFT_DARKGREY;auxColorBack=MENU_CONFIG_BACK_COLOR;
    tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(10,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Upload Measu.: ");
    tft.setCursor(10+tft.textWidth("Upload Measu.: ",TEXT_FONT_MENU),34+(tft.fontHeight(TEXT_FONT_MENU)+5)*2,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);
    if (uploadSamplesEnabled) {tft.println("ON");} else {tft.println("OFF");}
  }

  //Printing the 4th Config Menu Option - Bat. Mode: Red./Low-
  if ( (stateSelected==confMenuSavBatMode) ) {auxColorFore=MENU_CONFIG_BACK_COLOR;auxColorBack=MENU_CONFIG_FORE_COLOR;} else {auxColorFore=MENU_CONFIG_FORE_COLOR;auxColorBack=MENU_CONFIG_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);tft.setCursor(10,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*3,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);tft.println("Bat. Mode: ");
  tft.setCursor(10+tft.textWidth("Bat. Mode: ",TEXT_FONT_MENU),34+(tft.fontHeight(TEXT_FONT_MENU)+5)*3,TEXT_FONT_MENU); tft.setTextSize(TEXT_SIZE_MENU);
  if (reducedEnergy==configSavingEnergyMode) {tft.println("Reduced");} else {tft.println("Lowest");}
  
  //Printing the Back option
  if (stateSelected==mainMenu) {auxColorFore=MENU_CONFIG_BACK_COLOR;auxColorBack=MENU_CONFIG_FORE_COLOR;} else {auxColorFore=MENU_CONFIG_FORE_COLOR;auxColorBack=MENU_CONFIG_BACK_COLOR;}
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(10,34+(tft.fontHeight(TEXT_FONT_MENU)+5)*4,TEXT_FONT_MENU);tft.setTextSize(TEXT_SIZE_MENU);tft.println("Back");
}

void printGralInfo() {
  struct tm timeinfo;
  boolean ntpServerAvailable=true;
  if(!getLocalTime(&timeinfo)){
    if (debugModeOn) Serial.println("Failed to obtain time");
    ntpServerAvailable=false;
  }
  
  tft.fillScreen(MENU_INFO_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(54,10,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.print("Gral. Info - FW v");tft.print(VERSION);
  
  auxColorFore=MENU_INFO_FORE_COLOR,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(15,30,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  switch (powerState) {
    case(off):
      //It's suppossed this case is no possible
    break;
    case(chargingUSB):
      tft.print("USB charging: ");tft.print(batADCVolt/1000);tft.print("v, ");tft.print(batCharge);tft.print("%");
    break;
    case(onlyBattery):
      tft.print("Battery: ");tft.print(batADCVolt/1000);tft.print("v, ");tft.print(batCharge);tft.print("%");
    break;
    case(noChargingUSB):
      tft.print("USB no charging: ");tft.print(batADCVolt/1000);tft.print("v, ");tft.print(batCharge);tft.print("%");
    break;
  }
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*1,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Date: ");
  if (ntpServerAvailable) tft.println(&timeinfo, "%d/%m/%Y - %H:%M:%S");
  else tft.println("NTP server down");
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*2,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Uptime since: ");tft.println(&startTimeInfo, "%d/%m/%y - %H:%M:%S");
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*3,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Report URL: ");
  if (uploadSamplesEnabled) //URL shown only if setup
    tft.print("http://"+serverToUploadSamplesIPAddress.toString()+String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1));
  else
    tft.print("Not Available");
  tft.setTextColor(MENU_INFO_BACK_COLOR,MENU_INFO_FORE_COLOR);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*5,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.println("Back");

  boolean exitWhile=false,batFlag=true;
  while (!exitWhile) {
    //Loop to update the date/hour every second
    if(!getLocalTime(&timeinfo)){
      if (debugModeOn) Serial.println("Failed to obtain time");
      ntpServerAvailable=false;
    }
    else ntpServerAvailable=true;
    for (uint8_t count=1; count<=10; count++) {
      if (button2.pressed()) {checkButton2(); exitWhile=true;}
      delay(100); //10 times 100 ms = 1 sg . Loop to update date/hour every second
    }
    if (exitWhile) break;
    auxColorFore=MENU_INFO_FORE_COLOR,auxColorBack=MENU_INFO_BACK_COLOR;
    tft.setTextColor(auxColorFore,auxColorBack);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*1,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.print("Date:                        ");
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*1,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.print("Date: ");
    if (ntpServerAvailable) tft.println(&timeinfo, "%d/%m/%Y - %H:%M:%S");
    else tft.println("NTP server down");

    //Update battery parameters also every 2 seconds
    //Power state check
    if (batFlag) {
      digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_ENABLE); delay(POWER_ENABLE_DELAY);
      batADCVolt=0; for (u8_t i=1; i<=ADC_SAMPLES; i++) batADCVolt+=analogReadMilliVolts(BAT_ADC_PIN); batADCVolt=batADCVolt/ADC_SAMPLES;
      digitalWrite(POWER_ENABLE_PIN, BAT_CHECK_DISABLE); //To minimize BAT consume

      /*--><--*///batADCVolt=1900;
      
      if (batADCVolt >= VOLTAGE_TH_STATE) {
        //USB is plugged. Assume battery is always plugged and charged after FULL_CHARGE_TIME milliseconds
        if(noChargingUSB!=powerState) {
          powerState=chargingUSB;
          if ((nowTimeGlobal-timeUSBPowerGlobal)>=FULL_CHARGE_TIME)
            powerState=noChargingUSB;
        }
        if (0==timeUSBPowerGlobal) timeUSBPowerGlobal=nowTimeGlobal;
      }
      else {
        powerState=onlyBattery;
        timeUSBPowerGlobal=0;
      }

      if (onlyBattery==powerState)
        //Take battery charge when the Battery is plugged
        batteryStatus=getBatteryStatus(batADCVolt,0);
      else
        //When USB is plugged, the Battery charge can be only guessed based on
        // the time the USB is being plugged 
        batteryStatus=getBatteryStatus(batADCVolt,nowTimeGlobal-timeUSBPowerGlobal);

      //Print battery parameters
      tft.setCursor(15,30,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
      switch (powerState) {
        case(off):
          //It's suppossed this case is no possible
        break;
        case(chargingUSB):
          tft.print("USB charging: ");tft.print(batADCVolt/1000);tft.print("v, ");tft.print(batCharge);tft.print("%     ");
        break;
        case(onlyBattery):
          tft.print("Battery: ");tft.print(batADCVolt/1000);tft.print("v, ");tft.print(batCharge);tft.print("%          ");
        break;
        case(noChargingUSB):
          tft.print("USB no charging: ");tft.print(batADCVolt/1000);tft.print("v, ");tft.print(batCharge);tft.print("%");
        break;
      }
    }
    batFlag=!batFlag;
  }

  return;
}

void printSensorsInfo() {
  tft.fillScreen(MENU_INFO_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.println("Sensors Info");
  
  auxColorFore=MENU_INFO_FORE_COLOR,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(15,30,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Sensor Temp/HUM: ");tft.print(tempHumSensorType);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*1,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("CO2 Sensor type: ");tft.println(co2SensorType);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*2,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("  Version: ");tft.println(co2SensorVersion);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*3,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("  Accuracy: ");tft.print(co2Sensor.getAccuracy(false));
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*4,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("  Int. Temp (C): ");tft.print(co2Sensor.getTemperature(true,true));
  tft.setTextColor(MENU_INFO_BACK_COLOR,MENU_INFO_FORE_COLOR);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*5,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.println("Back");

  return;
}

void printWifiInfo() {
  int16_t numberWiFiNetworks=0;
  tft.fillScreen(MENU_INFO_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.println("WiFi Info");

  //Scanning first
  auxColorFore=MENU_INFO_FORE_COLOR,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(40,50,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Wait while scanning");
  tft.setCursor(30,75,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("It might take a while.....");
  wifiNetworkInfo* auxWifiNet=printCurrentWiFi(false,&numberWiFiNetworks);

  //Priting results
  tft.fillScreen(MENU_INFO_BACK_COLOR);
  auxColorFore=TFT_GREEN,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.println("WiFi Info");

  auxColorFore=MENU_INFO_FORE_COLOR,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(15,30,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("SSID: ");

  //If no SSID connected, tell
  if (nullptr==auxWifiNet) {
    tft.print("Not Connected");tft.print(" & detected: ");tft.print(numberWiFiNetworks);

    tft.setTextColor(MENU_INFO_BACK_COLOR,MENU_INFO_FORE_COLOR);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*5,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.println("Back");
  }
  else {
    //If SSID connected, then show info
    tft.print(wifiNet.ssid);
    tft.print(" & detected: ");tft.print(numberWiFiNetworks);

    uint8_t bssid[6];
    memcpy(bssid, wifiNet.BSSID, 6);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*1,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.print("BSSID: ");tft.print(bssid[5], HEX); tft.print(":");tft.print(bssid[4], HEX); tft.print(":");tft.print(bssid[3], HEX); tft.print(":");tft.print(bssid[2], HEX); tft.print(":");tft.print(bssid[1], HEX); tft.print(":");tft.println(bssid[0], HEX);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*2,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.print("Signal strength (RSSI): ");tft.print(wifiNet.RSSI);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*3,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.print("Encryption Type: ");tft.print(wifiNet.encryptionType, HEX);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*4,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.print("WiFi Channel: ");tft.print(wifiNet.channel);
    tft.setTextColor(MENU_INFO_BACK_COLOR,MENU_INFO_FORE_COLOR);
    tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*5,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
    tft.println("Back");
  }
  
  return;
}

void printNetInfo() {
  tft.fillScreen(MENU_INFO_BACK_COLOR);
  uint16_t auxColorFore=TFT_GREEN,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(60,10,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.println("Net Info");

  auxColorFore=MENU_INFO_FORE_COLOR,auxColorBack=MENU_INFO_BACK_COLOR;
  tft.setTextColor(auxColorFore,auxColorBack);
  tft.setCursor(15,30,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  byte mac[6];
  WiFi.macAddress(mac);
  tft.print("MAC address: ");tft.print(mac[0], HEX);tft.print(":");tft.print(mac[1], HEX);tft.print(":");tft.print(mac[2], HEX);tft.print(":");tft.print(mac[3], HEX);tft.print(":");tft.print(mac[4], HEX);tft.print(":");tft.println(mac[5], HEX);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*1,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("IP Address: "); tft.print(WiFi.localIP().toString());
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*2,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Mask: "); tft.println(WiFi.subnetMask().toString());
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*3,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("Default GW: ");tft.println(WiFi.gatewayIP().toString());
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*4,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.print("NTP Server: "); if (CloudClockCurrentStatus==CloudClockOnStatus) tft.print(ntpServers[ntpServerIndex]); else tft.print("No Available");
  tft.setTextColor(MENU_INFO_BACK_COLOR,MENU_INFO_FORE_COLOR);
  tft.setCursor(15,30+(tft.fontHeight(TEXT_FONT_BOOT_SCREEN)+3)*5,TEXT_FONT_BOOT_SCREEN);tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);
  tft.println("Back");

  return;
}

void loadBootImage() {
  //-->>Load the logo image when booting up
  
  return;
}

void showIcons() {
  //Load the icons

  tft.setSwapBytes(true);
  
  //Drawign wifi icon
  switch (wifiCurrentStatus) {
    case (wifi100Status):
      tft.pushImage(0,0,24,24,wifi100);
    break;
    case (wifi75Status):
      tft.pushImage(0,0,24,24,wifi075);
    break;
    case (wifi50Status):
      tft.pushImage(0,0,24,24,wifi050);
    break;
    case (wifi25Status):
      tft.pushImage(0,0,24,24,wifi025);
    break;
    case (wifi0Status):
      tft.pushImage(0,0,24,24,wifi000);
    break;
    case (wifiOffStatus):
      tft.pushImage(0,0,24,24,wifiOff);
    break;
  }
  
  //-->>Get BLE status
  switch (BLECurrentStatus) {
    case BLEOnStatus:
      tft.pushImage(30,0,24,24,bluetooth);
    break;
    case BLEConnectedStatus:
      tft.pushImage(30,0,24,24,bluetoothConnected);
    break;
    case BLEStandbyStatus:
      tft.pushImage(30,0,24,24,bluetoothStandby);
    break;
    case BLEOffStatus:
      tft.pushImage(30,0,24,24,bluetoothOff);
    break;
    
  }
  
  //-->>Get NTP status
  switch (CloudClockCurrentStatus) {
    case (CloudClockOnStatus):
      tft.pushImage(95,0,24,24,cloudClockOn);
    break;
    case (CloudClockOffStatus):
      tft.pushImage(95,0,24,24,cloudClockOff);
    break;
  }
  
    //-->>Get MQTT status
  switch (MqttSyncCurrentStatus) {
    case (MqttSyncOnStatus):
      tft.pushImage(125,0,24,24,mqttSyncOn);
    break;
    case (MqttSyncOffStatus):
      tft.pushImage(125,0,24,24,mqttSyncOff);
    break;
   }
   
   //-->>Get Cloud status
  switch (CloudSyncCurrentStatus) {
    case (CloudSyncOnStatus):
      tft.pushImage(155,0,24,24,cloudSyncOn);
    break;
    case (CloudSyncOffStatus):
      tft.pushImage(155,0,24,24,cloudSyncOff);
    break;
   }
  
  //-->>Get Batery status
  switch (batteryStatus) {
    case (batteryCharging000):
      tft.pushImage(215,0,24,24,StatusBatteryCharging000);
    break;
    case (batteryCharging010):
      tft.pushImage(215,0,24,24,StatusBatteryCharging010);
    break;
    case (batteryCharging025):
      tft.pushImage(215,0,24,24,StatusBatteryCharging025);
    break;
    case (batteryCharging050):
      tft.pushImage(215,0,24,24,StatusBatteryCharging050);
    break;
    case (batteryCharging075):
      tft.pushImage(215,0,24,24,StatusBatteryCharging075);
    break;
    case (batteryCharging100):
      tft.pushImage(215,0,24,24,StatusBatteryCharging100);
    break;
    case (battery000):
      tft.pushImage(215,0,24,24,StatusBattery000);
    break;
    case (battery010):
      tft.pushImage(215,0,24,24,StatusBattery010);
    break;
    case (battery025):
      tft.pushImage(215,0,24,24,StatusBattery025);
    break;
    case (battery050):
      tft.pushImage(215,0,24,24,StatusBattery050);
    break;
    case (battery075):
      tft.pushImage(215,0,24,24,StatusBattery075);
    break;
    case (battery100):
      tft.pushImage(215,0,24,24,StatusBattery100);
    break;
  } 
}

void loadAllIcons() {
  tft.setSwapBytes(true);

  tft.pushImage(0,0,24,24,StatusBattery000);
  tft.pushImage(30,0,24,24,StatusBattery010);
  tft.pushImage(60,0,24,24,StatusBattery025);
  tft.pushImage(90,0,24,24,StatusBattery050);
  tft.pushImage(120,0,24,24,StatusBattery075);
  tft.pushImage(150,0,24,24,StatusBattery100);
    
  tft.pushImage(0,30,24,24,StatusBatteryCharging000);
  tft.pushImage(30,30,24,24,StatusBatteryCharging010);
  tft.pushImage(60,30,24,24,StatusBatteryCharging025);
  tft.pushImage(90,30,24,24,StatusBatteryCharging050);
  tft.pushImage(120,30,24,24,StatusBatteryCharging075);
  tft.pushImage(150,30,24,24,StatusBatteryCharging100);

  tft.pushImage(0,60,24,24,wifi000);
  tft.pushImage(30,60,24,24,wifi025);
  tft.pushImage(60,60,24,24,wifi050);
  tft.pushImage(90,60,24,24,wifi075);
  tft.pushImage(120,60,24,24,wifi100);
  tft.pushImage(150,60,24,24,wifiOff);

  tft.pushImage(0,90,24,24,bluetooth);
  tft.pushImage(30,90,24,24,bluetoothConnected);
  tft.pushImage(60,90,24,24,bluetoothOff);

  tft.pushImage(180,0,24,24,cloudClockOn);
  tft.pushImage(180,30,24,24,cloudClockOff);

  tft.pushImage(210,0,24,24,cloudSyncOn);
  tft.pushImage(210,30,24,24,cloudSyncOff);
  
  while(true);
}

void loadAllWiFiIcons() {
  tft.setSwapBytes(true);

  
  /*tft.pushImage(0,0,24,24,wifi000_blue);
  tft.pushImage(30,0,24,24,wifi010_blue);
  tft.pushImage(60,0,24,24,wifi025_blue);
  tft.pushImage(90,0,24,24,wifi050_blue);
  tft.pushImage(120,0,24,24,wifi075_blue);
  tft.pushImage(150,0,24,24,wifi100_blue);
  //tft.pushImage(180,0,24,24,wifiOff_blue);
  tft.pushImage(180,0,24,24,wifiOff_3_blue);
  tft.pushImage(210,0,24,24,wifiOff_2_blue);

  tft.pushImage(0,70,24,24,wifi000_white);
  tft.pushImage(30,70,24,24,wifi010_white);
  tft.pushImage(60,70,24,24,wifi025_white);
  tft.pushImage(90,70,24,24,wifi050_white);
  tft.pushImage(120,70,24,24,wifi075_white);
  tft.pushImage(150,70,24,24,wifi100_white);
  //tft.pushImage(180,70,24,24,wifiOff_white);
  tft.pushImage(180,70,24,24,wifiOff_3_white);
  tft.pushImage(210,70,24,24,wifiOff_2_white);*/
  
  /*tft.pushImage(0,0,24,24,wifi000_blue);
  tft.pushImage(30,0,24,24,wifi025_blue);
  tft.pushImage(60,0,24,24,wifi050_blue);
  tft.pushImage(90,0,24,24,wifi075_blue);
  tft.pushImage(120,0,24,24,wifi100_blue);
  tft.pushImage(150,0,24,24,wifiOff_blue);

  tft.pushImage(0,30,24,24,wifi000_blue_bis);
  tft.pushImage(30,30,24,24,wifi025_blue_bis);
  tft.pushImage(60,30,24,24,wifi050_blue_bis);
  tft.pushImage(90,30,24,24,wifi075_blue_bis);
  tft.pushImage(120,30,24,24,wifi100_blue);
  tft.pushImage(150,30,24,24,wifiOff_blue_bis);

  tft.pushImage(0,70,24,24,wifi000_white);
  tft.pushImage(30,70,24,24,wifi025_white);
  tft.pushImage(60,70,24,24,wifi050_white);
  tft.pushImage(90,70,24,24,wifi075_white);
  tft.pushImage(120,70,24,24,wifi100_white);
  tft.pushImage(150,70,24,24,wifiOff_white);

  tft.pushImage(0,100,24,24,wifi000_bis3);
  tft.pushImage(30,100,24,24,wifi025_bis2);
  tft.pushImage(60,100,24,24,wifi050_bis2);
  tft.pushImage(90,100,24,24,wifi075_bis2);
  tft.pushImage(120,100,24,24,wifi100_bis2);
  tft.pushImage(150,100,24,24,wifiOff);
  */
  
  /*tft.pushImage(0,100,24,24,wifi000_white_bis);
  tft.pushImage(30,100,24,24,wifi025_white_bis);
  tft.pushImage(60,100,24,24,wifi050_white_bis);
  tft.pushImage(90,100,24,24,wifi075_white_bis);
  tft.pushImage(120,100,24,24,wifi100_white);
  tft.pushImage(150,100,24,24,wifiOff_white_bis);

  tft.pushImage(0,100,24,24,wifi000_bis);
  tft.pushImage(30,100,24,24,wifi025_bis);
  tft.pushImage(60,100,24,24,wifi050_bis);
  tft.pushImage(90,100,24,24,wifi075_bis);
  tft.pushImage(120,100,24,24,wifi100);
  tft.pushImage(150,100,24,24,wifiOff);*/
  
  while(true);
}

void drawGraphLastHourCo2() {
  //Clean-up display
  tft.fillScreen(TFT_BLACK);

  //Draw thresholds, axis & legend
  tft.fillRect(CO2_GRAPH_X,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH2*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),CO2_GRAPH_WIDTH,8,TFT_NAVY);
  tft.drawFastVLine(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_HEIGH+5,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_X-5,CO2_GRAPH_Y+2,CO2_GRAPH_X+5,CO2_GRAPH_Y+2,TFT_DARKGREY);
  tft.drawFastHLine(CO2_GRAPH_X,CO2_GRAPH_Y+CO2_GRAPH_HEIGH,CO2_GRAPH_WIDTH,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X_END,CO2_GRAPH_Y_END,CO2_GRAPH_X_END-5,CO2_GRAPH_Y_END-3,CO2_GRAPH_X_END-5,CO2_GRAPH_Y_END+3,TFT_DARKGREY);
  for (int i=1;i<=12;i++) if (3*(int)(i/3)==i) tft.drawFastVLine(CO2_GRAPH_X+15*i,CO2_GRAPH_Y_END-5,10,TFT_DARKGREY);
    else tft.drawFastVLine(CO2_GRAPH_X+15*i,CO2_GRAPH_Y_END-2,4,TFT_DARKGREY); 
  for (int i=0;i<=3;i++) tft.drawFastHLine(CO2_GRAPH_X-5,CO2_GRAPH_Y+25*i,10,TFT_DARKGREY);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(CO2_GRAPH_X+35,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-45");
  tft.setCursor(CO2_GRAPH_X+80,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-30");
  tft.setCursor(CO2_GRAPH_X+125,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-15");
  tft.setCursor(CO2_GRAPH_X+170,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("Now");
  tft.setCursor(CO2_GRAPH_X+190,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("t(m)");
  tft.setCursor(CO2_GRAPH_X+15,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("Last 60 min   ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREEN,TFT_BLACK);tft.print("CO2 (ppm) [0-2000]");
  tft.setCursor(CO2_GRAPH_X+15,14,TEXT_FONT_BOOT_SCREEN-1);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_CYAN,TFT_BLACK);tft.print("Temp (C) [0-50] ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_MAGENTA,TFT_BLACK);tft.print(" Hum (%) [0-100]");

  //Draw samples
  int32_t co2Sample,tempSample,humSample,auxCo2Color,auxTempColor=TFT_CYAN,auxHumColor=TFT_MAGENTA;
  for (int i=0; i<(int)(3600/SAMPLE_T_LAST_HOUR); i++) {
    co2Sample=(int32_t) (CO2_GRAPH_Y_END-lastHourCo2Samples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX);
    tempSample=(int32_t) (CO2_GRAPH_Y_END-lastHourTempSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_TEMP_MAX);
    humSample=(int32_t) (CO2_GRAPH_Y_END-lastHourHumSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_HUM_MAX);
    if (lastHourCo2Samples[i]<=CO2_GAUGE_TH1) auxCo2Color=TFT_GREEN;
    else if (CO2_GAUGE_TH1 < lastHourCo2Samples[i] && lastHourCo2Samples[i] <= CO2_GAUGE_TH2) auxCo2Color=TFT_YELLOW;
    else auxCo2Color=TFT_RED;
    if (co2Sample==CO2_GRAPH_Y_END) auxCo2Color=TFT_DARKGREY;
    if(lastHourTempSamples[i]==0) auxTempColor=TFT_DARKGREY; else auxTempColor=TFT_CYAN;
    if(lastHourHumSamples[i]==0) auxHumColor=TFT_DARKGREY; else auxHumColor=TFT_MAGENTA;
    tft.drawPixel(i+CO2_GRAPH_X,humSample,auxHumColor); //Hum sample
    tft.drawPixel(i+CO2_GRAPH_X,tempSample,auxTempColor); //Temp sample
    tft.drawPixel(i+CO2_GRAPH_X,co2Sample,auxCo2Color); //CO2 sample
  }
}

void drawGraphLastDayCo2() {
  //Clean-up display
  tft.fillScreen(TFT_BLACK);

  tft.fillRect(CO2_GRAPH_X,(int32_t)(CO2_GRAPH_Y_END-CO2_GAUGE_TH2*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX),CO2_GRAPH_WIDTH,8,TFT_NAVY);
  tft.drawFastVLine(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_HEIGH+5,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X,CO2_GRAPH_Y-5,CO2_GRAPH_X-5,CO2_GRAPH_Y+2,CO2_GRAPH_X+5,CO2_GRAPH_Y+2,TFT_DARKGREY);
  tft.drawFastHLine(CO2_GRAPH_X,CO2_GRAPH_Y+CO2_GRAPH_HEIGH,CO2_GRAPH_WIDTH+12,TFT_DARKGREY);tft.fillTriangle(CO2_GRAPH_X_END+12,CO2_GRAPH_Y_END,CO2_GRAPH_X_END+12-5,CO2_GRAPH_Y_END-3,CO2_GRAPH_X_END+12-5,CO2_GRAPH_Y_END+3,TFT_DARKGREY);
  for (int i=1;i<=12;i++) if (3*(int)(i/3)==i) tft.drawFastVLine(CO2_GRAPH_X+16*i,CO2_GRAPH_Y_END-5,10,TFT_DARKGREY);
    else tft.drawFastVLine(CO2_GRAPH_X+16*i,CO2_GRAPH_Y_END-2,4,TFT_DARKGREY);
  for (int i=0;i<=3;i++) tft.drawFastHLine(CO2_GRAPH_X-5,CO2_GRAPH_Y+25*i,10,TFT_DARKGREY);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(CO2_GRAPH_X+38,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-18");
  tft.setCursor(CO2_GRAPH_X+86,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-12");
  tft.setCursor(CO2_GRAPH_X+134,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("-6");
  tft.setCursor(CO2_GRAPH_X+165,CO2_GRAPH_Y_END+10,TEXT_FONT_BOOT_SCREEN-1);tft.print("Now t(h)");
  tft.setCursor(CO2_GRAPH_X+15,4,TEXT_FONT_BOOT_SCREEN-1);tft.print("Last 24 h.    ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_DARKGREEN,TFT_BLACK);tft.print("CO2 (ppm) [0-2000]");
  tft.setCursor(CO2_GRAPH_X+15,14,TEXT_FONT_BOOT_SCREEN-1);
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_CYAN,TFT_BLACK);tft.print("Temp (C) [0-50] ");
  tft.setTextSize(TEXT_SIZE_BOOT_SCREEN);tft.setTextColor(TFT_MAGENTA,TFT_BLACK);tft.print(" Hum (%) [0-100]");
  //Draw samples
  int32_t co2Sample,tempSample,humSample,auxCo2Color,auxTempColor=TFT_CYAN,auxHumColor=TFT_MAGENTA;
  for (int i=0; i<(int)(24*3600/SAMPLE_T_LAST_DAY); i++)
  {
    co2Sample=(int32_t) (CO2_GRAPH_Y_END-lastDayCo2Samples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_CO2_MAX);
    tempSample=(int32_t) (CO2_GRAPH_Y_END-lastDayTempSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_TEMP_MAX);
    humSample=(int32_t) (CO2_GRAPH_Y_END-lastDayHumSamples[i]*CO2_GRAPH_HEIGH/CO2_SENSOR_HUM_MAX);
    if (lastDayCo2Samples[i]<=CO2_GAUGE_TH1) auxCo2Color=TFT_GREEN;
    else if (CO2_GAUGE_TH1 < lastDayCo2Samples[i] && lastDayCo2Samples[i] <= CO2_GAUGE_TH2) auxCo2Color=TFT_YELLOW;
    else auxCo2Color=TFT_RED;
    if (co2Sample==CO2_GRAPH_Y_END) auxCo2Color=TFT_DARKGREY;
    if(lastDayTempSamples[i]==0) auxTempColor=TFT_DARKGREY; else auxTempColor=TFT_CYAN;
    if(lastDayHumSamples[i]==0) auxHumColor=TFT_DARKGREY; else auxHumColor=TFT_MAGENTA;
    tft.drawPixel(i+CO2_GRAPH_X,humSample,auxHumColor); //Hum sample
    tft.drawPixel(i+CO2_GRAPH_X,tempSample,auxTempColor); //Temp sample
    tft.drawPixel(i+CO2_GRAPH_X,co2Sample,auxCo2Color);   //CO2 sample
  }
}