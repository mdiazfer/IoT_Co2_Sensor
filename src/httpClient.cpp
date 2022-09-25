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

uint8_t sendHttpRequest(IPAddress server, uint16_t port, String httpRequest) {
  //Sending httpReques
  boolean logsOn=false;

  if (client.connect(server, 80)) {
    if (logsOn) {Serial.print("\n[sendHttpRequest] - connected to ");Serial.print(IpAddress2String(server));Serial.print(" to send httpRequest: '");Serial.print(httpRequest);Serial.println("'");}
    // Send a HTTP request:
    client.println(httpRequest);
    client.print("Host: "); client.println(IpAddress2String(server));
    client.println("User-Agent: Arduino/1.0");
    client.println("Accept-Language: es-es");
    client.println("Connection: close");
    client.println();
  }

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (!client.available()); //wait till there is server answer
  if (logsOn) {Serial.println("Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  while (client.available()) {
    char c = client.read();
    if (logsOn) {Serial.write(c);}
  }
  if (logsOn) {Serial.println("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (logsOn) {Serial.println("[sendHttpRequest] - Disconnecting from server. Bye!");}
    client.stop();
  }
  
  return 0;
}