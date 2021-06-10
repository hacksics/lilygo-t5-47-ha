#include <Arduino.h>


// esp32 sdk imports
#include "esp_heap_caps.h"
#include "esp_log.h"

// epd
#include "epd_driver.h"
#include "epd_highlevel.h"

// battery
#include <driver/adc.h>
#include "esp_adc_cal.h"

// deepsleep
#include "esp_sleep.h"

// font
#include "opensans8b.h"
#include "opensans9b.h"
#include "opensans24b.h"

// wifi
#include <WiFi.h>


#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "configurations.h"
#include "homeassistantapi.h"

// Icons for Home Assistant 
#include "waterheateron.h"
#include "waterheateroff.h"
#include "lightbulbon.h"
#include "lightbulboff.h"
#include "exhaustfanon.h"
#include "exhaustfanoff.h"
#include "fanoff.h"
#include "fanon.h"
#include "airpurifieron.h"
#include "airpurifieroff.h"
#include "plugon.h"
#include "plugoff.h"
#include "switchon.h"
#include "switchoff.h"
#include "airconditioneron.h"
#include "airconditioneroff.h"
#include "warning.h"

// sensor icons
#include "dooropen.h"
#include "doorclosed.h"
#include "motionsensoron.h"
#include "motionsensoroff.h"
#include "sensorerror.h"

#define White         0xFF
#define LightGrey     0xBB
#define Grey          0x88
#define DarkGrey      0x44
#define Black         0x00


#define BATT_PIN            36

#define TILE_IMG_WIDTH  100
#define TILE_IMG_HEIGHT 100
#define TILE_WIDTH      160
#define TILE_HEIGHT     160
#define TILE_GAP        6
#define SENSOR_TILE_WIDTH      120
#define SENSOR_TILE_HEIGHT     110
#define SENSOR_TILE_IMG_WIDTH  64
#define SENSOR_TILE_IMG_HEIGHT 64
#define BOTTOM_TILE_WIDTH      320
#define BOTTOM_TILE_HEIGHT     90

#define WAVEFORM EPD_BUILTIN_WAVEFORM

enum alignment {LEFT, RIGHT, CENTER};


// ambient temperature around device set accordinly 
int temperature = 30;
int wifi_signal = 0;
uint8_t *fb;
enum EpdDrawError err;

// required for NTP time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;

EpdiyHighlevelState hl;
EpdRotation orientation = EPD_ROT_LANDSCAPE;
EpdFont  currentFont;

uint8_t StartWiFi() {
  Serial.print("\r\nConnecting to: "); Serial.println(String(ssid));
  IPAddress dns(8, 8, 8, 8); // Google DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  uint8_t connectionStatus;
  bool AttemptConnection = true;
  while (AttemptConnection) {
    connectionStatus = WiFi.status();
    if (millis() > start + 15000) { // Wait 15-secs maximum
      AttemptConnection = false;
    }
    if (connectionStatus == WL_CONNECTED || connectionStatus == WL_CONNECT_FAILED) {
      AttemptConnection = false;
    }
    delay(50);
  }
  if (connectionStatus == WL_CONNECTED) {
    wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
    timeClient.begin();
    timeClient.setTimeOffset(gmtOffset_sec);
    Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  }
  else Serial.println("WiFi connection *** FAILED ***");
  return connectionStatus;
}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}


void drawString(int x, int y, String text, alignment align) {
  char * data  = const_cast<char*>(text.c_str());
  EpdFontProperties font_props = epd_font_properties_default();
  if (align == CENTER) font_props.flags = EPD_DRAW_ALIGN_CENTER;
  if (align == LEFT) font_props.flags = EPD_DRAW_ALIGN_LEFT;
  if (align == RIGHT) font_props.flags = EPD_DRAW_ALIGN_RIGHT;
  epd_write_string(&currentFont, data, &x, &y, fb, &font_props);
}

void fillCircle(int x, int y, int r, uint8_t color) {
  epd_fill_circle(x, y, r, color, fb);
}

void drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color) {
  epd_draw_hline(x0, y0, length, color, fb);
}

void drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color) {
  epd_draw_vline(x0, y0, length, color, fb);
}

void drawCircle(int x0, int y0, int r, uint8_t color) {
  epd_draw_circle(x0, y0, r, color, fb);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  EpdRect area = {
        .x = x,
        .y = y,
        .width = w,
        .height = h,
    };
  epd_draw_rect(area, color, fb);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  EpdRect area = {
        .x = x,
        .y = y,
        .width = w,
        .height = h,
    };
  epd_fill_rect(area, color, fb);
}

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                  int16_t x2, int16_t y2, uint16_t color) {
  epd_fill_triangle(x0, y0, x1, y1, x2, y2, color, fb);
}

