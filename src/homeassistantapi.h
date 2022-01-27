HTTPClient http;
WiFiClientSecure client;

int checkOnOffState(String entity)
{
    String api_url = ha_server + "/api/states/" + entity;
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        Serial.println("Error connecting to HA API");
        Serial.println(code);
        return entity_state::ERROR;
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return entity_state::ERROR;
    }
    String state = doc["state"];
    Serial.println(state);
    http.end();
    if (state == "on"){
        return entity_state::ON;
    }
    if (state == "unavailable"){
        return entity_state::UNAVAILABLE;
    }
    return entity_state::OFF;
}

HAConfigurations getHaStatus()
{
    HAConfigurations haConfigs;
    haConfigs.haStatus = "ERROR";
    haConfigs.timeZone = "ERROR";
    haConfigs.version  = "ERROR"; 

    String api_url = ha_server + "/api/config";
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        Serial.println("Error connecting to HA API");
        Serial.println(code);
        return haConfigs;
    }
    DynamicJsonDocument doc(4096);
    StaticJsonDocument<64> filter;
    // Filter JSON data to save RAM
    filter["time_zone"] = true;
    filter["version"] = true;
    filter["state"] = true;
    DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return haConfigs;
    }
    String state = doc["state"];
    String timeZone = doc["time_zone"];
    String version = doc["version"];
    haConfigs.haStatus = state;
    haConfigs.timeZone = timeZone;
    haConfigs.version  = version; 
    //String state = doc["state"];
    //Serial.println(state);
    http.end();
    return haConfigs;
}

float getSensorFloatValue(String entity)
{
    String api_url = ha_server + "/api/states/" + entity;
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        Serial.println("Error connecting to HA API");
        Serial.println(code);
        return 0;
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return 0;
    }
    String state = doc["state"];
    Serial.println(state);
    http.end();
    return state.toFloat();
}
