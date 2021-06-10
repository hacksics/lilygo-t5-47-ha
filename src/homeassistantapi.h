HTTPClient http;
WiFiClientSecure client;

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    //char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        //code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
    
}

float getLast30DayEnergyUsage(String entity_id)
{
    // thisi is custom function I used to get the last 30day energy from influxdb
    // you may change/reuse/ignore this
    // entity_id = energy_meter_floor_01_energy_today
    HTTPClient http2;
    float energy = 0;
    String query = "select sum(\"max\") from (SELECT max(\"value\") FROM \"autogen\".\"kWh\" WHERE (\"entity_id\" = '" + entity_id + "') AND time > now() - 30d GROUP BY time(1d)  fill(null) )";
    String api_url = "http://192.168.2.138:8086/query?db=homeassistant&u=homeassistant&p=homeassistant&q=" + urlencode(query);
    http2.useHTTP10(); // sometimes you need to use HTTP1.0 - https://arduinojson.org/v6/api/misc/deserializationerror/ 
    http2.begin(api_url);
    int code = http2.GET();
    Serial.println(api_url);
    if (code != HTTP_CODE_OK)
    {
        Serial.println("Error connecting to Influx DB");
        Serial.println(code);
        Serial.println(http2.getString());
        return entity_state::ERROR;
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http2.getStream());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return entity_state::ERROR;
    }
    String energy_s = doc["results"][0]["series"][0]["values"][0][1];
    Serial.println(energy_s);
    energy = energy_s.toFloat();
    http2.end();
    return energy;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


int toggleEntityState(String entity)
{
    String entityDomain = getValue(entity, '.', 0);
    String api_url = ha_server + "/api/services/" + entityDomain + "/toggle";
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST("{\"entity_id\": \""+ entity + "\"}");
    if (code != HTTP_CODE_OK)
    {
        Serial.println("Error connecting to HA API");
        Serial.println(code);
        Serial.println(http.getString());
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
        Serial.println(http.getString());
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
        Serial.println(http.getString());
        return haConfigs;
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());
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
        Serial.println(http.getString());
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