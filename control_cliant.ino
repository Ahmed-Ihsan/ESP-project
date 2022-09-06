#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;


const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

//Your IP address or domain name with URL path
const char* serverData = "http://192.168.4.1/C1";

//varuible
String Data;
int counter_off_wind = 0;

// pins serail Data 
const int soler_power = D3 ;
const int tower = D2;
const int wind = A0 ;
const int relay = D4;
const int battery1 = D1;
const int battery2 = D0;


void setup() {
  Serial.begin(115200);
  pinMode(wind, INPUT);
  pinMode(soler_power, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(tower, OUTPUT);
  pinMode(battery1, OUTPUT);
  pinMode(battery2, OUTPUT);
  
  Serial.println(); 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
}

void loop() {
  unsigned long currentMillis = millis();
  
     // Check WiFi connection status
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      Data = httpGETRequest(serverData);
      Serial.println("Data: " + Data );
      
      if (Data == "1")
        case1();
      else if (Data == "2")
        case2();
      else if (Data == "3")
        case3();
    
    }
    else {
      Serial.println("WiFi Disconnected");
      WiFi.begin(ssid, password);
      Serial.print("Connecting to ");
      Serial.println(ssid);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("Connected to WiFi");
    }
 }

void case1(){
  digitalWrite(tower,LOW); // on 
  digitalWrite(soler_power,LOW); // on
  digitalWrite(battery1,HIGH);
  digitalWrite(battery2,HIGH);
  run_wind_tower();
}

void case2(){
  digitalWrite(battery1,LOW);
  digitalWrite(battery2,LOW);
  digitalWrite(tower,HIGH); 
  digitalWrite(soler_power,HIGH); 
  run_wind_tower();
}

void case3(){
  digitalWrite(soler_power,LOW); 
  digitalWrite(battery1,LOW);
  digitalWrite(battery2,HIGH);
  digitalWrite(tower,LOW); 
  run_wind_tower();
}

void run_wind_tower(){
  int on_win = check_wind();
  if (on_win == 1) {
    Serial.print("on wind tower :");
    Serial.println(on_win);
    digitalWrite(relay,LOW);//ON
  } else {
    Serial.print("off wind tower :");
    Serial.println(on_win);
    digitalWrite(relay,HIGH);//OFF
  }
}

int wind_power(){
  int IO = analogRead(wind);
  int on_wind = 0 ;
  if(IO >= 16){
    on_wind = 1;
  }
  else{
    on_wind = 0;
  }
  return on_wind;
}

int check_wind(){
  int check_power = wind_power();
  if(check_power == 0){
    if(counter_off_wind == 50){
      return 0;
    }else{
      counter_off_wind += 1;
      return 1;
    }
  }else{
    counter_off_wind = 0;
    return 1;
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 

    
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
