#include <inttypes.h>
#include <Wire.h>
#include "SPI.h"

// ESP8266 header
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Sensor header
#include <ADXL362.h>
#include "MAX30100.h"


#define SAMPLING_RATE                       MAX30100_SAMPRATE_100HZ
#define IR_LED_CURRENT                      MAX30100_LED_CURR_24MA
#define RED_LED_CURRENT                     MAX30100_LED_CURR_24MA
#define PULSE_WIDTH                         MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE                        true

#define SAMPLES_PER_POST          250

// JSON definition
char json_str[25000];
size_t json_offset = 0;
char currentTime[100];

// SENSOR definition
int samples_taken = 0;

WiFiClient client;

// MAX30100
MAX30100 maxSensor;
uint16_t ir;
uint16_t red;

// ADXL Accelerator
ADXL362 xl;
int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;
int16_t chipSelpin = 15;

LEDCurrent RED_LED_CURRENT_CHANGE = MAX30100_LED_CURR_24MA;
LEDCurrent IR_LED_CURRENT_CHANGE = MAX30100_LED_CURR_24MA;
void setup()
{
  // Setting serial
  Serial.begin(115200);

  // Connect to WiFi
  connectToWifi();

  // Print WiFi config
  Serial.println("WiFi config");
  Serial.println(WiFi.localIP());
  byte mac[6];
  WiFi.macAddress(mac);
  char MAC_char[18] = "";
  for (int i = 0; i < sizeof(mac); ++i) {
    sprintf(MAC_char, "%s%02x:", MAC_char, mac[i]);
  }
  Serial.print("Mac address: ");
  Serial.println(MAC_char);

  // Initialize Sensor
  setupSensor();

  // Reset JSON string
  resetJsonString();

  // Connect to server
  connectToServer();

  maxSensor.resetFifo();
}


// ESP8266 loop
void loop()
{
  while(1)
  {
      maxSensor.update();
      // maxSensor.resetFifo();
      while (readSensorData()) {
      samples_taken++;
  
      // Read Current Time
      // getCurrentTime();
  
      updateJsonString();
      // Create JSON data
  
      if (samples_taken >= SAMPLES_PER_POST)
      {
        // Post To Server
        postToServer();
  
        // Get Current Level of Board
        getIRCurrentLevel();
        getREDCurrentLevel();
  
        // Change Current Level of Board
        changeLEDCurrent();
  
        // Reset JSON data
        resetJsonString();
  
        // Reset samples_taken
        samples_taken = 0;
  
        maxSensor.resetFifo();
      }
    }
  }
  
}

/****************
**WIFI FUNCTION**
****************/

// Reset JSON String
void resetJsonString() {
  json_offset = 0;
  json_offset += sprintf(json_str, "{\"samples\":[");
}

void updateJsonString() {
  char *json_str_at_offset = &json_str[json_offset];

  json_offset += sprintf(json_str_at_offset, "{\"time\":\"%.3lf\",", getCurrentTime());
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"ir\":%"PRIu16",", ir);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"r\":%"PRIu16",", red);
  json_str_at_offset = &json_str[json_offset];

  json_offset += sprintf(json_str_at_offset, "\"x\":%d,", XValue);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"y\":%d,", YValue);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"z\":%d,", ZValue);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"ir_c\":%d, ", RED_LED_CURRENT_CHANGE);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"red_c\":%d},", IR_LED_CURRENT_CHANGE);
}

void connectToWifi()
{
  // WiFi begin
  WiFi.begin("JC-DESKTOP", "99189918");
  Serial.print("Connect to wifi");

  // Waiting WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// Connect To Server
void connectToServer()
{
  // Connect To Server
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      connectToWifi();
    }
    client.connect("192.168.137.1", 8080);
  }
  //  client.setNoDelay(true);
}

// Get Current Time
double getCurrentTime()
{
  static double serverTime = -1;
  static unsigned long callibrationTime = -1;
  if (serverTime == -1) {
    // Time Stamp URL
    String url = "http://192.168.137.1:8080/time";
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode != 200) {
        Serial.printf("Response: %d\n", httpCode);
        return -1;
      }
      // get current time from server
      serverTime = strtod(http.getString().c_str(), NULL);
      http.end();

      callibrationTime = millis();
    }
    else {
      Serial.println("WIFI not connected, POST aborted.");
      return -1;
    }
  }
  return serverTime + ((double)millis() - callibrationTime) / 1000.0;
}