void drawPixel(int x, int y, uint8_t color) {
  epd_draw_pixel(x, y, color, fb);
}

void setFont(EpdFont const &font) {
  currentFont = font;
}

void epd_update() {
    epd_poweron();
    epd_hl_update_screen(&hl, MODE_GC16, temperature);
    delay(500);
    epd_poweroff();
}

// this will place a tile on screen that includes icon, staus and name of the HA entity
void placeTile(int x, int y, int width, int height, const uint8_t* image_data, String label, String state)
{
  // this assumes images are 100x100px size. make sure images are cropped to 100x100 before converting
  int image_x = int((width - TILE_IMG_WIDTH)/2) + x; 
  int image_y = int((height - TILE_IMG_HEIGHT)/2) + y;
  EpdRect image_area = {
        .x = image_x,
        .y = image_y,
        .width = TILE_IMG_WIDTH, 
        .height = TILE_IMG_HEIGHT, 
    };
  int txt_cursor_x = width/2 + x;
  int txt_cursor_y = image_y + TILE_IMG_HEIGHT + 10 + 12;
  int state_txt_cursor_x = int(width/2) + x;
  int state_txt_cursor_y = y + 21;
  drawRect(x, y, width, height, Black);
  drawRect(x+1, y+1, width-2, height-2, Black);
  epd_copy_to_framebuffer(image_area, (uint8_t *) image_data, fb);
  drawString(txt_cursor_x, txt_cursor_y, label, CENTER);
  drawString(state_txt_cursor_x, state_txt_cursor_y, state, CENTER);
}

void placeSensorTile(int x, int y, int width, int height, const uint8_t* image_data, String label)
{
  // this assumes images are 128x128px size. make sure images are cropped to 128x128 before converting
  int image_x = int((width - SENSOR_TILE_IMG_WIDTH)/2) + x; 
  int image_y = y + 10;
  EpdRect image_area = {
        .x = image_x,
        .y = image_y,
        .width = SENSOR_TILE_IMG_WIDTH, 
        .height = SENSOR_TILE_IMG_HEIGHT, 
    };
  int txt_cursor_x = int(width/2) + x;
  int txt_cursor_y = image_y + SENSOR_TILE_IMG_HEIGHT + 10 + 12;
  drawRect(x, y, width, height, Black);
  drawRect(x+1, y+1, width-2, height-2, Black);
  epd_copy_to_framebuffer(image_area, (uint8_t *) image_data, fb);
  drawString(txt_cursor_x, txt_cursor_y, label, CENTER);
}

void drawTile(int x, int y, int state, int type, String name)
{
    int tile_width = TILE_WIDTH - TILE_GAP; 
    int tile_height = TILE_HEIGHT - TILE_GAP;
    String state_txt = "OFF";
    if (state == entity_state::ON) state_txt = "ON"; 
    else if (state == entity_state::UNAVAILABLE) state_txt = "UNAVAILABLE"; 
    switch (type)
    {
    case entity_type::SWITCH:
        if (state == entity_state::ON) placeTile(x,y,tile_width,tile_height,switchon_data, name, state_txt); 
        else if (state == entity_state::OFF) placeTile(x,y,tile_width,tile_height,switchoff_data, name, state_txt); 
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "SWITCH"); 
        break;
    case entity_type::LIGHT:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,lightbulbon_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,lightbulboff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "LIGHT"); 
        break;
    case entity_type::FAN:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,fanon_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,fanoff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "FAN"); 
        break;
    case entity_type::EXFAN:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,exhaustfanon_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,exhaustfanoff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "EXHAUST FAN"); 
        break;
    case entity_type::AIRPURIFIER:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,airpurifieron_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,airpurifieroff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "AIR PURIFIER"); 
        break;
    case entity_type::WATERHEATER:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,waterheateron_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,waterheateroff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "WATER HEATER"); 
        break;
    case entity_type::PLUG:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,plugon_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,plugoff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "PLUG"); 
        break;
    case entity_type::AIRCONDITIONER:
        if (state == entity_state::ON)  placeTile(x,y,tile_width,tile_height,airconditioneron_data,name, state_txt);
        else if (state == entity_state::OFF)  placeTile(x,y,tile_width,tile_height,airconditioneroff_data,name, state_txt);
        else placeTile(x,y,tile_width,tile_height,warning_data, name, "AIR CONDITIONER"); 
        break;
    default:
        break;
    }
}

