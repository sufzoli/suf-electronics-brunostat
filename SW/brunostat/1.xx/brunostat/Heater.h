#pragma once
class Heater
{
public:
  Heater() {}
	virtual void Enable(bool enable) {}
  bool Heating;
};

