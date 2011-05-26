#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H

#include <windows.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#include <comutil.h>
#pragma comment(lib, "comsuppw.lib")
#include <string>
#include <sstream>
#include <iostream>
#include "bstr.h"

class NetworkAdapter
{
public:
	NetworkAdapter(GUID guid);
	NetworkAdapter(std::string guid);
	void Initialize(std::string guid);
	~NetworkAdapter();

	void Enable();
	void Disable();

	void Query();

	std::string GetProperty(std::string prop);
	_variant_t GetVariantProperty(std::string prop);
	void ExecMethod(std::string method);

private:
	std::string guid;

	IWbemLocator* pIWbemLocator;
	IWbemServices* pWbemServices;

	IEnumWbemClassObject* pEnumrator;
	IWbemClassObject* pClassObject;

	std::string StringFromGUID(GUID guid);
};

#endif