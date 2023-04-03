/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "secrets.h"


const char* ssid = STASSID;
const char* password = STAPSK;

X509List cert(URL_CERT);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  String requestBody = prepareRequestBody(random(2),random(50),random(50));
  Serial.print(sendHttpRequest("POST", URL_HOST, URL_PORT,URL_PATH,URL_AUTH,requestBody)); 
    
}
String prepareRequestBody(long isMotionDetected,long temperature,long humidity){
  String response = String("{")
  +"\"isMotionDetected\":"+isMotionDetected
  +",\"temperature\":"+temperature
  +",\"humidity\":"+humidity
  +"}";
  return response;
}
String sendHttpRequest(String method, String host,uint16_t port, String path,String auth,String requestBody){
  WiFiClientSecure client;
  
  Serial.print("Connecting to ");
  Serial.println(URL_HOST);

  Serial.printf("Using certificate: %s\n", URL_CERT);
  client.setTrustAnchors(&cert);

  if (!client.connect(URL_HOST, URL_PORT)) {
    Serial.println("Connection failed");
    return "";
  }

  String httpRequest = String(method)+" " + path + " HTTP/1.1\r\n" 
  + "Host: " + host + "\r\n" 
  + "User-Agent: ESP8266_"+WiFi.macAddress()+"\r\n" 
  + "Authorization: "+auth+"\r\n"
  + "Content-Type: application/json\r\n"
  + "Content-Length: "+requestBody.length()+"\r\n"
  + "Connection: close\r\n"
  + "\r\n"
  +requestBody;

  client.print(httpRequest);
  Serial.println("Request sent \r\n"+httpRequest);
  
  while (client.available()) {
  // Serial.println("Response received1: ");
  // Serial.println(client.readString());
  
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }
  Serial.println("Response received2: ");
  Serial.println("Closing connection");
  return client.readString();
  
}
void loop() {}
