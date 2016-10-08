#include <user_interface.h>
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FS.h>
#include "HashMap.h"
// #include <TextFinder.h>


#define SERIAL_DEBUG
#define SERIAL_BAUD 115200
#define CONFIG_PATH "/config.ini"

#define SITE_PATH "/site"
#define TEMPLATE_EXT "tpl"
#define TEMPLATE_TAG_BEGIN "<%"
#define TEMPLATE_TAG_END "%>"
#define TEMPLATE_DATATYPE "text/html"

const int upper_th= 5;
const int lower_th= 5;

CreateHashMap(webDataTypes, String, char* , 20);
CreateHashMap(sysConfig, String, String , 20);
CreateHashMap(webInsertionStrings, String, String, 20);
ESP8266WebServer server(80);

File web_UploadFile;
File web_UpgradeFile;

// Data wire is plugged into pin D1 on the ESP8266 12-E - GPIO 5
#define ONE_WIRE_BUS 14
os_timer_t temp_TimerObj;
bool temp_IsProcess;
float temp_ValueFloat;
char temp_ValueStr[6];
int temp_Set;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

bool ContentDebug;

void temp_ReadSensor()
{
  int i;
  strncpy(temp_ValueStr, "N/A", 6);
  for(i=0;i<10;i++)
  {
    DS18B20.requestTemperatures();
    temp_ValueFloat = DS18B20.getTempCByIndex(0);
    if(temp_ValueFloat != 85.0 && temp_ValueFloat != (-127.0))
    {
      dtostrf(temp_ValueFloat, 2, 2, temp_ValueStr);
      break;
    }
    delay(100);
  }
  webInsertionStrings["CurrentTemp"] = temp_ValueStr;
}


void temp_TimerCallback(void *pArg)
{
  temp_IsProcess = true;
}

void init_TempSensor()
{
  temp_IsProcess = false;
  os_timer_setfn(&temp_TimerObj, temp_TimerCallback, NULL);
  os_timer_arm(&temp_TimerObj, 1000, true);
}


void wifi_Init_ClientMode(String ssid, String password)
{
#ifdef SERIAL_DEBUG
  Serial.println("Wi-Fi: Client Mode");
  Serial.print("Wi-Fi: Connecting to ");
  Serial.println(ssid);
#endif
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
#ifdef SERIAL_DEBUG
    Serial.print(".");
#endif
  }
#ifdef SERIAL_DEBUG
  Serial.println();
  Serial.print("Wi-Fi: Connected - ");
  Serial.println(WiFi.localIP());
#endif
}

void spiffs_Init()
{
#ifdef SERIAL_DEBUG
  Serial.println("SPIFFS: Start filesystem");
#endif  
  SPIFFS.begin();
}

void web_HandleUpload()
{
  int i;
  String filename;
  HTTPUpload& upload = server.upload();
  switch(upload.status)
  {
    case UPLOAD_FILE_START:
      filename = upload.name;
    #ifdef SERIAL_DEBUG
      Serial.println("WEB: File Upload");
      Serial.print("WEB: Filename - ");
      Serial.println(filename);
    #endif
      web_UploadFile = SPIFFS.open(filename, "w");
      if(!web_UploadFile)
      {
        Serial.println("File open failed");
      }
      break;
    case UPLOAD_FILE_WRITE:
      if(web_UploadFile)
      {
        web_UploadFile.write(upload.buf, upload.currentSize);
        Serial.print("Write bytes - ");
        Serial.println(upload.currentSize);
      }
      break;
    case UPLOAD_FILE_END:
      if(web_UploadFile)
      {
        web_UploadFile.close();
        Serial.print("Total - ");
        Serial.println(upload.totalSize);
        Serial.println("File Closed");
      }
      break;
  }
}

