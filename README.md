# Wi-Fi Sniffer as a Human detector - now useable with OpenHAB
This project is my take on a Wifi-MAC address presence monitor for openhab.

It was originally forked from https://github.com/andreaslink-de/WiFi-Sniffer-as-a-Human-detector-usable-with-OpenHAB wich was forked from  Andreas Spiess' "Wi-Fi Sniffer as a Human detector" who has based it on a https://www.hackster.io/rayburne/esp8266-mini-sniff-f6b93a from Ray Burnette. 

Main goal of my fork was to get rid of the huge (mqtt) arrays that were used in Andreas Link's version, because I could not get it to work stable on a ESP-01. Additionally, I wanted to make the ESP more 'smart', so it would know what to monitor.

## How it works
The Wifi Sniffer will connect to the REST interface of an openhab server. There it will get items that a tagged for monitoring, and will store the MAC adresses of these items in a local array. Once this is done, it will start to monitor these MAC adressees on Wifi. If they are detected, the state of the item will be set to ON.

## Prerequisities
- Openhab server with enabled REST binding
- ESP (ESP01 will work)

## Setup Openhab
For each item that you want to "sniff" create two items. One item will server as a "proxy" item, so  you can use multiple ESP- Sniffers to update presence on the "real" presence item. A rule will make the presence time-out after a while. 

For example, one phone is from person "Seinpiller" with mac address Mac:66:B4:A5:C4:15:21.

Create proxy item:
'''
Switch PresenceMonitor_Seinpiller "Seinpiller" <network> ["Sniffer", "Mac:66:B4:A5:C4:15:21"]
'''
Note in the proxy items, put the tags "Sniffer" and the MAC address.

Create real item (for using in sitemaps)
'''
Switch Presence_Seinpiller "Seinpiller [MAP(home.map):%s]" <network>
'''

finally, create a rule that acts on the proxy item, and updates real item:
'''
val logName = ".presence"
var Timer presenceTimerSeinpiller = null 
rule "PresenceMonitor_Seinpiller"
    when   
            Item PresenceMonitor_Seinpiller changed from OFF to ON
    then        

    Presence_Seinpiller.sendCommand(ON)    
    logInfo(logName, "Seinpiller is Present");
    if (presenceTimerSeinpiller!=null){
                    presenceTimerSeinpiller.cancel()
                    presenceTimerSeinpiller = null;
    }   

    presenceTimerSeinpiller = createTimer(now.plusMinutes(5), [|
                    logInfo(logName, "Seinpiller went away");
					Presence_Seinpiller.sendCommand(OFF)
                    presenceTimerSeinpiller = null
	])   

    PresenceMonitor_Seinpiller.sendCommand(OFF)
end
'''

## Setup ESP
Download the code and upload to ESP. 

First time the esp will start as accesspoint. Connect to it using phone or laptop, and go to the settings webpage.
The address will be http://192.168.4.1

If you want to change the config, pull down GPIO-0. The sniffer will break out of the scan loop, and start serving the config webpage.