void drawSensorTile(int x, int y, int state, int type, String name)
{
    int tile_width = SENSOR_TILE_WIDTH - TILE_GAP; 
    int tile_height = SENSOR_TILE_HEIGHT - TILE_GAP;
    switch (type)
    {
    case sensor_type::DOOR:
        if (state == entity_state::ON) placeSensorTile(x,y,tile_width,tile_height,dooropen_data, name); 
        else if (state == entity_state::OFF) placeSensorTile(x,y,tile_width,tile_height,doorclosed_data, name); 
        else placeSensorTile(x,y,tile_width,tile_height,sensorerror_data, name); 
        break;
    case sensor_type::MOTION:
        if (state == entity_state::ON) placeSensorTile(x,y,tile_width,tile_height,motionsensoron_data, name); 
        else if (state == entity_state::OFF) placeSensorTile(x,y,tile_width,tile_height,motionsensoroff_data, name); 
        else placeSensorTile(x,y,tile_width,tile_height,sensorerror_data, name); 
        break;
    default:
        break;
    }
}

void drawBottomTile(int x, int y, String value, String name)
{
    int tile_width = BOTTOM_TILE_WIDTH - TILE_GAP; 
    int tile_height = BOTTOM_TILE_HEIGHT - TILE_GAP;
    drawRect(x, y, tile_width, tile_height, Black);
    drawRect(x+1, y+1, tile_width-2, tile_height-2, Black);
    setFont(OpenSans24B);
    drawString(int(tile_width/2) + x, 508, value, CENTER);
    setFont(OpenSans9B);
    drawString(int(tile_width/2) + x, 532, name, CENTER);
}

void showBottomBar()
{
    int tiles = 3;
    float totalEnergy = 0;
    float totalPower  = 0;
    String totalEnergyName;
    String totaPowerName;
    for (int i = 0; i < (sizeof(haFloatSensors) / sizeof(haFloatSensors[0])); i++){
        if (haFloatSensors[i].entityType == sensor_type::ENERGYMETERIFXDB)
        {
            totalEnergy = totalEnergy + getLast30DayEnergyUsage(haFloatSensors[i].entityID);
            totalEnergyName = haFloatSensors[i].entityName;
        }
        else if (haFloatSensors[i].entityType == sensor_type::ENERGYMETER)
        {
            totalEnergy = totalEnergy + getSensorFloatValue(haFloatSensors[i].entityID);
            totalEnergyName = haFloatSensors[i].entityName;
        }
        else if (haFloatSensors[i].entityType == sensor_type::ENERGYMETERPWR)
        {
            totalPower = totalPower + getSensorFloatValue(haFloatSensors[i].entityID);
            totaPowerName = haFloatSensors[i].entityName;
        }
    }
    int x = 3;
    int y = 456;
    // first one 
    if (totalEnergy != 0)
    {
        drawBottomTile(x, y, String((int)totalEnergy) + " kWh", totalEnergyName);
        x = x + BOTTOM_TILE_WIDTH;
        tiles--;
    }
    if (totalPower != 0)
    {
        drawBottomTile(x, y, String((int)totalPower) + " W", totaPowerName);
        x = x + BOTTOM_TILE_WIDTH;
        tiles--;
    }
    
    for (int i = 0; i < (sizeof(haFloatSensors) / sizeof(haFloatSensors[0])); i++){
        if (haFloatSensors[i].entityType == sensor_type::TEMP && tiles >= 1)
        {
            float roomTemp = getSensorFloatValue(haFloatSensors[i].entityID);
            drawBottomTile(x, y, String(roomTemp) + " C", haFloatSensors[i].entityName);
            x = x + BOTTOM_TILE_WIDTH;
            tiles--;
        }
    }
}

void showWifiErrorScreen()
{
    int cursor_x = EPD_WIDTH / 2;
    int cursor_y = EPD_HEIGHT / 2;
    drawString(cursor_x, cursor_y, "UNABLE TO JOIN WIFI!", CENTER);
}

void showSwitchBar()
{
    setFont(OpenSans9B);
    int x = 3;
    int y = 23;
    for (int i = 0; i < 12; i++){
        if (haEntities[i].entityType == entity_type::SWITCH ||
            haEntities[i].entityType == entity_type::LIGHT ||
            haEntities[i].entityType == entity_type::EXFAN ||
            haEntities[i].entityType == entity_type::FAN ||
            haEntities[i].entityType == entity_type::AIRPURIFIER ||
            haEntities[i].entityType == entity_type::WATERHEATER ||
            haEntities[i].entityType == entity_type::AIRCONDITIONER)
        {
            drawTile(x,y,checkOnOffState(haEntities[i].entityID),haEntities[i].entityType, haEntities[i].entityName);
        }
        x = x + TILE_WIDTH;
        if (i == 5){
            x = 3;
            y = y + TILE_HEIGHT;
        }
    }
}

