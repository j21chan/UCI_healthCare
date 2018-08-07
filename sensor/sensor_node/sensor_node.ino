#include <inttypes.h>
#include <Wire.h>
#include "SPI.h"

// ESP8266 header
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Sensor header
#include <ADXL362.h>
#include "MAX30100.h"

#define SAMPLING_RATE MAX30100_SAMPRATE_100HZ
#define PULSE_WIDTH MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE true

#define SAMPLES_PER_POST 300

// JSON definition
char json_str[30000];
size_t json_offset = 0;

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

LEDCurrent currentRedCurrent = MAX30100_LED_CURR_24MA;
LEDCurrent currentIRCurrent = MAX30100_LED_CURR_24MA;
LEDCurrent nextRedCurrent, nextIRCurrent;

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
  for (int i = 0; i < sizeof(mac); ++i)
  {
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
  maxSensor.fixedUpdate();

  while (readSensorData())
  {
    // Create JSON data
    updateJsonString();

    samples_taken++;

    if (samples_taken >= SAMPLES_PER_POST)
    {
      // Communicate with Server
      postToServer();

      // Reset JSON data
      resetJsonString();
      // Reset samples_taken
      samples_taken = 0;
    }
  }
}

/*
* Name: void resetJsonString()
* Function: Resets JSON buffer
*/
void resetJsonString()
{
  json_offset = 0;
  json_offset += sprintf(json_str, "{\"samples\":[");
}

/*
* Name: void updateJsonString()
* Function: Write datas to the JSON buffer
*/
void updateJsonString()
{
  char *json_str_at_offset = &json_str[json_offset];

  json_offset += sprintf(json_str_at_offset, "{\"time\":\"%.3lf\",", getCurrentTime());
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"ir\":%" PRIu16 ",", ir);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"r\":%" PRIu16 ",", red);
  json_str_at_offset = &json_str[json_offset];

  json_offset += sprintf(json_str_at_offset, "\"x\":%d,", XValue);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"y\":%d,", YValue);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"z\":%d,", ZValue);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"ir_c\":%d, ", currentIRCurrent);
  json_str_at_offset = &json_str[json_offset];
  json_offset += sprintf(json_str_at_offset, "\"red_c\":%d},", currentRedCurrent);
}

void connectToWifi()
{
  // WiFi begin
  WiFi.begin("HJWIFI", "11181118");
  Serial.print("Connect to wifi");

  // Waiting WiFi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}

// Connect To Server
void connectToServer()
{
  // Connect To Server
  while (!client.connected())
  {
    if (WiFi.status() != WL_CONNECTED)
    {
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
  if (serverTime == -1)
  {
    // Time Stamp URL
    String url = "http://192.168.137.1:8080/time";
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode != 200)
      {
        Serial.printf("Response: %d\n", httpCode);
        return -1;
      }
      // get current time from server
      serverTime = strtod(http.getString().c_str(), NULL);
      http.end();

      callibrationTime = millis();
    }
    else
    {
      Serial.println("WIFI not connected, Failed to get current time");
      return -1;
    }
  }
  return serverTime + ((double)millis() - callibrationTime) / 1000.0;
}

/*
* Name: void postToServer()
* Function : sends JSON data to server and recieves, adjust LED current
*/
void postToServer()
{
  // Finish JSON
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

    http.POST((uint8_t *)json_str, json_offset);

    String payload = http.getString();

    // Parse To Int and Set IR_RED_CURRENT
    nextIRCurrent = (LEDCurrent)(payload.toInt() >> 4);
    nextRedCurrent = (LEDCurrent)(payload.toInt() & 0x0f);
 
    if(nextIRCurrent != currentIRCurrent || nextRedCurrent != currentRedCurrent) {
        maxSensor.setLedsCurrent(nextIRCurrent, nextRedCurrent);
        currentRedCurrent = nextRedCurrent;
        currentIRCurrent = nextIRCurrent;
    }
  }
  else
    Serial.println("WIFI not connected, Failed to send data to Server");
}

// Set Up All Sensor`
void setupSensor()
{
  // Initialize MAX30100
  Serial.print("Initializing MAX30100: ");
  if (!maxSensor.begin())
    Serial.println("Failed");
  else
    Serial.println("Success");

  // MAX30100 Configuration
  maxSensor.setMode(MAX30100_MODE_SPO2_HR);
  maxSensor.setLedsCurrent(currentIRCurrent, currentRedCurrent);
  maxSensor.setLedsPulseWidth(PULSE_WIDTH);
  maxSensor.setSamplingRate(SAMPLING_RATE);
  maxSensor.setHighresModeEnabled(HIGHRES_MODE);

  // Initialize ADXL362
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  xl.begin(chipSelpin);
  xl.beginMeasure();
}

// Read All Sensor Data
bool readSensorData()
{
  xl.readXYZTData(XValue, YValue, ZValue, Temperature);
  return maxSensor.getRawValues(&ir, &red);
}
