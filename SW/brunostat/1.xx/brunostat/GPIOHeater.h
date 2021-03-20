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
		digitalWrite(iopin, heat ? 1 : 0);
		Heating = heat;
	}
protected:
	int iopin;
};

