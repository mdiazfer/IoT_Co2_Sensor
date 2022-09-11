/* Library for displaying graphs in the TFT

*/

#include "display_support.h"

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