// Post To Server
void postToServer()
{
  // Finalize JSON
  json_str[json_offset - 1] = ']';
  json_offset += sprintf(json_str + json_offset, "}\0");

  String url = "http://192.168.137.1:8080/sensorData";

  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    // HTTP Client definition
    HTTPClient http;

    // HTTP begin
    http.begin(url);

    // Send the Request
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "keep-alive");

    http.quickPOST((uint8_t*)json_str, json_offset);
  }
}

// Set Up All Sensor
void setupSensor()
{
  // Initialize MAX30100
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);

  Serial.print("Initializing MAX30100: ");
  if (!maxSensor.begin()) {
    Serial.println("Failed");
  }
  else {
    Serial.println("Sucess");
  }

  // MAX30100 Configuration
  maxSensor.setMode(MAX30100_MODE_SPO2_HR);
  maxSensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
  maxSensor.setLedsPulseWidth(PULSE_WIDTH);
  maxSensor.setSamplingRate(SAMPLING_RATE);
  maxSensor.setHighresModeEnabled(HIGHRES_MODE);

  // Initialize ADXL362
  xl.begin(chipSelpin);
  xl.beginMeasure();              // switch adxl362 to measure mode
}

// Read All Sensor Data
bool readSensorData()
{
  xl.readXYZTData(XValue, YValue, ZValue, Temperature);
  return maxSensor.getRawValues(&ir, &red);
}


// Change IR, RED Current Level
void changeLEDCurrent()
{
  maxSensor.setLedsCurrent(IR_LED_CURRENT_CHANGE, IR_LED_CURRENT_CHANGE);
}


// Get IR Current Level
void getIRCurrentLevel()
{
  // Create IR Current URL
  String url = "http://192.168.137.1:8080/getIRCurrent";
  Serial.println(String("GET to ") + url);


  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    // HTTP Client definition
    HTTPClient http;

    // HTTP begin
    http.begin(url);

    // Send the Request
    int httpCode = http.GET();

    // Store IR current data
    //String payload = "\"" + http.getString() + "\"";
    String payload = http.getString();

    // Parse To Int and Set IR_RED_CURRENT
    IR_LED_CURRENT_CHANGE = (LEDCurrent)payload.toInt();
    Serial.print("CHANGED IR: ");
    Serial.println(IR_LED_CURRENT_CHANGE);

    // Print HTTP return code
    Serial.println("response code: " + String(httpCode));

    // Print request response payload
    Serial.println("response data: " + payload + "\n\n");

    // Close connection
    http.end();

    if (httpCode != 200)
    {
      Serial.print("HTTP Code for IR Current : ");
      Serial.println(httpCode);
      return;
    }

    return;
  }
  else
  {
    Serial.println("WIFI not connected, POST aborted.");
    return;
  }
}


// Get RED Current Level
void getREDCurrentLevel()
{
  // Create IR Current URL
  String url = "http://192.168.137.1:8080/getREDCurrent";
  Serial.println(String("GET to ") + url);


  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    // HTTP Client definition
    HTTPClient http;

    // HTTP begin
    http.begin(url);

    // Send the Request
    int httpCode = http.GET();

    // Store RED current data
    //String payload = "\"" + http.getString() + "\"";
    String payload = http.getString();

    // Parse To Int and Set RED_LED_CURRENT
    RED_LED_CURRENT_CHANGE = (LEDCurrent)payload.toInt();
    Serial.print("CHANGED RED: ");
    Serial.println(RED_LED_CURRENT_CHANGE);

    // Print HTTP return code
    Serial.println("response code: " + String(httpCode));

    // Print request response payload
    Serial.println("response data: " + payload + "\n\n");

    // Close connection
    http.end();

    if (httpCode != 200)
    {
      Serial.print("HTTP Code for IR Current : ");
      Serial.println(httpCode);
      return;
    }

    return;
  }
  else
  {
    Serial.println("WIFI not connected, POST aborted.");
    return;
  }
}