void web_HandleUpgrade()
{
  int filesize;
//  int i;
//  String filename;
  HTTPUpload& upload = server.upload();
  switch(upload.status)
  {
    case UPLOAD_FILE_START:
//      filename = upload.name;
    #ifdef SERIAL_DEBUG
      Serial.println("WEB: Firmware Upgrade");
    #endif
      web_UpgradeFile = SPIFFS.open("/firmware.bin", "w");
      if(!web_UpgradeFile)
      {
        Serial.println("File open failed");
      }
      break;
    case UPLOAD_FILE_WRITE:
      if(web_UpgradeFile)
      {
        web_UpgradeFile.write(upload.buf, upload.currentSize);
//    #ifdef SERIAL_DEBUG
//        Serial.print("Write bytes - ");
//        Serial.println(upload.currentSize);
//    #endif
      }
      break;
    case UPLOAD_FILE_END:
      if(web_UpgradeFile)
      {
        filesize = upload.totalSize;
        web_UpgradeFile.close();
      #ifdef SERIAL_DEBUG  
        Serial.println("File Closed");
        Serial.println("File Closed");
        Serial.print("Uploaded Firmware Size - ");
        Serial.println(filesize);
      #endif
        // reopen for reading
        web_UpgradeFile = SPIFFS.open("/firmware.bin", "r");
        if(web_UpgradeFile)
        {
          if (!ESP.updateSketch(web_UpgradeFile, filesize))
          {
            #ifdef SERIAL_DEBUG
              Serial.println("Update failed");
            #endif
          }
        }
        else
        {
            #ifdef SERIAL_DEBUG
              Serial.println("Can't open upgradefile");
            #endif
        }
      }
      break;
  }
}
/*
String readStringUntilStr(File src, String searchString)
{
  String retvalue = "";
  do
  {
    retvalue += (char)src.read();
    if(retvalue.substring(retvalue.length() - searchString.length()) == searchString)
    {
      retvalue = retvalue.substring(0, retvalue.length() - searchString.length());
      break;
    }
  } while(src.available() > 0);
  return retvalue;
}
*/
String readStringUntilStr(Stream* src, String searchString)
{
  String retvalue = "";
  do
  {
    retvalue += (char)src->read();
    if(retvalue.substring(retvalue.length() - searchString.length()) == searchString)
    {
      retvalue = retvalue.substring(0, retvalue.length() - searchString.length());
      break;
    }
  } while(src->available() > 0);
  return retvalue;
}


void web_HandleFiles()
{
  #ifdef SERIAL_DEBUG
    Serial.print("WEB: File request - ");
    Serial.println(server.uri());
  #endif
  String path = SITE_PATH + server.uri();
  String extension = path.substring(path.lastIndexOf(".") + 1);
  String dataType = "text/plain";
  String content = "";
  String template_tag;
  signed int keyindex;
  
  #ifdef SERIAL_DEBUG
    Serial.print("WEB: Extension - ");
    Serial.println(extension);
    Serial.println(keyindex);
    Serial.println(webDataTypes[extension]);
    Serial.println(path);
  #endif
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if(dataFile)
  {
    if(extension == TEMPLATE_EXT)
    {
      dataType = TEMPLATE_DATATYPE;
      Serial.println("WEB - Process template");
      // read file
      do
      {
        content += readStringUntilStr(&dataFile, TEMPLATE_TAG_BEGIN);
        Serial.println(content);
        // ContentDebug = false;
        if(dataFile.position() >= dataFile.size())
        {
          break;
        }
        template_tag = readStringUntilStr(&dataFile, TEMPLATE_TAG_END);
        Serial.print("WEB - Insertiontag: ");
        Serial.println(template_tag);
        keyindex = webInsertionStrings.indexOf(template_tag);
        if(keyindex > -1)
        {
          Serial.println("Tag found");
          content += webInsertionStrings[template_tag];
        }
      } while(dataFile.position() < dataFile.size());
      
      server.send(200, dataType, content);
    }
    else
    {
      keyindex = webDataTypes.indexOf(extension);
      if(keyindex > -1)
      {
        Serial.println("Extension found");
        dataType = webDataTypes[extension];
      }
    #ifdef SERIAL_DEBUG
      Serial.print("WEB: DataType - ");
      Serial.println(dataType);
    #endif
      server.streamFile(dataFile, dataType);
    }
    dataFile.close();
  }
}

void temp_SetTemp()
{
  Serial.println("WEB: /settemp requested");
  if(server.hasArg("temp"))
  {
    temp_Set = server.arg("temp").toInt();
  }
  server.send(200, "text/html","<!DOCTYPE HTML><html><head><link rel=\"icon\" href=\"about:blank\"></head><body><p id=\"ret_temp\">" + String(temp_Set) + "</p></body></html>");
}


