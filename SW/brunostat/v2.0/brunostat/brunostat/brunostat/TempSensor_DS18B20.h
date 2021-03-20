#pragma once
#include "TempSensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>
class TempSensor_DS18B20 :
	public TempSensor
{
public:

	TempSensor_DS18B20(int iopin)
	{
		OneWire oneWire(iopin);
		DS18B20 = DallasTemperature(&oneWire);
	}
	float GetTemp()
	{
		int i;
		float retvalue;
    Serial.println("inside gettemp");
		for (i = 0; i<10; i++)
		{
			DS18B20.requestTemperatures();
			retvalue = DS18B20.getTempCByIndex(0);
			if (retvalue != 85.0 && retvalue != (-127.0))
			{
				// dtostrf(temp_ValueFloat, 2, 2, temp_ValueStr);
				break;
			}
			delay(100);
		}
		return retvalue;
	}
protected:
	DallasTemperature DS18B20;
};

/*
void temp_ReadSensor()
{
	int i;
	strncpy(temp_ValueStr, "N/A", 6);
	for (i = 0; i<10; i++)
	{
		DS18B20.requestTemperatures();
		temp_ValueFloat = DS18B20.getTempCByIndex(0);
		if (temp_ValueFloat != 85.0 && temp_ValueFloat != (-127.0))
		{
			dtostrf(temp_ValueFloat, 2, 2, temp_ValueStr);
			break;
		}
		delay(100);
	}
	webInsertionStrings["CurrentTemp"] = temp_ValueStr;
}
*/
