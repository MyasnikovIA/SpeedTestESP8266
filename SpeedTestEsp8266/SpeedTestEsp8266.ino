#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "XXXXXXXXXXXXXXXX";   
const char* password = "XXXXXXXXX";

unsigned long start_time = 0;
unsigned long end_time = 0;
unsigned long chunk_elapsed = 0; // Holds the time to download the current chunk.
int response_size = 0;
float chunk_speed = 0.0; 
float total_speed = 0.0; // Holds the speed of the total download.
int total_bytes = 0; // Holds the total bytes transferred.
unsigned long total_elapsed = 0; // Holds the elapsed time counter for the loop.


const int relayPin = 2; // Реле подключено к D4 (GPIO2)
bool relayState = false; // Текущее состояние реле


void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  
  WiFi.begin(ssid, password);

  Serial.print("\n WiFi Connect: \n");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("... IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
  Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
}
int indQuery=0;
int indfailedConnection=0;


void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    HTTPClient https;
    total_bytes = 0;
    total_elapsed = 0; 
    client.setInsecure(); // Skip verification
    Serial.print("--------------------\n");
    if (https.begin(client,  "https://jwsmythe.com/xfer/esp8266-32-group/1MB_file.bin")) {  // HTTPS
    // if (https.begin(client,  "https://raw.githack.com/MyasnikovIA/SpeedTestESP8266/main/1MB_file.bin")) {  // HTTPS
    // if (https.begin(client,  "http://www.smwrap.ru/1MB_file.bin")) {  // HTTPS
        https.setTimeout(15000);
        https.addHeader("Content-Type", "application/json");
        start_time = micros();
        int httpCode = https.GET();
        end_time = micros();
        chunk_elapsed = end_time - start_time;
        if (httpCode > 0) {
            indQuery+=1;
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                indfailedConnection=0; 
                response_size = https.getSize();
                total_bytes += response_size;         
                total_elapsed += chunk_elapsed;       
                chunk_speed = ((response_size * 8) / 1000.0) / (chunk_elapsed / 1000000.0);
                Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
                Serial.print(indQuery);
                Serial.print(")... Speed ");
                Serial.print(chunk_speed);
                Serial.print(" Kb/s\n");
                //дом - 659.36 Kb
                //дом - 659.36 Kb
                if (chunk_speed<200) { // если скорость меньше 200 Kb/s тогда перегружаем  соединение
                    handleToggle();      
                    delay(5000);
                    handleToggle();
                    indQuery = 0;
                }
                // String payload = https.getString();
                // Serial.println(payload);
             }
         } else {
             indfailedConnection+=1;
             if (indfailedConnection == 4) { // если после 4 запросов интернет не появился, тогда перегружаем соединение
                 handleToggle();      
                 delay(10000);
                 handleToggle();
                 indfailedConnection = 0;
                 indQuery = 0;
             }
             Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
         }
         https.end();
    } else {
        indfailedConnection+=1;
        if (indfailedConnection == 4) { // если после 4 запросов интернет не появился, тогда перегружаем соединение
            handleToggle();      
            delay(10000);
            handleToggle();
            indfailedConnection = 0;
            indQuery = 0;
        }
        Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  delay(5000); // Wait 5 seconds before next request
}


void handleToggle() {
  relayState = !relayState;
  digitalWrite(relayPin, relayState ? LOW : HIGH);
  Serial.println(relayState ? "Relay toggled ON" : "Relay toggled OFF");
}
