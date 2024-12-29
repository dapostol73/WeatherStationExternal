/*
Result of using the 32% & 84% Humidty packs

BME280 Sensors
Sensor: 1 Low: 31.1 High: 84.6
Sensor: 2 Low: 31.7 High: 78.0

DHT22 Sensors
Sensor: 1 Low: 35.4 High: 85.9
Sensor: 2 Low: 34.9 High: 87.4

DHT11 Sensors
Sensor: 1 Low: 33.0 High: 89.0
*/
#include <Arduino.h>
#include <Strings.h>
#include <ESP8266WiFi.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750FVI.h>

#include "UserSettings.h"
#include "SensorData.h"
#include "NetworkManager.h"

#ifndef SERIAL_LOGGING
// disable Serial output
#define Serial KillDefaultSerial
static class {
public:
    void begin(...) {}
    void print(...) {}
    void println(...) {}
} Serial;
#endif

// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
ApplicationSettings appSettings; //change to pointer
NetworkManager netManager;

// Uncomment the type of sensor in use:
WiFiClient client;

/***************************
 * Begin Atmosphere Sensor Settings
 **************************/
SensorData sensorData;
const int BME280_DEFAULT_I2CADDR = 0x76;  // address:0x76
const float SEALEVELPRESSURE_HPA = 1021.1;
void readTemperature();
void readHumidity();
void readAtmosphere();

/***************************
 * Begin Light Sensor Settings
 **************************/
void readLight();

/***************************
 * Begin Settings
 **************************/
// Setup
long readTime = LONG_MIN; 
long uploadTime = LONG_MIN;
const int SENSOR_INTERVAL_SECS = 15; // Sensor query every 15 seconds
const int UPLOAD_INTERVAL_SECS = 5 * 60; // Upload every 5 minutes

const uint16_t WIFI_UPDATE_SECS = 120; // wait 2 minutes to reconnect
long wiFiTimeLastUpdate = LONG_MIN;

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
Adafruit_BME280 bme280;
BH1750FVI bh1750(BH1750_DEFAULT_I2CADDR, BH1750_CONTINUOUS_HIGH_RES_MODE, BH1750_SENSITIVITY_DEFAULT, BH1750_ACCURACY_DEFAULT);

//declaring prototypes
void blinkLedStatus(int times, int pulse = 500);

void setup()
{
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	//initialize Atmosphere sensor
	if (!bme280.begin(BME280_DEFAULT_I2CADDR))
	{
		#ifdef SERIAL_LOGGING
		Serial.println("Could not find BME280 sensor at 0x76");
		#endif
	}
	else
	{
		#ifdef SERIAL_LOGGING
		Serial.println("Found BMP280 sensor at 0x76");
		#endif
	}

	//initialize Atmosphere sensor
	if (!bh1750.begin(SDA_PIN, SCL_PIN))
	{
		#ifdef SERIAL_LOGGING
		Serial.println("Could not find BH1750 sensor at default address");
		#endif
	}
	else
	{
		#ifdef SERIAL_LOGGING
		Serial.println("Fuond BH1750 sensor at default address");
		#endif
	}

	netManager.init();
    uint8_t appSetID = netManager.scanSettingsID(AppSettings, AppSettingsCount);
    appSettings = AppSettings[appSetID];
    netManager.connectWiFi(appSettings.WifiSettings);
	netManager.printWiFiInfo();
    
	delay(2000);
}

void loop()
{
    if (!netManager.isConnected() && millis() - wiFiTimeLastUpdate > (1000L*WIFI_UPDATE_SECS))
    {
        #ifdef SERIAL_LOGGING
        Serial.println("Attempting to connect to WiFi");
        #endif
        if (!netManager.connectWiFi(appSettings.WifiSettings));
        {
            //If a connection failed, rescan for new settings.
            uint8_t appSetID = netManager.scanSettingsID(AppSettings, AppSettingsCount);
            appSettings = AppSettings[appSetID];
        }
        wiFiTimeLastUpdate = millis();
    }

	//Read sensor values base on Upload interval seconds
	if(millis() - readTime > 1000L*SENSOR_INTERVAL_SECS)
	{
		blinkLedStatus(2);
		readTemperature();
		readHumidity();
		readAtmosphere();
		readLight();
		readTime = millis();
	}

	//Upload Sensor values base on Upload interval seconds
	if(millis() - uploadTime > 1000L*UPLOAD_INTERVAL_SECS)
	{
		blinkLedStatus(4, 250);
		netManager.uploadSensorData(&appSettings.ThingSpeakSettings, &sensorData);
		uploadTime = millis();
	}
}

void blinkLedStatus(int times, int pulse)
{
	for (int i = 0; i < times; i++)
	{
		digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on by making the voltage LOW
		delay(pulse);            // Wait for a second
		digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
		delay(pulse);            // Wait for two seconds
	}
}

float roundUpDecimal(float value, int decimals = 1)
{
	float multiplier = powf(10.0, decimals);
	value = roundf(value * multiplier) / multiplier;
	return value;
}

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void readTemperature()
{
	sensorData.Temperature = roundUpDecimal(bme280.readTemperature() - 1.5);
	#ifdef SERIAL_LOGGING
	Serial.println("Temperature: " + String(sensorData.Temperature));
	#endif
}

void readHumidity(){
	float reading = map(bme280.readHumidity(), 31.1, 84.6, 32, 84.0);
	sensorData.Humidity = roundUpDecimal(reading);
	//tempHmd = roundUpDecimal(bme280.readHumidity());
	#ifdef SERIAL_LOGGING
	Serial.println("Humidity: " + String(sensorData.Humidity));
	#endif
}

void readAtmosphere()
{
	sensorData.Pressure = roundUpDecimal(bme280.readPressure() / 100.0F);
	// approx low from https://vancouver.weatherstats.ca/charts/pressure_sea-hourly.html
	sensorData.Altitude = roundUpDecimal(bme280.readAltitude(SEALEVELPRESSURE_HPA));
	#ifdef SERIAL_LOGGING
	Serial.println("Pressure: " + String(sensorData.Pressure));
	Serial.println("Altitude: " + String(sensorData.Altitude));
	#endif
}

void readLight()
{
	sensorData.Illuminance = roundUpDecimal(bh1750.readLightLevel());
	#ifdef SERIAL_LOGGING
	Serial.println("Light Level: " + String(sensorData.Illuminance));
	#endif
}

