#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define WiFi_SSID      "S"
#define WiFi_PASSWORD  "marco12345"

#define API_KEY        "AIzaSyCw6WujCRHbDPo2oqM5d7BmnHj1xAE0rew"
#define DATABASE_URL   "https://m11-iot-b28b6-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL     "marcoowentanaka@gmail.com"
#define USER_PASSWORD  "marcoowen"

#define DHT_PIN     23
#define LDR_PIN     19
#define SOIL_PIN    18
#define PIN_P       25
#define PIN_AIR     26
#define PIN_FAN     27
#define PIN_MIST    14
#define PIN_LIGHT   33

FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

unsigned long lastSensorUpdate = 0;
unsigned long sensorInterval = 5000;

bool motionDetected = false;
bool flameDetected = false;
bool objectDetected = false;

unsigned long getTimestamp() {
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return millis();
    time(&now);
    return ((unsigned long)now * 1000);
}

void bacaDanKirimData() {
    int rawLdr = analogRead(LDR_PIN);
    int lightLevel = map(rawLdr, 4095, 0, 0, 100);
    lightLevel = constrain(lightLevel, 0, 100);

    int rawSoil = analogRead(SOIL_PIN);
    int soilPercent = map(rawSoil, 4095, 0, 0, 100);
    soilPercent = constrain(soilPercent, 0, 100);

    motionDetected = digitalRead(PIN_P) == HIGH;
    flameDetected = digitalRead(PIN_AIR) == HIGH;
    objectDetected = digitalRead(PIN_LIGHT) == HIGH;

    if (Firebase.ready()) {
        String basePath = "/greenhouse/sensors";
        bool allSuccess = true;

        if (!Firebase.RTDB.setInt(&fbdo, basePath + "/lightLevel", lightLevel)) allSuccess = false;
        if (!Firebase.RTDB.setInt(&fbdo, basePath + "/soilMoisture", soilPercent)) allSuccess = false;
        if (!Firebase.RTDB.setBool(&fbdo, basePath + "/motion", motionDetected)) allSuccess = false;
        if (!Firebase.RTDB.setBool(&fbdo, basePath + "/flame", flameDetected)) allSuccess = false;
        if (!Firebase.RTDB.setBool(&fbdo, basePath + "/object", objectDetected)) allSuccess = false;

        unsigned long timestamp = getTimestamp();
        if (!Firebase.RTDB.setDouble(&fbdo, basePath + "/timestamp", timestamp)) allSuccess = false;
    }
}

void connectWiFi() {
    WiFi.begin(WiFi_SSID, WiFi_PASSWORD);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (millis() - start > 20000) ESP.restart();
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LDR_PIN, INPUT);
    pinMode(SOIL_PIN, INPUT);
    pinMode(PIN_P, INPUT);
    pinMode(PIN_AIR, INPUT);
    pinMode(PIN_FAN, INPUT);
    pinMode(PIN_MIST, INPUT);
    pinMode(PIN_LIGHT, INPUT);

    connectWiFi();

    configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    unsigned long fbstart = millis();
    while (!Firebase.ready() && millis() - fbstart < 8000) delay(400);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) connectWiFi();

    unsigned long now = millis();
    if (now - lastSensorUpdate > sensorInterval) {
        lastSensorUpdate = now;
        bacaDanKirimData();
    }
}
