#include "NetworkAdapterFactory.h"

void QueryNetworkAdapters(std::string query, std::vector<NetworkAdapter*> &vAdapters)
{

	std::cout << "1: " << &vAdapters << std::endl;

	HRESULT hRes;
 //   // Step 1: --------------------------------------------------
 //   // Initialize COM. ------------------------------------------
 //   hRes = CoInitializeEx(0, COINIT_MULTITHREADED); 
 //   if (FAILED(hRes))
 //   {
	//	std::cout << "Failed to initialize COM library. Error code = 0x" << std::hex << hRes << std::endl;
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
	//}

	IWbemLocator* pIWbemLocator = NULL;
	IWbemServices* pWbemServices = NULL;
	BSTR bstrNamespace = (L"root\\cimv2");
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

	// Format query
	query = "SELECT * FROM Win32_NetworkAdapter " + query;
	BSTR bsQuery = ConvertStringToBSTR(query.c_str());

	IEnumWbemClassObject* pEnumrator = NULL;
	hRes = pWbemServices->ExecQuery(L"WQL", bsQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumrator);

	if (FAILED(hRes))
	{
		std::cout << "Failed pWbemServices->ExecQuery. Error code = 0x" << std::hex << hRes << std::endl;
		CoUninitialize();
	}

	IWbemClassObject* pClassObject[128] = {0};
	ULONG uReturned;
	hRes = pEnumrator->Reset();
	hRes = pEnumrator->Next(WBEM_INFINITE, 128, pClassObject, &uReturned);
	if (FAILED(hRes))
	{
		std::cout << "Failed pEnumObject->Next. Error code = 0x" << std::hex << hRes << std::endl;
		CoUninitialize();
	}	

	// Fill the vector
	for (int i = 0; i < uReturned; i++)
	{
		_variant_t guid;
		pClassObject[i]->Get(L"GUID", 0, &guid, NULL, NULL);

		//vNetworkAdapters.push_back(new NetworkAdapter(std::string(_com_util::ConvertBSTRToString(guid.bstrVal))));

		if (guid.vt == VT_BSTR)
		{
			//NetworkAdapter na(std::string(_com_util::ConvertBSTRToString(guid.bstrVal)));
			//vAdapters.push_back(new NetworkAdapter("{D15F65F4-27C5-4548-8705-B50B5B360737}"));
			char* chBuf = ConvertBSTRToString(guid.bstrVal);
			vAdapters.push_back(new NetworkAdapter(std::string(chBuf)));
			delete[] chBuf;
			//std::cout << _com_util::ConvertBSTRToString(guid.bstrVal) << std::endl;
		}

		pClassObject[i]->Release();
	}

	pEnumrator->Release();

	pIWbemLocator->Release();
	pWbemServices->Release();
}