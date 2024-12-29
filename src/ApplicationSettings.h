#ifndef _APPLICATION_SETTINGS_H_
#define _APPLICATION_SETTINGS_H_

#include "WiFiConnection.h"
#include "ThingSpeakInfo.h"

struct ApplicationSettings
{
    WiFiConnection WifiSettings;
    ThingSpeakInfo ThingSpeakSettings;

    ApplicationSettings() = default;

    ApplicationSettings(WiFiConnection wifiSettings, ThingSpeakInfo thingSpeakSettings)
    {
        WifiSettings = wifiSettings;
        ThingSpeakSettings = thingSpeakSettings;
    }
};

#endif