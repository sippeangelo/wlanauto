#include "NetworkAdapter.h"

NetworkAdapter::NetworkAdapter(GUID guid)
{
	this->Initialize(StringFromGUID(guid));
}

NetworkAdapter::NetworkAdapter(std::string guid)
{
	this->Initialize(guid);
}

void NetworkAdapter::Initialize(std::string gguid)
{
	this->guid = gguid;

	pIWbemLocator = NULL;
	pWbemServices = NULL;

	pEnumrator = NULL;
	pClassObject = NULL;

	HRESULT hRes;

 //   // Step 1: --------------------------------------------------
 //   // Initialize COM. ------------------------------------------
 //   hRes = CoInitializeEx(0, COINIT_MULTITHREADED); 
 //   if (FAILED(hRes))
 //   {
	//	std::cout << "Failed to initialize COM library. Error code = 0x" << std::hex << hRes << std::endl;
 //       return;
 //   }

 //   // Step 2: --------------------------------------------------
 //   // Set general COM security levels --------------------------
 //   // Note: If you are using Windows 2000, you need to specify -
 //   // the default authentication credentials for a user by using
 //   // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
 //   // parameter of CoInitializeSecurity ------------------------
 //   hRes = CoInitializeSecurity(
 //       NULL, 
 //       -1,                          // COM authentication
 //       NULL,                        // Authentication services
 //       NULL,                        // Reserved
 //       RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
 //       RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
 //       NULL,                        // Authentication info
 //       EOAC_NONE,                   // Additional capabilities 
 //       NULL                         // Reserved
 //       );	

	//if (FAILED(hRes))
	//{
	//	std::cout << "Failed to initialize security. Error code = 0x" << std::hex << hRes << std::endl;
	//	CoUninitialize();
	//	//return;
	//}

	hRes = CoCreateInstance(
	  CLSID_WbemAdministrativeLocator,
	  NULL,
	  CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, 
	  IID_IUnknown,
	  (void **)&pIWbemLocator
	  );

	if (SUCCEEDED(hRes))
	{
	  hRes = pIWbemLocator->ConnectServer(
		  L"root\\cimv2", // Namespace
		  NULL, // Userid
		  NULL, // PW
		  NULL, // Locale
		  0, // flags
		  NULL, // Authority
		  NULL, // Context
		  &pWbemServices
	  );
	}

	// Convert the GUID to a string
	//std::string sGUID = StringFromGUID(guid);

	//std::stringstream ss;
	//ss << "SELECT * FROM Win32_NetworkAdapter WHERE GUID='" << cGUID << "'";

	Query();
}

void NetworkAdapter::Query()
{
	if (pClassObject != NULL)
		pClassObject->Release();

	std::string sQuery = "SELECT * FROM Win32_NetworkAdapter WHERE GUID='" + guid + "'";
	// And back into a bloody bstr
	_bstr_t bsQuery(sQuery.c_str());

	// Do the query
	HRESULT hRes = pWbemServices->ExecQuery(L"WQL", bsQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumrator);

	if (FAILED(hRes))
	{
		std::cout << "Failed pWbemServices->ExecQuery. Error code = 0x" << std::hex << hRes << std::endl;
		return;
	}

	ULONG uReturned;
	hRes = pEnumrator->Reset();
	hRes = pEnumrator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturned);
	pEnumrator->Release();

	if (FAILED(hRes))
	{
		std::cout << "Failed pEnumObject->Next. Error code = 0x" << std::hex << hRes << std::endl;
		return;
	}
}

NetworkAdapter::~NetworkAdapter()
{
	if (pIWbemLocator != NULL)
		pIWbemLocator->Release();
	if (pWbemServices != NULL)
		pWbemServices->Release();
	if (pClassObject != NULL)
		pClassObject->Release();
}

void NetworkAdapter::Enable()
{
	ExecMethod("Enable");
}

void NetworkAdapter::Disable()
{
	ExecMethod("Disable");
}

std::string NetworkAdapter::GetProperty(std::string prop)
{
	_variant_t vProp;
	pClassObject->Get(ConvertStringToBSTR(prop.c_str()), 0, &vProp, NULL, NULL);

	if (vProp.vt & VT_BSTR)
	{
		char* chBuf = ConvertBSTRToString(vProp.bstrVal);
		std::string sProp(chBuf);
		delete[] chBuf;
		return sProp;
	}
	else
		return NULL;
}

_variant_t NetworkAdapter::GetVariantProperty(std::string prop)
{
	BSTR bsProp = ConvertStringToBSTR(prop.c_str());

	_variant_t vProp;
	pClassObject->Get(bsProp, 0, &vProp, NULL, NULL);

	return vProp;
}

void NetworkAdapter::ExecMethod(std::string method)
{
	HRESULT hRes;

	// Method
	_bstr_t bsMethodName(method.c_str());
	// Class
	std::string sClassName = "Win32_NetworkAdapter.DeviceID='" + GetProperty("DeviceID") + "'";
	_bstr_t bsClassName(sClassName.c_str());

	// Execute Method
	//IWbemClassObject* pOutParams = NULL;
	//hRes = pWbemServices->ExecMethod(bsClassName, bsMethodName, 0,
	//NULL, NULL, &pOutParams, NULL);
	hRes = pWbemServices->ExecMethod(bsClassName, bsMethodName, 0, NULL, NULL, NULL, NULL);

	if (FAILED(hRes))
	{
		std::cout << "Could not execute method. Error code = 0x" << std::hex << hRes << std::endl;
		return;
	}
}

std::string NetworkAdapter::StringFromGUID(GUID guid)
{
	wchar_t orig[MAX_PATH] = {0};
	StringFromGUID2(guid, orig, MAX_PATH);

	// Convert to a char*
	size_t origsize = wcslen(orig) + 1;
	char nstring[MAX_PATH];
	wcstombs_s(NULL, nstring, origsize, orig, _TRUNCATE);

	return std::string(nstring);
}