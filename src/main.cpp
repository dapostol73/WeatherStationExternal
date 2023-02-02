#include <ESP8266WiFi.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <DHT_U.h>
#include <BH1750FVI.h>

#include "WifiInfo.h"
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

WiFiConnection wiFiInfo("Unknown", "Invalid");

// Uncomment the type of sensor in use:
WiFiClient client;
const char *host = "api.thingspeak.com";                  //IP address of the thingspeak server
const char *api_key ="EMCNAORN3ZXKCFW1";                  //Your own thingspeak api_key
const int httpPort = 80;

/***************************
 * Begin DHT11 Settings
 **************************/
#define DHTTYP DHT22 // DHT 22 (AM2302)
#define DHTPIN 14    // Digital pin connected to the DHT sensor 
float tempTemp = 0.0; //temperature
float tempHmd = 0.0; //humidity
void readTemperatureHumidity();
void uploadSensorData();
long readTime = LONG_MIN; 
long uploadTime = LONG_MIN; 

/***************************
 * Begin Light Sensor Settings
 **************************/
void readLight();
float tempLight = 0.0;

/***************************
 * Begin Atmosphere Sensor Settings
 **************************/
void readAtmosphere();
const int I2C_ATOM_ADDRESS = 0x76;  // address:0x76
float tempAtom = 0.0;
float tempAlit = 0.0;

/***************************
 * Begin Settings
 **************************/
// Setup
const int SENSOR_INTERVAL_SECS = 15; // Sensor query every 15 seconds
const int UPLOAD_INTERVAL_SECS = 5 * 60; // Upload every 5 minutes

#if defined(ESP8266)
const int SDA_PIN = D2;
const int SCL_PIN = D1;
#else
//const int SDA_PIN = GPIO5;
//const int SCL_PIN = GPIO4 
const int SDA_PIN = GPIO0;
const int SCL_PIN = GPIO2 
#endif

/***************************
 * End Settings
 **************************/
// Initialize the sensors
DHT_Unified dht22(DHTPIN, DHTTYP);
BH1750FVI bh1750(BH1750_DEFAULT_I2CADDR, BH1750_CONTINUOUS_HIGH_RES_MODE, BH1750_SENSITIVITY_DEFAULT, BH1750_ACCURACY_DEFAULT);
Adafruit_BMP280 bmp280;

//declaring prototypes
void resolveWiFiInfo();

void setup() {
  Serial.begin(115200);
  delay(2000);

  //initialize Atmosphere sensor
  if (!bmp280.begin(I2C_ATOM_ADDRESS)) {
    Serial.println("Could not find BMP280 sensor at 0x76");
  }
  else {
    Serial.println("Found BMP280 sensor at 0x76");
  }

  //initialize Atmosphere sensor
  if (!bh1750.begin(SDA_PIN, SCL_PIN)) {
    Serial.println("Could not find BH1750 sensor at default address");
  }
  else {
    Serial.println("Fuond BH1750 sensor at default address");
  }


  // initlialize Humidity 
  dht22.begin();

  WiFi.mode(WIFI_STA);
  resolveWiFiInfo();
  WiFi.begin(wiFiInfo.SSID, wiFiInfo.Password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
  }
  
  Serial.println("");
  while (!client.connect(host, httpPort)) {
    Serial.println("Connection Failed");
  }  
}

void loop() {  
  //Read sensor values base on Upload interval seconds
  if(millis() - readTime > 1000L*SENSOR_INTERVAL_SECS){
    readTemperatureHumidity();
    readLight();
    readAtmosphere();
    readTime = millis();
  }

  //Upload Sensor values base on Upload interval seconds
  if(millis() - uploadTime > 1000L*UPLOAD_INTERVAL_SECS){
    uploadSensorData();
    uploadTime = millis();
  }
}

void resolveWiFiInfo()
{
	int8_t numNetworks = WiFi.scanNetworks();
  Serial.println("Number of networks found: " + String(numNetworks));

	for (uint8_t i=0; i<numNetworks; i++)
	{
		String ssid = WiFi.SSID(i);
		Serial.println(WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + ")");
		for (uint8_t j=0; j < WiFiConnectionsCount; j++)
		{
      if (strcasecmp(WiFiConnections[j].SSID, ssid.c_str()) == 0)
			{
				//Serial.println("Found: " + String(ssid));
				WiFiConnections[j].Avialable = true;
				WiFiConnections[j].Strength = WiFi.RSSI(i);

				if (WiFiConnections[j].Strength > wiFiInfo.Strength)
				{
					wiFiInfo = WiFiConnections[j];
				}
			}
		}
	}

	Serial.println("Using WiFi " + String(wiFiInfo.SSID));	
}

float roundUpDecimal(float value, int decimals = 1) {
  float multiplier = powf(10.0, decimals);
  value = roundf(value * multiplier) / multiplier;
  return value;
}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void readTemperatureHumidity(){
  sensors_event_t event;
  dht22.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    tempTemp = roundUpDecimal(event.temperature-1.2);
    Serial.println("Temperature: " + String(tempTemp));
  }
  
  dht22.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    tempHmd = map(event.relative_humidity, 20.6, 69.5, 40.0, 75.0);
		tempHmd = roundUpDecimal(tempHmd);
    Serial.println("Humidity: " + String(tempHmd));
  }
}

void readLight() {
  tempLight = roundUpDecimal(bh1750.readLightLevel());
  Serial.println("Light Level: " + String(tempLight));
}


void readAtmosphere(){
  tempAtom = roundUpDecimal(bmp280.readPressure()/100);
  // approx low from https://vancouver.weatherstats.ca/charts/pressure_sea-hourly.html
  tempAlit = roundUpDecimal(bmp280.readAltitude(1021.1));
  
  Serial.println("Pressure: " + String(tempAtom));
  Serial.println("Altitude: " + String(tempAlit));
}

//upload temperature humidity data to thinkspak.com
void uploadSensorData(){
   if(!client.connect(host, httpPort)){
    Serial.println("connection failed");
    return;
  }
  // Three values(field1 field2 field3 field4) have been set in thingspeak.com 
  client.print(String("GET ") + "/update?api_key="+api_key+"&field1="+tempTemp+"&field2="+tempHmd+"&field3="+tempLight+"&field4="+tempAtom+"&field5="+tempAlit+" HTTP/1.1\r\n" +"Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println("Updated ThingSpeak");
}


