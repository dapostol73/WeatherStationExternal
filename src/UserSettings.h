#ifndef _USER_SETTINGS_H_
#define _USER_SETTINGS_H_

#include "ApplicationSettings.h"

ApplicationSettings Home(WiFiConnection("homessid", "homepw123"), ThingSpeakInfo("homeWriteAPIKey", "homeReadAPIKey"));
ApplicationSettings Office(WiFiConnection("officessid", "officepw123"), ThingSpeakInfo("officeWriteAPIKey", "officeReadAPIKey"));

ApplicationSettings AppSettings[] = { Home, Office };
uint8_t AppSettingsCount = sizeof(AppSettings) / sizeof(AppSettings[0]);

#endif