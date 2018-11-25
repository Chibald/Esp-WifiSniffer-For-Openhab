#include "WebServerTemplate.h"
//---------------------------------------------------------------------------------------------------
void WebServerSetup()
{    
  WebServer.on("/", handle_root);
  WebServer.on("/restart", handle_restart);
  WebServer.on("/save", handle_save);
  WebServer.begin();
  Serial.println("Webserver Listening.");
}
//---------------------------------------------------------------------------------------------------
void handle_root()  {
  String page = FPSTR(HTML_HEAD);
  page += FPSTR(HTML_SCRIPT);  
  if (WifiAccessPointEnabled()){
  page += FPSTR(BS_STYLE_INLINE);
  }else{
  page += FPSTR(BS_STYLE);      
  }
  String inputs = bs_input("devicename", "Device Name:", "text", Settings.Name);
  inputs += bs_input("WifiSSID", "WifiSSID:", "text", Settings.WifiSSID);
  inputs += bs_input("WifiKey", "WifiKey:", "text", Settings.WifiKey);
  inputs += bs_input("IP", "IP:", "text", IpAddress2String(Settings.IP));
  inputs += bs_input("Gateway", "Gateway:", "text", IpAddress2String(Settings.Gateway));
  inputs += bs_input("Subnet", "Subnet:", "text", IpAddress2String(Settings.Subnet));
  inputs += bs_input("DNS", "DNS:", "text", IpAddress2String(Settings.DNS));
  inputs += bs_input("OpenhabServer", "OpenhabServer URL:", "text", Settings.OpenhabServer);
  String body = FPSTR(HTML_TEMPATE);
  body.replace("{{forminputs}}" , inputs);
  page += body;
  WebServer.sendHeader("Content-Length", String(page.length()));
  WebServer.send(200, "text/html", page);
}
//---------------------------------------------------------------------------------------------------
void handle_restart(){
    ESP.restart();
}
//---------------------------------------------------------------------------------------------------
void handle_save() {
  if (WebServer.arg("devicename")!= ""){
    Serial.println("devicename: " + WebServer.arg("devicename"));
    strcpy(Settings.Name, WebServer.arg("devicename").c_str());
  }
  if (WebServer.arg("WifiSSID")!= ""){
    Serial.println("WifiSSID: " + WebServer.arg("WifiSSID"));
    strcpy(Settings.WifiSSID, WebServer.arg("WifiSSID").c_str());
  }
  if (WebServer.arg("WifiKey")!= ""){
    Serial.println("WifiKey: " + WebServer.arg("WifiKey"));
    strcpy(Settings.WifiKey, WebServer.arg("WifiKey").c_str());
  }
  if (WebServer.arg("IP")!= ""){
    Serial.println("IP: " + WebServer.arg("IP"));
    Settings.IP.fromString(WebServer.arg("IP").c_str());
  }
  if (WebServer.arg("Gateway")!= ""){
    Serial.println("Gateway: " + WebServer.arg("Gateway"));
    Settings.Gateway.fromString(WebServer.arg("Gateway").c_str());
  }
  if (WebServer.arg("Subnet")!= ""){
    Serial.println("Subnet: " + WebServer.arg("Subnet"));
    Settings.Subnet.fromString(WebServer.arg("Subnet").c_str());
  }
  if (WebServer.arg("DNS")!= ""){
    Serial.println("DNS: " + WebServer.arg("DNS"));
    Settings.DNS.fromString(WebServer.arg("DNS").c_str());
  }
  if (WebServer.arg("OpenhabServer")!= ""){
    Serial.println("OpenhabServer: " + WebServer.arg("OpenhabServer"));
    strcpy(Settings.OpenhabServer, WebServer.arg("OpenhabServer").c_str());
  }
  String page = FPSTR(HTML_HEAD);
  page += FPSTR(HTML_SCRIPT);  
  page += FPSTR(BS_STYLE);  
  String body = FPSTR(HTML_TEMPATE_MESSAGE);
  if (!saveConfig()) {
    Serial.println("Failed to save config");
    body.replace("{{message}}" , "Failed to save config.");
  } else {
    Serial.println("Config saved");
    body.replace("{{message}}" , "Data Saved.");
  }  
  page += body;
  WebServer.sendHeader("Content-Length", String(page.length()));
  WebServer.send(200, "text/html", page);  
}
//---------------------------------------------------------------------------------------------------
String bs_input(String name, String label, String type, String inputvalue)
{
  String input_name = FPSTR(BS_INPUT);
  input_name.replace("{{inputname}}", name);
  input_name.replace("{{inputlabel}}", label);
  input_name.replace("{{type}}", type);
  input_name.replace("{{inputvalue}}", inputvalue);  
  return input_name;
}
//---------------------------------------------------------------------------------------------------
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}