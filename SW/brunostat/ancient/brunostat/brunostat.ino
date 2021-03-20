/*
 Name:		brunostat.ino
 Created:	9/5/2016 11:03:34 PM
 Author:	zoli
*/

/*********
Baed on: 

Rui Santos
Complete project details at http://randomnerdtutorials.com
*********/

// the setup function runs once when you press reset or power the board
/*
#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
*/


// Including the ESP8266 WiFi library
#include <user_interface.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <FS.h>

// Replace with your network details
const char* ssid = "RI-DENT";
const char* password = "nd37tUONpRkt9TfcFUB6";
const int settemp = 250;
const int upper_th= 5;
const int lower_th= 5;

// Data wire is plugged into pin D1 on the ESP8266 12-E - GPIO 5
#define ONE_WIRE_BUS 5

WiFiClient client;
float CurrentTemp;
int SetTemp;
os_timer_t ThermoTimer;
bool Heat;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);
char temperatureCString[6];
bool IsProcessTemp;

// Web Server on port 80
WiFiServer server(80);


void getTemperature()
{
  //float tempC;
  int i;
  strncpy(temperatureCString, "N/A", 6);
  for(i=0;i<10;i++)
  {
    DS18B20.requestTemperatures();
    CurrentTemp = DS18B20.getTempCByIndex(0);
    if(CurrentTemp != 85.0 && CurrentTemp != (-127.0))
    {
      dtostrf(CurrentTemp, 2, 2, temperatureCString);
      break;
    }
    delay(100);
  }
}



void ThermoTimerCallback(void *pArg)
{
  // getTemperature();
  IsProcessTemp= true;
}


// only runs once on boot
void setup()
{
  // Initialize relay output
  pinMode(4, OUTPUT);
  digitalWrite(4, 0);
  Heat = false;

	// Initializing serial port for debugging purposes
	Serial.begin(115200);
	delay(10);

  SPIFFS.begin();

  

	DS18B20.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

					 // Connecting to WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");

	// Starting the web server
	server.begin();
	Serial.println("Web server running. Waiting for the ESP IP...");

  getTemperature();
  SetTemp = settemp;
  IsProcessTemp = false;
  
	delay(10000);

	// Printing the ESP IP address
	Serial.println(WiFi.localIP());
  os_timer_setfn(&ThermoTimer, ThermoTimerCallback, NULL);
  os_timer_arm(&ThermoTimer, 1000, true);
}


void httpSendOkHeader()
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
}

void httpSendErr()
{
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><link rel=\"icon\" href=\"about:blank\"></head><body></body></html>");
}

void httpSendTemp()
{
  Serial.println("Sending header");
  httpSendOkHeader();
  Serial.println("Header sent");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><link rel=\"icon\" href=\"about:blank\"><meta http-equiv=\"refresh\" content=\"10\" /></head>");
  client.println("<body bgcolor=\"" + String(Heat ? "red" : "green") + "\" >");
  client.println("<p style=\"font-family:courier,'courier new',monospace;font-size: 4em;font-weight: bold;margin: 0px 0px 0px 0px;\">");
  client.println(temperatureCString);
  client.println("</p></body></html>");
}

void httpSendSetTemp()
{
  httpSendOkHeader();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><link rel=\"icon\" href=\"about:blank\"></head><body><p id=\"ret_temp\">" + String(SetTemp) + "</p></body></html>");
}

