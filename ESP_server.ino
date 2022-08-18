#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"

// Set your access point network credentials
const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  bool status;
  // Start server
  server.begin();
}

char* x = "1";
void loop(){
   server.on("/C1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", x);
  });
  while(Serial.available()){
    char copy = Serial.read();
    Serial.write(copy);
    if (copy == '1' || copy == '2' || copy == '3' ){
      x[0] = copy;
    }
  }
  delay(1000);
}
