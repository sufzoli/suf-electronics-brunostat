#pragma once
#include "TempSensor.h"
class TempSensor_Dummy :
	public TempSensor
{
public:
	TempSensor_Dummy()
	{
	}
	float GetTemp()
	{
		return 25.0;
	}
};