void httpSendMain()
{
  httpSendOkHeader();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><link rel=\"icon\" href=\"about:blank\"></head><body>");
  client.println("<p style=\"font-family:courier,'courier new',monospace;font-size: 2em;font-weight: bold; text-align:center; margin: 0px 0px 0px 0px;\">BrunoStat</p>");
  client.println("<iframe id=\"ct_frame\" height=\"90\" width=\"210\" src=\"/currenttemp\" frameBorder=\"0\" scrolling=\"no\" style=\"margin: auto; display: block;\"></iframe>");
  client.println("<iframe id=\"st_frame\" height=\"0\" width=\"0\" src=\"/settemp\" style=\"visibility:hidden;display:none\" onload=\"get_target_temp();\"></iframe>");
  client.println("<center><form style=\"padding-top: 10px;\">");
  client.println("<input type=\"text\" id=\"set_temp\" value=\"\" size=\"4\" style=\"font-weight: bold; text-align:center;\" /><b>&deg;C</b>");
  client.println("<input type=\"button\" name=\"btn_up\" value=\"&#x25B2;\" onclick=\"change_temp(5);\" />");
  client.println("<input type=\"button\" name=\"btn_down\" value=\"&#x25BC;\" onclick=\"change_temp(-5);\" />");
  client.println("<input type=\"button\" name=\"btn_set\" value=\"SET\" onclick=\"set_desired_temp();\" />");
  client.println("</form></center>");
  client.println("<script type=\"text/javascript\">");
  client.println("function get_temp_value() {");
  client.println("var temp_box = document.getElementById('set_temp');");
  client.println("return parseInt(temp_box.value.replace('.','')); }");
  client.println("function set_temp_value(value) {");
  client.println("var temp_box = document.getElementById('set_temp');");
  client.println("temp_box.value = value.substr(0,value.length-1) + '.' + value.substr(value.length-1,1); }");
  client.println("function change_temp(value) {");
  client.println("var temp_int = get_temp_value();");
  client.println("set_temp_value((temp_int + value).toString()); }");
  client.println("function set_desired_temp() {");
  client.println("document.getElementById('st_frame').src = '/settemp=' + document.getElementById('set_temp').value.replace('.',''); }");
  client.println("function get_target_temp() {");
  client.println("var target_frame = document.getElementById('st_frame');");
  client.println("var temp_box = document.getElementById('set_temp');");
  client.println("var win = target_frame.contentWindow;");
  client.println("var doc = target_frame.contentDocument? target_frame.contentDocument: target_frame.contentWindow.document;");
  client.println("var ret_temp = doc.getElementById('ret_temp');");
  client.println("set_temp_value(ret_temp.innerHTML.replace(' ','')); }");
  client.println("</script>");
  client.println("</body></html>");
}

// runs over and over again
void loop()
{
  String httpMethod;
  String httpRequest;
  float tempC;
  int paramsindex;
  String httpParam;
  int intCurrTemp;
  // Termosthat function
  if(IsProcessTemp)
  {
    // read temerature
    getTemperature();
    // convert temperature to 0.1 deg C integer format
    intCurrTemp = (int)(CurrentTemp * 10);
    if(intCurrTemp < SetTemp - lower_th)
    {
      digitalWrite(4, 1);
      Heat = true;
    }
    if(intCurrTemp > SetTemp + upper_th)
    {
      digitalWrite(4, 0);
      Heat = false;
    }
    IsProcessTemp = false;
  }
	// Listenning for new clients
	client = server.available();

	if (client)
	{
    Serial.println("New client");
		// bolean to locate when the http request ends
		boolean blank_line = true;
		while (client.connected())
		{
		  if (client.available())
		  {
        httpMethod = client.readStringUntil(' ');
        httpRequest = client.readStringUntil(' ');
        paramsindex = httpRequest.indexOf("=");
        httpParam = "";
        if (paramsindex != -1)
        {
          httpParam = httpRequest.substring(paramsindex + 1, httpRequest.length());
          httpRequest = httpRequest.substring(0, paramsindex);
        }
        
        client.readStringUntil('\r'); // the HTTP/1.1 text, can be ignored
        Serial.println(httpRequest);
        Serial.println(httpParam);
        client.flush();
        if(httpRequest == "/")
        {
            httpSendMain();
            break;
        }
        if(httpRequest == "/currenttemp")
        {
            httpSendTemp();
            break;
        }
        if(httpRequest == "/settemp")
        {
          if(httpParam != "")
          {
            SetTemp = httpParam.toInt();
          }
          httpSendSetTemp();
          break;
        }
        httpSendErr();
				break;
			}
		}
		// closing the client connection
		delay(1);
		// client.flush();
		client.stop();
		Serial.println("Client disconnected.");
	}
}


