#ifndef _SENSOR_DATA_H_
#define _SENSOR_DATA_H_

struct SensorData
{
    float Temperature = 0.0; //temperature in C
    float Humidity = 0.0; //humidity in %
    float Pressure = 0.0; //pressure in HPA
    float Altitude = 0.0; //altitude
    float Illuminance = 0.0; //illuminance in LUX

    SensorData() = default;
};

#endif