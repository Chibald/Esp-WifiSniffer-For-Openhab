#include <FS.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <set>
#include <string>
#include "./functions.h"
#include <ESP8266WebServer.h>
#include <esp8266httpclient.h>

#define disable 0
#define enable  1
#define MAXDEVICES 80       // max device count to hold in memory
#define PURGETIME 60000     // milisec to purge from memory
#define MINRSSI -150        // RSSI sensitivity for sniffing Wifi       
#define MAX_OH_ITEMS   5    // Maximum number of openhab items to monitor. 
#define SENDTIME  15000     // time to send to openhab
#define REBOOTTIME  86400000   // time to reboot esp to refresh list
unsigned int channel = 2;   // Wifi channel for AP mode
unsigned long sendEntry, showState;

int nbrDevices = 0;
int usedChannels[15];

const short int PIN_GPIO0 = 0; //GPIO0 hold for getting into config mode
bool configmode = false;       //config mode. 
bool configmodeauto = false;   //went to config mode automatic? 
bool extensiveLogging = true; //logging level. set to true to get a lot of serial info.
// ----------------------------------------------------------------------------------- 
// WebServer
ESP8266WebServer WebServer(80);
HTTPClient http;
// ----------------------------------------------------------------------------------- 
struct SettingsStruct
{
   char          Name[26];
   char          WifiSSID[32];
   char          WifiKey[64];
   IPAddress          IP;
   IPAddress          Gateway;
   IPAddress          Subnet;
   IPAddress          DNS;  
   char          OpenhabServer[129];
} Settings;
// ----------------------------------------------------------------------------------- 
struct OpenHabItem
{
   char          Name[32];
   char          MacAddress[64];  
} OpenHabItem[MAX_OH_ITEMS];
int OhItems = 0;
bool OpenHabItemsInitialized = false;
// ----------------------------------------------------------------------------------- //
bool SoftAPIntitialized = false;
unsigned int ConnectionFailureCount = 0;
unsigned int ConnectionOKCount = 0;
// ----------------------------------------------------------------------------------- //
void setup() 
{
  Serial.begin(115200);
  Serial.println("");
  
  // If esp keeps rebooting after save confguration, SPIFFS might have become corrupted. Upload sketch and format spiffs.
  // Afterwards, re-upload with format commented again.
  // format();
  
  InitSettings();
  Serial.println(Settings.Name);
  wifi_set_channel(channel);
  wifi_set_opmode(STATION_MODE);            // Promiscuous works only with station mode
  wifi_promiscuous_enable(disable);
  wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
  wifi_promiscuous_enable(enable);
  WebServerSetup();
  pinMode(PIN_GPIO0, INPUT_PULLUP);
}
// ----------------------------------------------------------------------------------- 
void loop() 
{ 
  if(digitalRead(PIN_GPIO0) == 0 && !configmode) // GPIO0 Pull DOwn.
  { 
    Serial.print(F("Going Into Config Mode "));
    configmode = true;
  } 
  if (configmode) {  
    if(!WifiCheck())
      connectToWiFi();    
    WebServer.handleClient();
  }
  if(!OpenHabItemsInitialized){
      ConnectOpenHab();      
    }else if (!configmode){
      scanDevices();
      boolean openhabItemsToSend = showDevices();
      if(openhabItemsToSend)
        SendItemsToOpenHab();
  }
  Diag();
}
// ----------------------------------------------------------------------------------- 