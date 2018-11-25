bool connectToWiFi() 
{
  wifi_promiscuous_enable(disable);
  delay(10);

  if(strlen(Settings.WifiSSID) == 0)
  {   
    WifiAPMode(true);     // No settings found. Set ESP to config mode
    configmode = true;     
    configmodeauto = true;
    return false;
  }

  Serial.print(F("Connecting to '"));
  Serial.print(Settings.WifiSSID);
  Serial.println(F("'"));
 
  WiFi.config(Settings.IP, Settings.Gateway, Settings.Subnet, Settings.DNS);
  WiFi.begin(Settings.WifiSSID, Settings.WifiKey);

  int connectionCounter = 0;

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(50);
    Serial.print(".");
    connectionCounter++;
    if(configmode)
      WebServer.handleClient();    
    if (connectionCounter > 1200)
         break;
  }
  bool wifistatus = WifiCheck();
  if(wifistatus){
    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
  }else{
    wifi_promiscuous_enable(enable);
    if(ConnectionFailureCount > 20){
        Serial.println(F("Start access point mode."));
        Serial.println(F("Will disable after connection is OK again."));
        configmode = true;
        configmodeauto = true;
        WifiAPMode(true);
    }
  }
  return wifistatus;
}
//---------------------------------------------------------------------------------------------------
boolean WifiDisconnect()
{
  if(WifiCheck())
    WiFi.disconnect();
  
  Serial.println(F("WiFi disconnected"));
  delay(50);
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(enable);
  return true;
}
//---------------------------------------------------------------------------------------------------
boolean WifiCheck()
{
  String log = "";
  if (WiFi.status() != WL_CONNECTED)
  {
    ConnectionFailureCount++;
    return false;    
  }
  else
  {
    if(configmodeauto) // If esp went to configmode and AP automatic (so, wifi connection failure), disable config mode.
    {
      configmode = false;
      configmodeauto = false;
      WifiAPMode(false); 
    } 
    ConnectionOKCount++;
    ConnectionFailureCount = 0;    
    return true;
  }
}
//---------------------------------------------------------------------------------------------------
bool WifiAccessPointEnabled()
{
  byte wifimode = wifi_get_opmode();
  return(wifimode == 2 || wifimode == 3); //apmode is enabled
}
//---------------------------------------------------------------------------------------------------
void WifiAPMode(boolean enableAccessPoint)
{
  if (WifiAccessPointEnabled())
  {
    if (!enableAccessPoint)
    {
      WiFi.mode(WIFI_STA);
      wifi_set_opmode(STATION_MODE);     
    }
  }
  else
  {
    if (enableAccessPoint)
    {
      if(!SoftAPIntitialized){
        Serial.println(F("Start access point mode."));
        WifiAPconfig();
      }      
      WiFi.mode(WIFI_AP_STA);        
    }
  }
}
//---------------------------------------------------------------------------------------------------
void WifiAPconfig()
{
  wifi_set_opmode(SOFTAP_MODE);
  WiFi.softAP(Settings.Name);
	Serial.print(F("AP IP address: "));
	Serial.println(WiFi.softAPIP());
}
//---------------------------------------------------------------------------------------------------