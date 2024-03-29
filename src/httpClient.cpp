#include "httpClient.h"

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

String IpAddress2String(const IPAddress& ipAddress)
{
  //Return IPAddress class in String format
  return String(ipAddress[0]) + String(".") +\
    String(ipAddress[1]) + String(".") +\
    String(ipAddress[2]) + String(".") +\
    String(ipAddress[3])  ; 
}

uint8_t sendHttpRequest(boolean debugModeOn, IPAddress server, uint16_t port, String httpRequest) {
  //Sending httpRequest
  //boolean debugModeOn=false;

  if (debugModeOn) {Serial.print("\n[sendHttpRequest] - Trying connection to ");Serial.print(IpAddress2String(server));Serial.print(" to send httpRequest: '");Serial.print(httpRequest);Serial.println("'");}
  if (client.connect(server, 80)) {
    if (debugModeOn) {Serial.println("[sendHttpRequest] - connected");}
    // Send a HTTP request:
    client.println(httpRequest);
    client.print("Host: "); client.println(IpAddress2String(server));
    client.println("User-Agent: Arduino/1.0");
    client.println("Accept-Language: es-es");
    client.println("Connection: close");
    client.println();
  }
  else {
    errorsSampleUpts++;  //Something went wrong. Update error counter for stats
    if (debugModeOn) {Serial.println("[sendHttpRequest] - Not connected, errorsSampleUpts="+String(errorsSampleUpts));}
  }
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  u_long nowMilliseconds,lastMilliseconds=millis();
  CloudSyncCurrentStatus=CloudSyncOnStatus;
  while (!client.available()){
    nowMilliseconds=millis();
    if (nowMilliseconds>=lastMilliseconds+HTTP_ANSWER_TIMEOUT) {
      //Too long with no server answer. Something was wrong. Changing icon
      CloudSyncCurrentStatus=CloudSyncOffStatus;
      if (debugModeOn) {Serial.println("NO Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
      break;
    }
  }; //wait till there is server answer

  if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
    if (debugModeOn) {Serial.println("Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
    while (client.available()) {
      char c = client.read();
      if (debugModeOn) {Serial.write(c);}
    }
    if (debugModeOn) {Serial.println("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  }
  else errorsSampleUpts++;  //Something went wrong. Update error counter for stats

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (debugModeOn) {Serial.println("[sendHttpRequest] - Disconnecting from server. Bye!");}
    client.stop();
  }
  
  return 0;
}

uint32_t sendAsyncHttpRequest(boolean debugModeOn, boolean fromSetup, uint32_t error_setup, IPAddress server, 
                             uint16_t port, String httpRequest, uint64_t* whileLoopTimeLeft) {
  //Sending Async httpRequest
  // Parameters:
  // - logOn: whether to print logs out or not
  //      Value false: logs are printed out
  //      Value true:  logs are not printed out
  // - fromSetup: where the function was called from. Diferent prints out are done base on its value
  //      Value false: from main loop
  //      Value true:  from the firstSetup() function
  // - server: IP address of the HTTP server where to send the HTTP Request to
  // - port: HTTP Server's Port number
  // - httpRequest: HTTP request with no HTTP headers. They are set in here.
  // - *whileLoopTimeLeft is the pointer for the time of while() loop to stop checking the HTTP 
  //   connection try. It's sent from the main loop
  //      Value HTTP_ANSWER_TIMEOUT: The HTTP connection will start from the beginig - First call to the funcion
  //      Value other: The HTTP connection monitoring will resume the last try at the same point where the previous interaction
  //          was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  // *whileLoopTimeLeft is a global variable. It is modified in here. The calling function
  //   just sends it to this function.
  
  if (debugModeOn) {
    Serial.print("\n"+String(loopStartTime+millis())+" [sendAsyncHttpRequest] - Trying connection to ");Serial.print(IpAddress2String(server));Serial.print(" to send httpRequest: '");Serial.print(httpRequest);Serial.println("'");
    Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] - forceWEBCheck="+String(forceWEBCheck)+", forceWEBTestCheck="+String(forceWEBTestCheck)+", webResuming="+String(webResuming));
  }

  //to avoid pointer issues
  if (whileLoopTimeLeft==nullptr) {
    errorsSampleUpts++; //Something went wrong. Update error counter for stats
    webServerError1++;
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest]  - webServerError1="+String(webServerError1)+" - Returning with ERROR_CLOUD_SERVER");}
    return(ERROR_CLOUD_SERVER);
  }
  
  CloudSyncStatus previousCloudSyncCurrentStatus=CloudSyncCurrentStatus;
  CloudSyncCurrentStatus=CloudSyncOffStatus;

  if (!webResuming) { //To avoid repeating sending the same http request if resuming after ABORT or BREAK
    if (client.connect(server, 80)) {
      if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] - connected");}
      // Send a HTTP request:
      client.println(httpRequest);
      client.print("Host: "); client.println(IpAddress2String(server));
      client.println("User-Agent: Arduino/1.0");
      client.println("Accept-Language: es-es");
      client.println("Connection: close");
      client.println();
    }
    else {
      errorsSampleUpts++;  //Something went wrong. Update error counter for stats
      webServerError2++;
      if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;  //To avoid resuming connection the next loop interacion       
      forceWEBCheck=false;
      webResuming=false;
      errorCloudServer=true;
      if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] - Not connected, errorsSampleUpts="+String(errorsSampleUpts));}
      if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest]  - webServerError2="+String(webServerError2)+" - Returning with ERROR_CLOUD_SERVER");}
      return(ERROR_CLOUD_SERVER); //not WEB server connection
    }
  }

  uint64_t whileStartTime=loopStartTime+millis(),auxTime=whileStartTime;
  if (*whileLoopTimeLeft>=HTTP_ANSWER_TIMEOUT) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;

  if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest]    - whileStartTime="+String(whileStartTime)+", *whileLoopTimeLeft="+String(*whileLoopTimeLeft)+", waiting for HTTP client.available()");}
  while ( !client.available() && *whileLoopTimeLeft<=HTTP_ANSWER_TIMEOUT) {
    *whileLoopTimeLeft=*whileLoopTimeLeft-(loopStartTime+millis()-auxTime);
    auxTime=loopStartTime+millis();
    delay(50);
  
    //Check if buttons are pressed during WEB connection handshake (if not in the first Setup)
    if (!fromSetup) {
      switch (checkButtonsActions(webcheck)) {
        case 1:
        case 2:
        case 3:
          //Button1 or Button2 pressed or released. WEB Connection process aborted if not Sync is completed
          if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest]  - checkButtonsActions() returns 1, 2 or 3 - Returning with ERROR_ABORT_WEB_SETUP");}
          if (!client.available()) { //Actions required as the process is aborted
            forceWEBCheck=true; //Let's grant WEB check again in the next loop interaction
            webResuming=true;
            CloudSyncCurrentStatus=previousCloudSyncCurrentStatus;//Restore Cloud Clock status 
            return(ERROR_ABORT_WEB_SETUP);
          }
        break;
        case 0:
        default:
          //Regular exit. Do nothing else
        break;
      }
      //Must the Display be refreshed? This check avoids the display gets blocked during WEB connection
      if (digitalRead(PIN_TFT_BACKLIGHT)!=LOW &&
          (
            (((loopStartTime+millis()-lastTimeDisplayModeCheck) >= DISPLAY_MODE_REFRESH_PERIOD) && currentState==displayingSequential)
            ||
            (((loopStartTime+millis()-lastTimeDisplayCheck) >= DISPLAY_REFRESH_PERIOD) && 
              (currentState==displayingSampleFixed || currentState==displayingCo2LastHourGraphFixed || 
              currentState==displayingCo2LastDayGraphFixed || currentState==displayingSequential) )
          )
        ) {
        forceWEBCheck=true;
        webResuming=true;
        CloudSyncCurrentStatus=previousCloudSyncCurrentStatus;//Restore Cloud Clock status - v1.5.0
        return (ERROR_BREAK_WEB_SETUP); //Returns to refresh the display
      }
    }
  } //end while() loop

  if (*whileLoopTimeLeft>HTTP_ANSWER_TIMEOUT) { //Case if while() loop timeout.
    //Too long with no server answer. Something was wrong. Changing icon
    if ((debugModeOn && fromSetup) || debugModeOn) {
      Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] NO Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
  }
  else { 
    //End of while() due to successful WEB sync
    // (Button Pressed or Display Refresh force the function to return from the while() loop, 
    //  so this point is not reached in those cases)
    if ((debugModeOn && fromSetup) || debugModeOn) {
      Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
    while (client.available()) {//Should be very fast.
      char c = client.read();
      if (debugModeOn) {Serial.write(c);}
    }
    //CloudSyncCurrentStatus=CloudSyncOnStatus;
    CloudSyncCurrentStatus=CloudSyncSendStatus; //v1.5.0
    if (debugModeOn) {Serial.println("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  }

  //This point is reached if either the while() loop timed out or successful WEB sync
  // (Button Pressed or Display Refresh force the function to return from the while() loop, 
  //  so this point is not reached in those cases)
  
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] - Disconnecting from server. Bye!");}
    client.stop();
  }
  
  if (CloudSyncCurrentStatus==CloudSyncOffStatus) {
    //Case for while loop timeout (no successfull WEB connection)
    errorsSampleUpts++; //Something went wrong. Update error counter for stats   
    webServerError3++;
    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    forceWEBCheck=false;
    webResuming=false;
    errorCloudServer=true;
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest]  - webServerError3="+String(webServerError3)+" - Returning with ERROR_CLOUD_SERVER");}
    return(ERROR_CLOUD_SERVER); //not WEB server connection
  }
  else {
    //Case for successfull WEB connection
    if (debugModeOn) {
        Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest]    - CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", errorsSampleUpts="+String(errorsSampleUpts));
        Serial.print(String(loopStartTime+millis())+" [sendAsyncHttpRequest] - Exit - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");
    }

    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    if (fromSetup) {
      forceWEBCheck=false;
      webResuming=false;
      return(error_setup); //return previous error - Only for firstSetup()
    }
    else {
      forceWEBCheck=false;
      webResuming=false;
      return(NO_ERROR);
    }
  }
}

