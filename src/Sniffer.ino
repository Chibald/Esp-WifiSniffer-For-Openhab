
void scanDevices()
{
  if (extensiveLogging)
    Serial.println(F("Scan Devices (Start)"));
  
  channel = 1;  
  wifi_set_channel(channel);
  while (true) 
  {
    nothing_new++;               // Array is not finite, check bounds and adjust if required
    if (nothing_new > 200)       // monitor channel for 200 ms
    {                
      nothing_new = 0;
      channel++;
      
      if (channel == 15) 
        break;             // Only scan channels 1 to 14
        
      wifi_set_channel(channel);
    }
    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
  }  
  purgeDevice();  //Remove old devices from internal DB

  if (extensiveLogging)
    Serial.println(F("Scan Devices (End)"));  
}
//---------------------------------------------------------------------------------------------------
void purgeDevice() 
{
  if (extensiveLogging)
    Serial.println(F("Purge Devices (Start)"));

  for (int u = 0; u < clients_known_count; u++) 
  {
    if ((millis() - clients_known[u].lastDiscoveredTime) > PURGETIME) 
    {
      if (extensiveLogging){
          Serial.print("Purge Client ");
          Serial.print(u);
          Serial.print(" --> ");
          Serial.println(formatMac1(clients_known[u].station));
      }
     
      for (int i = u; i < clients_known_count; i++) 
        memcpy(&clients_known[i], &clients_known[i + 1], sizeof(clients_known[i]));
        
      clients_known_count--;
      break;
    }
  }  

  for (int u = 0; u < aps_known_count; u++) 
  {
    if ((millis() - aps_known[u].lastDiscoveredTime) > PURGETIME) 
    {
      if (extensiveLogging){
        Serial.print("Purge Beacon" );
        Serial.println(u);
      }
      for (int i = u; i < aps_known_count; i++) 
        memcpy(&aps_known[i], &aps_known[i + 1], sizeof(aps_known[i]));
        
      aps_known_count--;
      break;
    }
  }
  if (extensiveLogging)
    Serial.println(F("Purge Devices (End)"));
}
//---------------------------------------------------------------------------------------------------
boolean showDevices() 
{
  if (extensiveLogging)
    Serial.println(F("Show Devices"));

  bool ohItem = false;
  if (extensiveLogging){
    Serial.println();
    Serial.println("-------------------Device DB-------------------");
    Serial.print("APs and devices: ");
    Serial.println(aps_known_count + clients_known_count);
  }
  // add Beacons
  for (int u = 0; u < aps_known_count; u++) 
  {
    if (extensiveLogging){
      Serial.print("Beacon ");
      Serial.print(formatMac1(aps_known[u].bssid));
      Serial.print(", RSSI ");
      Serial.print(aps_known[u].rssi);
      Serial.print(", channel ");
      Serial.print(aps_known[u].channel);
    }
    if(formatMac1(aps_known[u].bssid) == OpenHabItem[0].MacAddress){    
      if (extensiveLogging){
        Serial.print(" OH ");    
        Serial.println(OpenHabItem[0].Name);
      }
      ohItem = true;
    }else{
      if (extensiveLogging){ Serial.println(); }
    }
  }
  // add Clients
  for (int u = 0; u < clients_known_count; u++) 
  {
    if (extensiveLogging){
      Serial.print("Client ");
      Serial.print(formatMac1(clients_known[u].station));
      Serial.print(", RSSI ");
      Serial.print(clients_known[u].rssi);
      Serial.print(", channel ");
      Serial.print(clients_known[u].channel);
    }
    for (int h = 0; h < OhItems; h++) 
    {   
        if(formatMac1(clients_known[u].station) == OpenHabItem[h].MacAddress){ 
            if (extensiveLogging){
              Serial.print(" OH ");    
              Serial.print(OpenHabItem[h].Name);
            }
            ohItem = true;
        }
    }
    if (extensiveLogging){ Serial.println(); }
  }
  if (ohItem && millis() - sendEntry > SENDTIME) 
  {
      Serial.println(F("Time to send"));
      return true;
  }
  else return false; 
}