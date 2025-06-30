#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "ApplicationSettings.h"
#include "SensorData.h"

class NetworkManager
{
    private:
        WiFiClient wiFiClient;

    public:
        NetworkManager();
        bool init();
        bool isConnected();
        int scanSettingsID(ApplicationSettings* aSettings, uint16_t nSettings);
        bool connectWiFi(WiFiConnection wiFiConnection, uint16_t retryAttempts = 15, uint16_t retryDelay = 20);
        void printWiFiInfo();
        void uploadSensorData(ThingSpeakInfo* thingSpeakInfo, SensorData* sensorData);
};

#endif