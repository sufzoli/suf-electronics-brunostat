#pragma once

#include "TempSensor.h"
#include "Heater.h"
#include "WebCallback.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
class Termostat : WebCallback
{
public:
	
	Termostat(TempSensor* sensor, Heater* heater, ESP8266WebServer* webserver, String tempseturi)
	{
		ts = sensor;
		heat = heater;
		server = webserver;
		WebCallbackSingleton::Instance(server, this, tempseturi, HTTP_POST);
		// server->on(tempseturi.c_str(), HTTP_POST, this->SetDelegate);
	}
	
	/*
	Termostat(TempSensor* sensor, Heater* heater, ESP8266WebServer* webserver)
	{
		ts = sensor;
		heat = heater;
		server = webserver;
		// server->on(tempseturi.c_str(), HTTP_POST, this->SetDelegate);
	}
	*/
	
	// This is the termostat itself
	void Worker()
	{
		int intCurrTemp;
		// read temerature
    Serial.println("geting sensor data");
		CurrentTemp = ts->GetTemp();
//    CurrentTemp = 25.0;
    Serial.println("processing sensor data");
		// convert temperature to 0.1 deg C integer format
		intCurrTemp = (int)(CurrentTemp * 10);
    Serial.println("converted to int");
		if (intCurrTemp < SetTemp - lower_th)
		{
      Serial.println("heater switching on");
			heat->Enable(true);
      Serial.println("heater is on");
			// webInsertionStrings["heatcolor"] = "red";
		}
		if (intCurrTemp > SetTemp + upper_th)
		{
      Serial.println("heater switching off");
			heat->Enable(false);
      Serial.println("heater is off");
			// webInsertionStrings["heatcolor"] = "green";
		}
	}
	void OnWebCallDelegate()
	{
		server->send(200, "text/plain", "<!DOCTYPE HTML>\n<html><head><link rel=\"icon\" href=\"about:blank\"></head><body><p id=\"ret_temp\">" + String(SetTemp) + "</p></body></html>");
	}
protected:
	ESP8266WebServer* server;
	TempSensor* ts;
	Heater* heat;
	float CurrentTemp;
	int lower_th;
	int upper_th;
	int SetTemp;
};

