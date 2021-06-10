// Start of reserved configurations. Do not change if you dont know what you are doing
enum entity_state {ON, OFF, ERROR, UNAVAILABLE};
enum entity_type {SWITCH, LIGHT, METER, SENSOR, EXFAN, FAN, AIRPURIFIER, WATERHEATER, PLUG, AIRCONDITIONER};
enum entity_state_type {ONOFF, VALUE};
enum sensor_type {DOOR, MOTION, ENERGYMETER, TEMP, ENERGYMETERPWR, ENERGYMETERIFXDB};
struct HAEntities{
    String entityName;
    String entityID;
    int entityType;
    int entityStateType;
};

struct HAConfigurations{
    String timeZone;
    String version;
    String haStatus;
};
// End of reserved configurations


// Start of user configurations. Change as instructed in the comments

// Change to your WiFi credentials
const char* ssid        = "WIFI SSID";
const char* password    = "WIFI PASSWORD";


// url to HA server. Only http is supported for now!
const String ha_server  = "http://192.168.2.138:8123";
// create a long lived access token and put it here. ref: https://www.home-assistant.io/docs/authentication/
const String ha_token   = "..";

// GMT Offset in seconds. UK normal time is GMT, so GMT Offset is 0, for US (-5Hrs) is typically -18000, AU is typically (+8hrs) 28800
int   gmtOffset_sec     = 19800;

/**
 *  Entities are shown in top two rows. Supported types are in entity_type and different icons are used for easy recognition
 *  Only 12 different entities are supported 6 cols x 2 rows. 
 *  Entities follow the format of { <Name that should be displayed>, <entity_id in HA>, <entity_type>, <entity_state_type>}
 *  User a short entity name so it can fit nicely in 160px width in 9px font. 
**/
HAEntities haEntities [] {
    {"POND FILTER", "switch.pond_filter", SWITCH, ONOFF},
    {"ROOF", "switch.tasmota_2", LIGHT, ONOFF},
    {"FR. DOOR", "switch.tasmota_3", LIGHT, ONOFF},
    {"BAR", "switch.exhaust_fan", EXFAN, ONOFF},
    {"STAIRS2", "switch.stairs2", LIGHT, ONOFF},
    {"STAIRS1", "switch.stairs_1_zigbee_switch_on_off", LIGHT, ONOFF},
    {"M. BEDROOM", "fan.xiaomi_air_purifier_2s", AIRPURIFIER, ONOFF},
    {"HEATER", "switch.water_heater_2", WATERHEATER, ONOFF},
    {"FR. DOOR", "switch.tasmota_3", LIGHT, ONOFF},
    {"BEDROOM UVC", "switch.uvc_bedroom_ac", SWITCH, ONOFF},
    {"GARAGE", "switch.uvc_bedroom_ac", FAN, ONOFF},
    {"M.BEDROOM", "switch.stairs_1_zigbee_switch_on_off", AIRCONDITIONER, ONOFF},
};

/**
 *  Sensors are shown in 3rd row. Supported types are DOOR and MOTION currently. Different icons are used for easy recognition
 *  Only 8 different entities are supported - 8 cols. 
 *  Entities follow the format of { <Name that should be displayed>, <entity_id in HA>, <DOOR|MOTION>, <ONOFF>}
 *  User a short entity name so it can fit nicely in 120px width in 9px font. 
**/
HAEntities haSensors[] {
    {"M. BED", "binary_sensor.master_bedroom_door_sensor_ias_zone", DOOR, ONOFF},
    {"STAIRS 2", "binary_sensor.stairs_2_motion_sensor_ias_zone", MOTION, ONOFF},
    {"STAIRS 1", "binary_sensor.stairs_1_motion_sensor_ias_zone", MOTION, ONOFF},
    {"BAR", "binary_sensor.bar_area_motion_sensor_ias_zone", MOTION, ONOFF},
    {"KITCHEN", "binary_sensor.kitchen_motion_sensor_ias_zone", MOTION, ONOFF},
    {"MAIN", "binary_sensor.main_door_sensor_ias_zone", DOOR, ONOFF},
    {"KITCHEN", "binary_sensor.kitchen_door_sensor_ias_zone", DOOR, ONOFF},
    {"KITCHEN", "binary_sensor.kitchen_door_sensor_ias_zone", DOOR, ONOFF}
};

/**
 *  Sensors are shown in last row. Supported types are ENERGYMETER, ENERGYMETERPWR and  and TEMP currently. Different icons are used for easy recognition
 *  Only 3 different entities are supported - 3 cols. ENERGYMETER and ENERGYMETERPWR are grouped together and shown as total
 *  Entities follow the format of { <Name that should be displayed>, <entity_id in HA>, <ENERGYMETER|ENERGYMETERPWR|TEMP>, <VALUE>}
 *  User a short entity name so it can fit nicely in 120px width in 9px font. 
 *  You can have only one ENERGYMETER type and ENERGYMETERPWR type tile in the display. ENERGYMETER and ENERGYMETERPWR are grouped together and shown as total
 *  However you can have multiple temrature tiles (up to 3 if you are not using ENERGYMETER and ENERGYMETERPWR in you HA instances) 
 *  Or you can customize the code the way you see fit (advanced)
**/
HAEntities haFloatSensors[] {
    //{"TOTAL ENERGY TODAY", "sensor.energy_meter_floor_03_energy_today", ENERGYMETER, VALUE},
    //{"TOTAL ENERGY TODAY", "sensor.tasmota_energy_today", ENERGYMETER, VALUE},
    //{"TOTAL ENERGY TODAY", "sensor.energy_meter_floor_01_energy_today", ENERGYMETER, VALUE},
    {"TOTAL ENERGY LAST 30", "energy_meter_floor_03_energy_today", ENERGYMETERIFXDB, VALUE},
    {"TOTAL ENERGY LAST 30", "tasmota_energy_today", ENERGYMETERIFXDB, VALUE},
    {"TOTAL ENERGY LAST 30", "energy_meter_floor_01_energy_today", ENERGYMETERIFXDB, VALUE},
    {"CURRENT POWER", "sensor.energy_meter_floor_03_energy_power", ENERGYMETERPWR, VALUE},
    {"CURRENT POWER", "sensor.energy_meter_floor_02_energy_power", ENERGYMETERPWR, VALUE},
    {"CURRENT POWER", "sensor.energy_meter_floor_01_energy_power", ENERGYMETERPWR, VALUE},
    {"ROOM TEMP", "sensor.xiaomi_airpurifier_temp", TEMP, VALUE},
};
