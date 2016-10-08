#ifndef TERMOSTAT_H
#define TERMOSTAT_H

class TempSensor
{
  public:
    virtual float GetTemp();
}

class Termostat
{
  public:
    Termostat(TempSensor sensor)
    {
      ts = sensor;
    }
    
    // This is the termostat itself
    void Worker()
    {
      int intCurrTemp;
      // read temerature
      ts.;
  // convert temperature to 0.1 deg C integer format
  intCurrTemp = (int)(CurrentTemp * 10);
  if(intCurrTemp < SetTemp - lower_th)
  {
    digitalWrite(4, 1);
    Heat = true;
    webInsertionStrings["heatcolor"] = "red";
  }
  if(intCurrTemp > SetTemp + upper_th)
  {
    digitalWrite(4, 0);
    Heat = false;
    webInsertionStrings["heatcolor"] = "green";
  }
  temp_IsProcess = false;  
}
  protected:
    TempSensor ts;
    float CurrentTemp;
}

