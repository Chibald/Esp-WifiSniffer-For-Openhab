void ICACHE_FLASH_ATTR ConnectOpenHab()
{
    if(!WifiCheck())
        if(connectToWiFi()){
            GetItemsFromOpenhab(); 
            WifiDisconnect();
        }
}
//---------------------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR GetItemsFromOpenhab ()
{
    String publish = Settings.OpenhabServer;
    publish = publish + "/rest/items?tags=Sniffer&recursive=false";
    Serial.println(F("Fetching items from openhab to monitor"));
    http.begin(publish);
    int httpCode = http.GET();                                                                 
    OhItems = 0;
    if (httpCode > 0) { 
        String payload = http.getString();                  
        DynamicJsonBuffer jsonBuffer;         
        JsonArray& arr = jsonBuffer.parseArray(payload);    
        if(arr.success()){
         for (JsonObject& elem : arr) {
                bool addeditem = false;                     
                int numberOfElements = elem["tags"].size();                  
                for (int x = 0; x < numberOfElements; x++)
                { 
                    String tagContents = "";
                    elem["tags"][x].printTo(tagContents);                    
                    tagContents.replace("\"", "");  
                    if(tagContents.startsWith("Mac:"))
                    {                        
                        tagContents = tagContents.substring(4);
                        tagContents.replace(":", "");
                        tagContents.toLowerCase();   
                        strcpy(OpenHabItem[OhItems].Name, (const char*)elem["name"]);                        
                        tagContents.toCharArray(OpenHabItem[OhItems].MacAddress, 64);
                        addeditem = true;
                        OhItems++;
                        if(OhItems >= MAX_OH_ITEMS)
                            break;
                    }
                }
                if(!addeditem){
                    Serial.print(F("Could not monitor, no MAC address tag found. Add Tag to item: syntax 'Mac:[address]' Openhab Item: ")); 
                }else{
                    Serial.print(F("Added succesfully. Openhab Item: "));                     
                }
                elem["name"].printTo(Serial);  
                Serial.println();
            }       
        }else{
            Serial.println(F("parse failed"));  
            Serial.println(payload);
        }
    }
    http.end();   //Close connection
    if(OhItems>0) {
        OpenHabItemsInitialized = true;
        Serial.print(F("Succesfully found "));  
        Serial.print(OhItems);
        Serial.println(F(" openhab items to monitor."));  
    }else
    {
        Serial.println(F("No items found to monitor."));  
    }

    
}
//---------------------------------------------------------------------------------------------------
boolean ICACHE_FLASH_ATTR UpdateOpenhabItemState(String itemName)
{ 
    Serial.println("Updating item state for item " + itemName); 
    Diag();   
    String url = Settings.OpenhabServer; 
    url += "/rest/items/";
    url += itemName; 
    url += "/state";    
    http.begin(url);      
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Length", "2");
    int response = http.sendRequest("PUT", "ON");
    http.end();
    if(response <0){         
        Serial.print("HTTP Error. Response Code: ");
        Serial.println(response);  
        Serial.print(http.errorToString(response).c_str());
        Serial.println(http.getString()); 
        return false;
    }else{
        Serial.print(F("Updated Item OK. Response code "));  
        Serial.println(response); 
        Diag();
        return true; 
    }
}
//---------------------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR SendItemsToOpenHab() 
{   
  Serial.println(F("Updating items state in Openhab")); 
  //wifi_promiscuous_enable(disable);
  if(!connectToWiFi())
  {
    Serial.println(F("Cannot connect to openhab, wifi not connected!")); 
    return;
  }

  String deviceMac = "";
  bool allOkay = true;

  // add Beacons
  for (int u = 0; u < aps_known_count; u++) 
  {
    deviceMac = formatMac1(aps_known[u].bssid);
    if (deviceMac == OpenHabItem[0].MacAddress)
    {
        UpdateOpenhabItemState(OpenHabItem[0].Name);     
    }
  }
  // Add Clients
  for (int u = 0; u < clients_known_count; u++) 
  {
    for (int h = 0; h < OhItems; h++) 
    { 
            deviceMac = formatMac1(clients_known[u].station);
            if (deviceMac == OpenHabItem[h].MacAddress)
            {
                if(!UpdateOpenhabItemState(OpenHabItem[h].Name))
                {
                    delay(200);
                    if(!UpdateOpenhabItemState(OpenHabItem[h].Name)) // Retry Once
                        allOkay = false;            
                }
            }
    }
  }  
  WifiDisconnect();
  if(allOkay){
    sendEntry = millis();
    Serial.println(F("Done updating items state in Openhab"));
  }else{
    Serial.println(F("Errors while updating items state in Openhab"));
    ConnectionFailureCount++;
  }
}
//---------------------------------------------------------------------------------------------------