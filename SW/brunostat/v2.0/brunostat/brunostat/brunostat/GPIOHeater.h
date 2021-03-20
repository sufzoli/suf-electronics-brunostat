#pragma once
#include "Heater.h"
class GPIOHeater : public Heater
{
public:
	GPIOHeater(int pin)
	{
		iopin = pin;
		pinMode(iopin, OUTPUT);
		digitalWrite(iopin, 0);
		Heating = false;
	}
	void Enable(bool heat)
	{
    Serial.println("Change output");
		digitalWrite(iopin, heat ? 1 : 0);
		Heating = heat;
	}
	bool Heating;
protected:
	int iopin;
};