uint32_t checkURL(boolean debugModeOn,boolean fromSetup,uint32_t error_setup,IPAddress server,uint16_t port,String httpRequest) {
  //Sending httpRequest
  // Parameters:
  // - logOn: whether to print logs out or not
  //      Value false: logs are printed out
  //      Value true:  logs are not printed out
  // - fromSetup: where the function was called from. Diferent prints out are done base on its value
  //      Value false: from main loop
  //      Value true:  from the firstSetup() function
  // - server: IP address of the HTTP server where to send the HTTP Request to
  // - port: HTTP Server's Port number
  // - httpRequest: HTTP request with no HTTP headers. They are set in here.
  uint64_t timeLeft=0;
  
  if (debugModeOn) {Serial.print(String(loopStartTime+millis())+" [checkURL] - Trying connection to ");Serial.print(IpAddress2String(server));Serial.print(" to send httpRequest: '");Serial.print(httpRequest);Serial.println("'");}

  if (client.connect(server, port)) {
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [checkURL] - connected");}
    // Send a HTTP request:
    client.println(httpRequest);
    client.print("Host: "); client.println(IpAddress2String(server));
    client.println("User-Agent: Arduino/1.0");
    client.println("Accept-Language: es-es");
    client.println("Connection: close");
    client.println();
  }
  else {
    if (debugModeOn) {Serial.print(String(loopStartTime+millis())+" [checkURL] - No server connection. ERROR_WEB_SERVER - Exit - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
    return(ERROR_WEB_SERVER); //No WEB server connection
  }

  if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [checkURL]    - waiting for HTTP client.available()");}
  timeLeft=HTTP_ANSWER_TIMEOUT;
  while ( !client.available() && timeLeft>0) {
    delay(50);
    timeLeft-=50;
    if (timeLeft>HTTP_ANSWER_TIMEOUT) timeLeft=0; //Negative value
  
    //Check if buttons are pressed during WEB connection handshake
    switch (checkButtonsActions(webcheck)) {
      case 1:
      case 2:
      case 3:
        //Button1 or Button2 pressed or released. WEB Connection process aborted if not Sync is completed
        if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [checkURL]  - checkButtonsActions() returns 1, 2 or 3 - Returning with ERROR_ABORT_WEB_SETUP");}
        client.stop();
        return(ERROR_ABORT_WEB_SETUP);
      break;
      case 0:
      default:
        //Regular exit. Do nothing else
      break;
    }
  } //end while() loop

  if (timeLeft==0) { //Case if while() loop timeout.
    //Too long with no server answer. Something was wrong
    if (!client.connected()) client.stop();
    if (debugModeOn) {Serial.print(String(loopStartTime+millis())+" [checkURL] - No server connection. ERROR_WEB_SERVER - Exit - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
    return(ERROR_WEB_SERVER); //No WEB server connection
  }
  else { 
    //End of while() due to successful WEB sync
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [checkURL] Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
    while (client.available()) {//Should be very fast.
      char c = client.read();
      if (debugModeOn) {Serial.write(c);}
    }
    if (debugModeOn) {Serial.println("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  }

  //This point is reached everthing is right
  
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (debugModeOn) {Serial.println(String(loopStartTime+millis())+" [checkURL] - Disconnecting from server. Bye!");}
    client.stop();
  }

  if (debugModeOn) {Serial.print(String(loopStartTime+millis())+" [checkURL] - GOT URL. NO_ERROR - Exit - Time: ");getLocalTime(&nowTimeInfo);Serial.println(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
  return(NO_ERROR); //WEB server connection fine
  //return(ERROR_WEB_SERVER); //WEB server connection fine - Test
}