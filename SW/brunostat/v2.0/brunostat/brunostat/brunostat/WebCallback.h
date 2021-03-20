#pragma once
#include "HashMap.h"
#include <ESP8266WebServer.h>
class WebCallback
{
public:
	virtual void OnWebCallDelegate() {}
};

class WebCallbackSingleton
{
public:
	static WebCallbackSingleton* Instance(ESP8266WebServer* srv);
	static WebCallbackSingleton* Instance(ESP8266WebServer* srv, WebCallback* wcb, String uri, HTTPMethod method);
	static WebCallbackSingleton* Instance(WebCallback* wcb, String uri, HTTPMethod method);
	static WebCallbackSingleton* Instance();
	static void StaticOnWebCallDelegate();
	CreateHashMap(callbackObjects, String, WebCallback*, 20);
	//HashMap<String, WebCallback*, 20> callbackObjects;
protected:
	WebCallbackSingleton();
private:
	static bool IsInstanceExists;
	static WebCallbackSingleton* _instance;
	ESP8266WebServer* _srv;
};

bool WebCallbackSingleton::IsInstanceExists = false;
WebCallbackSingleton* WebCallbackSingleton::_instance = NULL;
WebCallbackSingleton* WebCallbackSingleton::Instance()
{
	if (!IsInstanceExists)
	{
		// throw exception needed because the web server object didn't initialized
	}
	return _instance;
}
WebCallbackSingleton* WebCallbackSingleton::Instance(ESP8266WebServer* srv)
{
	if (!IsInstanceExists)
	{
		_instance = new WebCallbackSingleton();
		IsInstanceExists = true;
		_instance->_srv = srv;
	}
	return _instance;
}

WebCallbackSingleton* WebCallbackSingleton::Instance(ESP8266WebServer* srv, WebCallback* wcb, String uri, HTTPMethod method)
{
	if (!IsInstanceExists)
	{
		_instance = new WebCallbackSingleton();
		IsInstanceExists = true;
		_instance->_srv = srv;
	}
	_instance->callbackObjects[uri] = wcb;
	_instance->_srv->on(uri.c_str(), method, WebCallbackSingleton::StaticOnWebCallDelegate);
	return _instance;
}

WebCallbackSingleton* WebCallbackSingleton::Instance(WebCallback* wcb, String uri, HTTPMethod method)
{
	if (!IsInstanceExists)
	{
		// throw exception needed because the web server object didn't initialized
	}
	_instance->callbackObjects[uri] = wcb;
	_instance->_srv->on(uri.c_str(), method, WebCallbackSingleton::StaticOnWebCallDelegate);
	return _instance;
}

// The constructor
WebCallbackSingleton::WebCallbackSingleton()
{
	//callbackObjects = new HashMap<String, WebCallback*, 20>();
}

void WebCallbackSingleton::StaticOnWebCallDelegate()
{
	WebCallbackSingleton *wcbs;
	wcbs = WebCallbackSingleton::Instance();
	if (wcbs->callbackObjects.indexOf(wcbs->_srv->uri()) > -1)
	{
		wcbs->callbackObjects[wcbs->_srv->uri()]->OnWebCallDelegate();
	}
}