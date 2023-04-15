// Get current sensor readings when the page loads  
window.addEventListener('newSamples', getReadings);

// Create Temperature Gauge
var gaugeTemp = new LinearGauge({
  renderTo: 'gauge-temperature',
  width: 90,
  height: 400,
  units: "Temperature C",
  minValue: -10,
  maxValue: 50,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
    "-10",
    "-5",
    "0",
    "5",
    "10",
    "15",
    "20",
    "25",
    "30",
    "35",
    "40",
    "45",
    "50"
  ],
  minorTicks: 5,
  strokeTicks: true,
  highlights: [
      {
          "from": 35,
          "to": 50,
          "color": "rgba(200, 50, 50, .75)"
      }
  ],
  //colorPlate: "#89BFFF",
  //colorPlate: "#fff",
  //colorPlate: "#34383b",
    colorPlate: "#43484c",
  //colorNumbers: "#ccc",
    colorNumbers: "#d5ca95",
    colorMajorTicks: "#d5ca95",
    colorMinorTicks: "#d5ca95",
    colorTitle: "#d5ca95",
    colorUnits: "#d5ca95",
colorBarProgress: "#CC2936",
  //colorBarProgressEnd: "#15d2e0",
  //colorBarProgressEnd: "#00e4f5",
  colorBarProgressEnd: "#00ffff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
  //value: 37
}).draw();

// Create Temperature Gauge
var gaugeHum = new LinearGauge({
  renderTo: 'gauge-humidity',
  width: 90,
  height: 400,
  units: "Humidity (%)",
  minValue: 0,
  maxValue: 100,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
    "0",
    "20",
    "40",
    "60",
    "80",
    "100"
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
        "from": 80,
        "to": 100,
        "color": "#03C0C1"
      }
  ],
  //colorPlate: "#fff",
  colorPlate: "#5e4870",
      colorNumbers: "#d5ca95",
  colorMajorTicks: "#d5ca95",
    colorMinorTicks: "#d5ca95",
    colorTitle: "#d5ca95",
    colorUnits: "#d5ca95",
colorBarProgress: "#327ac0",
  colorBarProgressEnd: "#f5f5f5",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  animationDuration: 1500,
  animationRule: "linear",
  tickSide: "left",
  numberSide: "left",
  needleSide: "left",
  barStrokeWidth: 4,
  barStrokeColor: "#5e4870",
  barBeginCircle: false,
  //value: 90
}).draw();	
  
// Create CO2 Gauge
var gaugeCO2 = new RadialGauge({
  renderTo: 'gauge-CO2',
  width: 210,
  height: 210,
  units: "CO2 (ppm)",
  minValue: 40,
  maxValue: 2000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
      "0",
      "400",
      "800",

      "1200",
      "1600",
      "2000"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    {
      "from": 45,
      "to": 820,
      "color": "#00FF44"
    },
    {
          "from": 840,
          "to": 970,
          "color": "#FFEA00"
      },
      {
        "from": 970,
        "to": 2000,
        "color": "#FF6200"
    }
  ],
  colorPlate: "#43484c",
  colorNumbers: "#ccc",
  colorTitle: "#ccc",
  colorUnits: "#ccc",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#2edcff",
  colorNeedleEnd: "#2edcff",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#2edcff",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

var dateUpdate;

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var temp = myObj.temperature;
      var hum = myObj.humidity;
      var CO2 = myObj.CO2;
      var date_Update=myObj.dateUpdate;
      gaugeTemp.value = temp;
      gaugeHum.value = hum;
      gaugeCO2.value = CO2;
      dateUpdate=date_Update;
      document.getElementById("latestUpdate").innerHTML = dateUpdate;
    }
  }; 
  xhr.open("GET", "/samples", true);
  xhr.send();
}

// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

if (!!window.EventSource) {
  var source = new EventSource('/sampleEvents');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_samples', function(e) {
    console.log("new_samples", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    gaugeTemp.value = myObj.temperature;
    gaugeHum.value = myObj.humidity;
    gaugeCO2.value = myObj.CO2;
    dateUpdate=myObj.dateUpdate;
    document.getElementById("latestUpdate").innerHTML = dateUpdate;
  }, false);
}