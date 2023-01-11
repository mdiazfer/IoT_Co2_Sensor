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

uint8_t sendHttpRequest(boolean logsOn, IPAddress server, uint16_t port, String httpRequest) {
  //Sending httpRequest
  //boolean logsOn=false;

  if (logsOn) {Serial.print("\n[sendHttpRequest] - Trying connection to ");Serial.print(IpAddress2String(server));Serial.print(" to send httpRequest: '");Serial.print(httpRequest);Serial.println("'");}
  if (client.connect(server, 80)) {
    if (logsOn) {Serial.println("[sendHttpRequest] - connected");}
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
    if (logsOn) {Serial.println("[sendHttpRequest] - Not connected, errorsSampleUpts="+String(errorsSampleUpts));}
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
      if (logsOn) {Serial.println("NO Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
      break;
    }
  }; //wait till there is server answer

  if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
    if (logsOn) {Serial.println("Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
    while (client.available()) {
      char c = client.read();
      if (logsOn) {Serial.write(c);}
    }
    if (logsOn) {Serial.println("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  }
  else errorsSampleUpts++;  //Something went wrong. Update error counter for stats

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (logsOn) {Serial.println("[sendHttpRequest] - Disconnecting from server. Bye!");}
    client.stop();
  }
  
  return 0;
}

uint8_t sendAsyncHttpRequest(boolean logsOn, boolean fromSetup, uint8_t error_setup, IPAddress server, 
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
  //   just send it to this function.
  
  if (logsOn) {
    Serial.print("\n"+String(loopStartTime+millis())+" [sendAsyncHttpRequest] - Trying connection to ");Serial.print(IpAddress2String(server));Serial.print(" to send httpRequest: '");Serial.print(httpRequest);Serial.println("'");
    Serial.println(String(loopStartTime+millis())+" [sendAsyncHttpRequest] - forceWEBCheck="+String(forceWEBCheck)+", forceWEBTestCheck="+String(forceWEBTestCheck)+", webResuming="+String(webResuming));
  }

  //to avoid pointer issues
  if (whileLoopTimeLeft==nullptr) {
    errorsSampleUpts++; //Something went wrong. Update error counter for stats
    return(ERROR_WEB_SERVER);
  }
  
  CloudSyncStatus previousCloudSyncCurrentStatus=CloudSyncCurrentStatus;
  CloudSyncCurrentStatus=CloudSyncOffStatus;

  if (!webResuming) { //To avoid repeating sending the same http request if resuming after ABORT or BREAK
    if (client.connect(server, 80)) {
      if (logsOn) {Serial.println("[sendAsyncHttpRequest] - connected");}
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
      if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;  //To avoid resuming connection the next loop interacion       
      forceWEBCheck=false;
      webResuming=false;
      if (logsOn) {Serial.println("[sendAsyncHttpRequest] - Not connected, errorsSampleUpts="+String(errorsSampleUpts));}
      return(ERROR_WEB_SERVER); //not WEB server connection
    }
  }

  uint64_t whileStartTime=loopStartTime+millis(),auxTime=whileStartTime;
  if (*whileLoopTimeLeft>=HTTP_ANSWER_TIMEOUT) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;

  if (debugModeOn) {Serial.println("    - whileStartTime="+String(whileStartTime)+", *whileLoopTimeLeft="+String(*whileLoopTimeLeft)+", waiting for HTTP client.available()");}
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
          if (debugModeOn) {Serial.println(String(loopStartTime+millis())+"  - checkButtonsActions() returns 1, 2 or 3 - Returning with ERROR_ABORT_WEB_SETUP");}
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
        return (ERROR_BREAK_WEB_SETUP); //Returns to refresh the display
      }
    }
  } //end while() loop

  if (*whileLoopTimeLeft>HTTP_ANSWER_TIMEOUT) { //Case if while() loop timeout.
    //Too long with no server answer. Something was wrong. Changing icon
    if ((logsOn && fromSetup) || debugModeOn) {
      Serial.println("NO Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
  }
  else { 
    //End of while() due to successful WEB sync
    // (Button Pressed or Display Refresh force the function to return from the while() loop, 
    //  so this point is not reached in those cases)
    if ((logsOn && fromSetup) || debugModeOn) {
      Serial.println("Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
    while (client.available()) {//Should be very fast.
      char c = client.read();
      if (logsOn) {Serial.write(c);}
    }
    CloudSyncCurrentStatus=CloudSyncOnStatus;
    if (logsOn) {Serial.println("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  }

  //This point is reached if either the while() loop timed out or successful WEB sync
  // (Button Pressed or Display Refresh force the function to return from the while() loop, 
  //  so this point is not reached in those cases)
  
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (logsOn) {Serial.println("[sendAsyncHttpRequest] - Disconnecting from server. Bye!");}
    client.stop();
  }
  
  if (CloudSyncCurrentStatus==CloudSyncOffStatus) {
    //Case for while loop timeout (no successfull WEB connection)
    errorsSampleUpts++; //Something went wrong. Update error counter for stats   
    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    forceWEBCheck=false;
    webResuming=false;
    return(ERROR_WEB_SERVER); //not WEB server connection
  }
  else {
    //Case for successfull WEB connection
    if (debugModeOn) {
        Serial.println("    - CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+", errorsSampleUpts="+String(errorsSampleUpts));
        Serial.println(String(loopStartTime+millis())+"  - [sendAsyncHttpRequest] - Exit - Time:");
    }

    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=HTTP_ANSWER_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    if (fromSetup) {
      forceWEBCheck=false;
      webResuming=false;
      return(error_setup); //return previous error - Mainly for firstSetup()
    }
    else {
      forceWEBCheck=false;
      webResuming=false;
      return(NO_ERROR);
    }
  }
}