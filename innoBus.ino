#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Inno";
const char* password = "Hackathon";
const char* serverName = "http://103f-178-160-200-178.ngrok-free.app/api/locations";
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;



TinyGPSPlus gps;
HardwareSerial sim808(1); // Use Serial1 for the SIM808

void jsonDataPost(float lat, float lon) {
    const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["car_id"] = "77MK001";
    jsonDoc["latitude"] = lat;
    jsonDoc["longitude"] = lon;
    String jsonData;
    serializeJson(jsonDoc, jsonData);
    Serial.println(jsonData);
    HTTPClient httPost;
    httPost.begin(serverName);
    httPost.addHeader("Content-Type", "application/json");
    int httpResponceCode = httPost.POST(jsonData);
    delay(1000);
    if (httpResponceCode > 0) {
        String response = httPost.getString();
        Serial.println(httpResponceCode);
        Serial.println(response);
    } else {
        Serial.print("err:");
        Serial.println(httpResponceCode);
    }
    httPost.end();
}

void setup() {
    Serial.begin(115200); 
    sim808.begin(9600, SERIAL_8N1, 16, 17); 
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Initializing SIM808...");

    // Send initialization commands to the SIM808
    sim808.println("AT");
    delay(500);
    while (sim808.available()) {
        Serial.write(sim808.read());
    }

    sim808.println("AT+CGPSPWR=1");  // Power on GPS
    delay(500);
    while (sim808.available()) {
        Serial.write(sim808.read());
    }
}

void loop() {
    sim808.println("AT+CGPSSTATUS?");  // Check GPS status
    delay(1000);
    while (sim808.available()) {
        char c = sim808.read();
        Serial.write(c);
    }

    sim808.println("AT+CGPSINF=0");  // Get GPS info
    delay(500);
    String gpsData = "";
    while (sim808.available()) {
        char c = sim808.read();
        Serial.write(c);
        gpsData += c;
    }
    // Parse the +CGPSINF response
    int startIndex = gpsData.indexOf(": 0,") + 4;
    int endIndex = gpsData.indexOf(",", startIndex);
    String latitudeStr = gpsData.substring(startIndex, endIndex);

    startIndex = endIndex + 1;
    endIndex = gpsData.indexOf(",", startIndex);
    String longitudeStr = gpsData.substring(startIndex, endIndex);

    // Convert latitude and longitude to float
    float latitude = latitudeStr.toFloat();
    float longitude = longitudeStr.toFloat();

    // Convert NMEA format to standard decimal format
    int latDegrees = int(latitude / 100);
    float latMinutes = latitude - (latDegrees * 100);
    latitude = latDegrees + (latMinutes / 60.0);

    int lonDegrees = int(longitude / 100);
    float lonMinutes = longitude - (lonDegrees * 100);
    longitude = lonDegrees + (lonMinutes / 60.0);

    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);
    delay(1000); 

    jsonDataPost(latitude, longitude);
}


