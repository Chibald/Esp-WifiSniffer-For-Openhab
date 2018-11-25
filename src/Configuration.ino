#define CONFIG_FILENAME  "/config.json"
//---------------------------
void InitSettings()
{
  showState = millis();
  Serial.println(F("AP Settings"));
  WifiAPconfig();

  Serial.println("");
  Serial.println(F("Mounting filesystem..."));
  
  if (!SPIFFS.begin()) {
    Serial.println(F("Failed to mount filesystem"));
    format();
    return;
  }

  if (!loadConfig()) {
    Serial.println(F("Failed to load config file"));
    format();
    strcpy(Settings.Name, "Snufferhondje"); 
    Settings.WifiSSID[0] = 0;   
    configmode = true;
  } else {
    Serial.println(F("Config loaded"));
  }
}
// ----------------------------------------------------------------------------------- 
bool loadConfig() {
  File configFile = SPIFFS.open(CONFIG_FILENAME, "r");

  if (!configFile) {
    Serial.println(F("Failed to open config file"));
    return false;
  }
  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println(F("Config file size is too big"));
    return false;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  configFile.close();
  if (!json.success()) {
    Serial.println(F("Failed to parse config file"));
    return false;
  } 
  strcpy(Settings.Name, (const char*)json["Name"]);
  strcpy(Settings.WifiSSID, (const char*)json["WifiSSID"]);
  strcpy(Settings.WifiKey, (const char*)json["WifiKey"]);
  strcpy(Settings.OpenhabServer, (const char*)json["OpenhabServer"]);
  
  const char *clientipch = json["ip"];
  const char *subnetch = json["subnet"];
  const char *gatewaych = json["gateway"];
  const char *dnsch = json["dns"];

  Settings.IP.fromString(clientipch);
  Settings.Gateway.fromString(gatewaych);
  Settings.Subnet.fromString(subnetch);
  Settings.DNS.fromString(dnsch);

  Serial.print("Loaded WifiSSID: ");
  Serial.println(Settings.WifiSSID);
  Serial.print("Loaded WifiKey: ");
  Serial.println(Settings.WifiKey); 

  return true;
}
// ----------------------------------------------------------------------------------- 
bool saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  String inputs = bs_input("devicename", "Device Name:", "text", Settings.Name);
  inputs += bs_input("WifiSSID", "WifiSSID:", "text", Settings.WifiSSID);
  inputs += bs_input("WifiKey", "WifiKey:", "text", Settings.WifiKey);
  inputs += bs_input("IP", "IP:", "text", IpAddress2String(Settings.IP));
  inputs += bs_input("Gateway", "Gateway:", "text", IpAddress2String(Settings.Gateway));
  inputs += bs_input("Subnet", "Subnet:", "text", IpAddress2String(Settings.Subnet));
  inputs += bs_input("DNS", "DNS:", "text", IpAddress2String(Settings.DNS));
  inputs += bs_input("OpenhabServer", "OpenhabServer URL:", "text", Settings.OpenhabServer);

  json["Name"] = Settings.Name; 
  json["WifiSSID"] = Settings.WifiSSID; 
  json["WifiKey"] = Settings.WifiKey;
  json["OpenhabServer"] = Settings.OpenhabServer;

  json["ip"] = IpAddress2String(Settings.IP);
  json["subnet"] = IpAddress2String(Settings.Subnet); 
  json["gateway"] = IpAddress2String(Settings.Gateway); 
  json["dns"] = IpAddress2String(Settings.DNS);

  File configFile = SPIFFS.open(CONFIG_FILENAME, "w");

  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    format();
    return false;
  }
  json.printTo(configFile);
  configFile.close();

  return true;
}
// ----------------------------------------------------------------------------------- 
bool format()
{
  SPIFFS.end();
  Serial.println(F("formatting..."));
  SPIFFS.format();
  Serial.println(F("formatting done..."));
  if (!SPIFFS.begin())
  {
    Serial.println(F("Format spiffs failed."));
    return false;
  }
  return true;
}
// ----------------------------------------------------------------------------------- 
void Diag()
{
  uint32_t heap = ESP.getFreeHeap();
  if(heap < 4096)
  {
    Serial.print(F("Heap is dangerously low. This can happen if updating REST failes too many times, this causes memory to leak. Going to retart.")); 
    Serial.print(F("If this problem persists, check conncection to WIFI station, or debug my code :)")); 
    ESP.restart();
  }
  if(millis() - showState > SENDTIME)
  { 
    showState = millis();
    Serial.print(F("Memory free: ")); 
    Serial.println(heap);
    Serial.print(F("Connection Failures: "));
    Serial.println(ConnectionFailureCount);    
  }
}