void web_Init()
{
#ifdef SERIAL_DEBUG
  Serial.println("WEB: Initialize Server");
#endif
// Read config from file


  server.on("/uploadfile", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, web_HandleUpload);
  server.on("/upgrade", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, web_HandleUpgrade);
  server.on("/settemp", HTTP_GET, temp_SetTemp);
  server.onNotFound(web_HandleFiles);
  server.begin();

  webDataTypes["htm"] = "text/html";
  webDataTypes["html"] = "text/html";
  webDataTypes["css"] = "text/css";
  webDataTypes["js"] = "application/javascript";
  webDataTypes["png"] = "image/png";
  webDataTypes["gif"] = "image/gif";
  webDataTypes["jpg"] = "image/jpeg";
  webDataTypes["ico"] = "image/x-icon";
  webDataTypes["xml"] = "text/xml";
  webDataTypes["pdf"] = "application/pdf";
  webDataTypes["zip"] = "application/zip";

}

void sys_LoadConfig()
{
  File configFile;
  String currentLine;
  String currentSection = "";
  String key;
  String hashKey;
  String value;
  int eqIndex;
  const char* hashKeyPtr;
  if(SPIFFS.exists(CONFIG_PATH))
  {
#ifdef SERIAL_DEBUG
  Serial.print("SYS: Reading config from ");
  Serial.println(CONFIG_PATH);
#endif
    configFile = SPIFFS.open(CONFIG_PATH, "r");
    if(configFile)
    {
      do
      {
        currentLine = configFile.readStringUntil('\n');
        // Section testing
        currentLine.trim(); // remove spaces
        if(currentLine.startsWith("[") && currentLine.endsWith("]"))
        {
          // This is a section
          currentSection = currentLine.substring(1,currentLine.length()-1);
        }
        else
        {
          // This is a line
          if(currentSection != "")
          {
            eqIndex = currentLine.indexOf('=');
            if(eqIndex)
            {
              key = currentLine.substring(0,eqIndex);
              key.trim();
              value = currentLine.substring(eqIndex + 1);
              value.trim();
              if(value.startsWith("\"") && value.endsWith("\""))
              {
                value = value.substring(1,value.length()-1);
              }
              sysConfig["/" + currentSection + "/" + key] = value;
              Serial.print(currentSection);
              Serial.print(" - ");
              Serial.print(key);
              Serial.print(" - ");
              Serial.println(value);
            }
          }
        }
      } while(configFile.position() < configFile.size());
      configFile.close();
    }
    else
    {
#ifdef SERIAL_DEBUG
      Serial.println("SYS: File open failed");
#endif      
    }
  }
}


// This is the termostat itself
void temp_Process()
{
  int intCurrTemp;
  // read temerature
  temp_ReadSensor();
  // convert temperature to 0.1 deg C integer format
  intCurrTemp = (int)(temp_ValueFloat * 10);
  if(intCurrTemp < temp_Set - lower_th)
  {
    digitalWrite(13, 1);
//    Heat = true;
    webInsertionStrings["heatcolor"] = "red";
  }
  if(intCurrTemp > temp_Set + upper_th)
  {
    digitalWrite(13, 0);
//    Heat = false;
    webInsertionStrings["heatcolor"] = "green";
  }
  temp_IsProcess = false;  
}


void setup()
{
  ContentDebug = true;
#ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUD);
  delay(10);
  Serial.println();
#endif
  delay(10);
  spiffs_Init();
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  sys_LoadConfig();
  temp_Set = sysConfig["/termostat/settemp"].toInt();
  Serial.print("Preset Temp: ");
  Serial.println(String(temp_Set));
  init_TempSensor();
  wifi_Init_ClientMode(sysConfig["/wlan/ssid"], sysConfig["/wlan/password"]);
  web_Init();
  // webInsertionStrings["heatcolor"] = "blue"; // Heating algorithm not implemented yet, so this is a fake value
  Serial.println("Firmware web update b0010");
}

void loop()
{
  if(temp_IsProcess)
  {
    temp_Process();
    temp_IsProcess = false;
  }
  server.handleClient();
}