void showSensorBar()
{
    setFont(OpenSans9B);
    int x = 3;
    int y = 345;
    for (int i = 0; i < 8; i++){
        if (haSensors[i].entityType == sensor_type::DOOR ||
            haSensors[i].entityType == sensor_type::MOTION )
        {
            drawSensorTile(x,y,checkOnOffState(haSensors[i].entityID),haSensors[i].entityType, haSensors[i].entityName);
        }
        x = x + SENSOR_TILE_WIDTH;
    }
}

void drawRSSI(int x, int y, int rssi) {
  int WIFIsignal = 0;
  int xpos = 1;
  for (int _rssi = -100; _rssi <= rssi; _rssi = _rssi + 20) {
    if (_rssi <= -20)  WIFIsignal = 30; //            <-20dbm displays 5-bars
    if (_rssi <= -40)  WIFIsignal = 24; //  -40dbm to  -21dbm displays 4-bars
    if (_rssi <= -60)  WIFIsignal = 18; //  -60dbm to  -41dbm displays 3-bars
    if (_rssi <= -80)  WIFIsignal = 12; //  -80dbm to  -61dbm displays 2-bars
    if (_rssi <= -100) WIFIsignal = 6;  // -100dbm to  -81dbm displays 1-bar
    fillRect(x + xpos * 8, y - WIFIsignal, 7, WIFIsignal, Black);
    xpos++;
  }
  //fillRect(x, y - 1, 5, 1, GxEPD_BLACK);
  drawString(x + 40,  y, String(rssi) + "dBm", LEFT);
}

void drawBattery(int x, int y) {
  uint8_t percentage = 100;
  float voltage = analogRead(35) / 4096.0 * 7.46;
  if (voltage > 1 ) { // Only display if there is a valid reading
    Serial.println("Voltage = " + String(voltage));
    percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
    if (voltage >= 4.20) percentage = 100;
    if (voltage <= 3.20) percentage = 0;  // orig 3.5
    drawRect(x + 55, y - 15 , 40, 15, Black);
    fillRect(x + 95, y - 9, 4, 6, Black);
    fillRect(x + 57, y - 13, 36 * percentage / 100.0, 11, Black);
    drawString(x, y, String(percentage) + "%", LEFT);
    //drawString(x + 13, y + 5,  String(voltage, 2) + "v", CENTER);
  }
}

void getNTPDateTime()
{
    while(!timeClient.update()) {
        timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
}

void displayGeneralInfoSection()
{
    // Uncomment the next line if the display of IP- and MAC-Adddress is wanted
    //drawString(SCREEN_WIDTH - 150, 20, "IP=" + LocalIP + ",  MAC=" + WiFi.macAddress() ,RIGHT);
    // drawFastHLine(5, 30, SCREEN_WIDTH - 8, Black);
    HAConfigurations haConfigs = getHaStatus();
    getNTPDateTime();
    drawString(EPD_WIDTH/2, 14, "Refreshed: " + dayStamp + " at " +  timeStamp + " (HA Ver:" + haConfigs.version + "/" + haConfigs.haStatus + ", TZ:" + haConfigs.timeZone + ")", CENTER);
}

void displayStatusSection() {
  setFont(OpenSans8B);
  drawRSSI(850, 14, wifi_signal);
  drawBattery(5, 14);
  displayGeneralInfoSection();
}



void drawHAScreen()
{
    epd_fullclear(&hl, temperature);

    epd_hl_set_all_white(&hl);
    setFont(OpenSans9B);
    drawString(EPD_WIDTH/2, EPD_HEIGHT/2, "LOADING...", CENTER);
    
    epd_update();
    
    uint8_t connectionState = StartWiFi();
    if (connectionState == WL_CONNECTED){
        epd_hl_set_all_white(&hl);
        displayStatusSection();
        showSwitchBar();
        showSensorBar();
        showBottomBar();
    }
    else{
        epd_hl_set_all_white(&hl);
        showWifiErrorScreen();
    }
    
    epd_update();

    StopWiFi();
    delay(1000);
}

void setup()
{
    Serial.begin(115200);
    currentFont = OpenSans9B;
    epd_init(EPD_OPTIONS_DEFAULT);
    hl = epd_hl_init(WAVEFORM);
    epd_set_rotation(orientation);
    fb = epd_hl_get_framebuffer(&hl);
    
    drawHAScreen();

}

void loop()
{
    delay(60000);
    drawHAScreen();
}