# Home Assistant (HA) Dashboard for LilyGo T5 4.7 Inch E-Paper Screen

![](IMG_2747.jpg)

LilyGo T5 4.7 Inch E-Paper Screen is very cheap all in one board that you can use to display various content. 
This would make a cool dashboard when integrated with HA to monitor the status of the devices intergated to HA. 

LilyGo T5 4.7 Inch E-Paper come with a SDK with limited features but this project is based on https://github.com/vroland/epdiy project where you have lot of flexibilities in programming the screen including rotation. 

You need platform.io to build the project and read platform.io documentation if you don't know how to build Arduino project in platform.io. 

The configurations are pretty straight forward and head on to configurations.h file and follow inline comments. 

Icons are taken with love from https://www.flaticon.com/ and resized to match the tile sizes. 

You can contribute to this project and improve it if you feel like helping others who will use this to view the home automation status. Here are the few things that you can help with.
- Implement DeepSleep mode
- Configuration of Home Assistant entities via Web Interface so we don't have to complie and upload the bin file each time we are updating or adding new entity
- Touch support (I dont have touch screen now. But I'm planning to order it. Maybe touch will toggle switches on/off)
- More uniformed Icon set - Free Icons are taken from flaticon.com and may not have uniformity. I'm not a graphic/icon designer but if someone can create 100x100px icons for OFF and ON positions, community will thank you for your effort. 