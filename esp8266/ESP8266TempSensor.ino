/*
 *  This sketch demonstrates how to set up a simple IoT Temperature server.
 *  The server will show the current temperature
 *  During the setup the server will act as AP mode so that you can set the
 *  Wifi connection, then it will try to connect to the WiFi.
 *  - Temperature sensor LM35 is connected to Pin A0
 *  - While serving the webserver and showing the temperature, it will also post
 *    the temperature to external website, so that the temperature can be
 *    retrieved without having to open port to your router.
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <TimeLib.h>

#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>


#include "settings.h"
#include "mainPage.h"

#define DebugStart() Serial.begin(115200)
#define DebugLn(s) Serial.println((s))
#define Debug(s) Serial.print((s))
#define SETUP_PIN 0

String webserver = "webserver.com";      // webserver where the cloud is hosted
String weburi = "/data_store.php"; // script to post the temperature point to data_store.php
String data;

//void setupWiFi(char checkAPMode);
//time_t getNtpTime(void);
//void sendNTPpacket(WiFiUDP *u);
// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

String httpUpdateResponse;


void displayAPWait(){
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}

void stopAnimation(int status){
  digitalWrite(LED_BUILTIN, status);
}

void displayBusy(){
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}

float temperature=0;

void handleRoot() {
  DebugLn("handleRoot");
  String s = MAIN_page;
  time_t t = now();
  s.replace("@@SSID@@", settings.ssid);
  s.replace("@@PSK@@", settings.psk);
  s.replace("@@TZ@@", String(settings.timezone));
  s.replace("@@USDST@@", settings.usdst?"checked":"");
  s.replace("@@HOUR@@", String(hour(t)));
  s.replace("@@MIN@@", String(minute(t)));
  s.replace("@@NTPSRV@@", settings.timeserver);
  s.replace("@@NTPINT@@", String(settings.interval));
  s.replace("@@SYNCSTATUS@@", timeStatus() == timeSet ? "OK" : "Overdue");
  s.replace("@@CLOCKNAME@@", settings.name);
  s.replace("@@TEMP@@", String(temperature));
  s.replace("@@UPDATERESPONSE@@", httpUpdateResponse);
//  s.replace("@@MODE@@", getMode());
  httpUpdateResponse = "";
  server.send(200, "text/html", s);
}

double analogValue = 0.0;
double analogVolts = 0.0;
int timer =0;

void handleIO() {
  /*while (timer < 120){
    timer ++;
    return;
  }
  timer = 0;*/
  DebugLn("handleIO");
  /*String httpPost = "POST " + weburi + "HTTP/1.0\r\n" +
          "Host: "+ webserver + "\r\n" +
          "Accept: *" + "/" + "*\r\n" +  
          "Content-Length: " + data.length() + "\r\n" +
          "Content-Type:application/x-www-form-urlencoded\r\n" +
          "\r\n" + data;*/
  analogValue = analogRead(A0);
  displayBusy();
   // convert the analog signal to voltage
  // the ESP2866 A0 reads between 0 and ~3 volts, producing a corresponding value
  // between 0 and 1024. The equation below will convert the value to a voltage value.
  analogVolts = (analogValue * 3.03) /1024;
  DebugLn(String(analogVolts));
  temperature = (analogVolts) * 100; //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
 
   data = "temp_sensor=" + String(temperature);
  DebugLn(data);
  String httpPost = "http://"+webserver+weburi;
  HTTPClient http;
  DebugLn(httpPost);
  http.begin(httpPost);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  DebugLn(data);
  int httpCode = http.POST(data);
  String payload = http.getString();

  DebugLn(httpCode);    // this return 200 when success
  DebugLn(payload);     // this will get the response
  http.end();
}

void handleForm() {
  DebugLn("handleForm");
  DebugLn("mode "+String(WiFi.status()));
  String update_wifi = server.arg("update_wifi");
  String t_ssid = server.arg("ssid");
  String t_psk = server.arg("psk");
  String t_timeserver = server.arg("ntpsrv");
  t_timeserver.toCharArray(settings.timeserver, EEPROM_TIMESERVER_LENGTH, 0);
  if (update_wifi == "1") {
    settings.ssid = t_ssid;
    settings.psk = t_psk;
  }
  String tz = server.arg("timezone");

  if (tz.length()) {
    settings.timezone = tz.toInt();
  }

  String usdst = server.arg("usdst");
  settings.usdst = (usdst == "1");

 // time_t newTime = getNtpTime();
//  if (newTime) {
 //   setTime(newTime);
//  }
  String syncInt = server.arg("ntpint");
  settings.interval = syncInt.toInt();

  settings.name = server.arg("clockname");
  settings.name.replace("+", " ");

  httpUpdateResponse = "The configuration was updated.";

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Moved");

  settings.Save();
  
  if (update_wifi == "1") {
    delay(500);
    setupWiFi(0);             // connect to Wifi
  }
}

void setup() {
  DebugStart();
  pinMode(LED_BUILTIN, OUTPUT);
  setupWiFi(1);               // SetupAP mode
  server.on("/", handleRoot);
  server.on("/form", handleForm);
  //server.on("/io", handleIO);
  server.begin();


  // prepare GPIO2
  //pinMode(2, OUTPUT);
  //digitalWrite(2, 0);
    
}

void loop() {
  static unsigned long lastSetupPin = 0;
  
  delay(100);
  if (timer > 1000){
    timer = 0;
    handleIO();
  }else{                  // handle http client while waiting
    timer++;
    server.handleClient();    
  }

}

void setupWiFi(char checkAPMode) {
  settings.Load();
  if (checkAPMode) {
    // Wait up to 5s for SETUP_PIN to go low to enter AP/setup mode.
    pinMode(SETUP_PIN, INPUT);
    digitalWrite(SETUP_PIN, HIGH);
    displayAPWait();
    long start = millis();
    DebugLn("Started at "+String(start));
    while (millis() < start + 5000) {
      if (!digitalRead(SETUP_PIN) || !settings.ssid.length()) {
        DebugLn("Setting up AP");
        //stopAnimation();
        setupAP();
        DebugLn("Done with AP");
        return;
      }
      delay(50);
    }
    //stopAnimation();
  }
  setupSTA();
}
int timeOut=0;
void setupSTA()
{
  char ssid[32];
  char psk[64];

  memset(ssid, 0, 32);
  memset(psk, 0, 64);
  

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  settings.ssid.toCharArray(ssid, 32);
  settings.psk.toCharArray(psk, 64);
  if (settings.psk.length()) {
    WiFi.begin(ssid, psk);
  } else {
    WiFi.begin(ssid);
  }
  
  DebugLn("Connecting to "+String(ssid));
  while (WiFi.status() != WL_CONNECTED) {
    if (timeOut < 10){          // if not timeout, keep trying
      delay(100);
      Debug(String(WiFi.status()));
      displayBusy();
      timeOut ++;
    } else{
      timeOut = 0;
      Debug("Wifi connection timeout");
      settings.ssid = "";     // clear the ssid and psk if timeout
      settings.psk = "";
      settings.Save();
      break;
    }
  }
  DebugLn("Connected");
// Print the IP address
  DebugLn(WiFi.localIP());  
  //stopAnimation(HIGH);
 // ntpActive = 1;
}

void setupAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(WIFI_AP_NAME);
}
