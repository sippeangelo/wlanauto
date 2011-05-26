#include <Windows.h>
#include <WinSvc.h>
#include <stdio.h>
#include <wlanapi.h>
#pragma comment(lib, "wlanapi.lib")
#include <iostream>
#include <string>
#include "log.h"
#include "NetworkAdapterFactory.h"
#include "NetworkAdapter.h"

// Visual Leak Detector
//#include <vld.h>

std::string StringFromGUID(GUID guid)
{
	wchar_t orig[MAX_PATH] = {0};
	StringFromGUID2(guid, orig, MAX_PATH);

	// Convert to a char*
	size_t origsize = wcslen(orig) + 1;
	char nstring[MAX_PATH];
	wcstombs_s(NULL, nstring, origsize, orig, _TRUNCATE);

	return std::string(nstring);
}

void InitializeCom()
{
	HRESULT hRes;

    hRes = CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (FAILED(hRes))
    {
		LOGFILE << "Failed to initialize COM library. Error code = 0x" << std::hex << hRes << std::endl;
        return;
    }

    hRes = CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );	

	if (FAILED(hRes))
	{
		std::cout << "Failed to initialize security. Error code = 0x" << std::hex << hRes << std::endl;
		CoUninitialize();
		return;
	}
}

SERVICE_STATUS m_ServiceStatus;
SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
bool RUNNING;

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI ServiceCtrlHandler(DWORD Opcode);
bool InstallService();
bool UninstallService();

int main(int argc, char* argv[])
{
	//LOGFILE.open("C:\\wlanauto4_log.txt");

	if (argc > 1)
	{
		if (strcmp(argv[1], "-i") == 0)
		{
			if (InstallService())
				printf("Service installed sucessfully\n");
			else
				printf("Error installing service\n");
		}

		if (strcmp(argv[1], "-u") == 0)
		{
			if (UninstallService())
				printf("Service uninstalled sucessfully\n");
			else
				printf("Error uninstalling service\n");
		}
	}
	else
	{
		// Start the service
		SERVICE_TABLE_ENTRY Table[] = {{"WlanAuto", ServiceMain}, {NULL, NULL}};
		StartServiceCtrlDispatcher(Table);
	}

	//LOGFILE.close();

	return 0;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	m_ServiceStatus.dwServiceType = SERVICE_WIN32;
	m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_ServiceStatus.dwWin32ExitCode = 0;
	m_ServiceStatus.dwServiceSpecificExitCode = 0;
	m_ServiceStatus.dwCheckPoint = 0;
	m_ServiceStatus.dwWaitHint = 0;

	m_ServiceStatusHandle = RegisterServiceCtrlHandler("WlanAuto", ServiceCtrlHandler);
	if (m_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
		return;

	/*
		1.	Find the GUID of all Wireless adapters
		2.	Loop until we find another connected network adapter that 
			doesn't equal to any of the Wireless ones.
		3.	Watch the connection of this adapter. 
			If it's on, disable wireless.
			If it's off, enable wireless.
	*/
	InitializeCom();

	/*
	DWORD error = ERROR_SUCCESS;
    HANDLE hClient = NULL;
    DWORD dwVersion = NULL;
    error = WlanOpenHandle(2, NULL, &dwVersion, &hClient);
	if (error != ERROR_SUCCESS)
	{	
		LOGFILE << "WlanOpenHandle error: " << error << std::endl;
		return;
	}

	// Step 1
	PWLAN_INTERFACE_INFO_LIST pInterfaces = NULL;
	error = WlanEnumInterfaces(hClient, NULL, &pInterfaces);
	if (error != ERROR_SUCCESS)
	{	
		LOGFILE << "WlanEnumInterfaces error: " << error << std::endl;
		return;
	}
	if (pInterfaces->dwNumberOfItems <= 0)
	{
		LOGFILE << "No wireless interfaces!" << std::endl;
		return;
	}

	NetworkAdapter* WirelessAdapter = NULL;
	WLAN_INTERFACE_INFO info = pInterfaces->InterfaceInfo[0];
	std::cout << info.InterfaceGuid.Data1 << " (" << info.isState << ")" << std::endl;
	std::cout << StringFromGUID(info.InterfaceGuid) << std::endl;
	WirelessAdapter = new NetworkAdapter(info.InterfaceGuid);
	std::cout << WirelessAdapter->GetProperty("Description") << std::endl;
	WlanFreeMemory(pInterfaces);

	// Step 2
	NetworkAdapter* WiredAdapter = NULL;
	std::string WirelessGUID = WirelessAdapter->GetProperty("GUID");
	while (WiredAdapter == NULL)
	{
		std::vector<NetworkAdapter*> adapters;
		QueryNetworkAdapters("WHERE NetConnectionStatus = 2", adapters); // Only connected adapters

		std::vector<NetworkAdapter*>::iterator it;
		for (it = adapters.begin(); it < adapters.end(); it++)
		{
			if (WirelessGUID != (*it)->GetProperty("GUID"))
			{
				WiredAdapter = (*it);
				break;
			}
		}

		// Clean up
		for (it = adapters.begin(); it < adapters.end(); it++)
		{
			if ((*it) != WiredAdapter)
				delete (*it);
		}
	}*/

	// Network adapters
	NetworkAdapter* LAN_Adapter = NULL;
	NetworkAdapter* WIFI_Adapter = NULL;
	
	std::vector<NetworkAdapter*> adapters;

	// Find LAN adapter
	QueryNetworkAdapters("WHERE NetConnectionID = 'Anslutning till lokalt nätverk' OR NetConnectionID = 'Local Area Connection'", adapters);
	if (adapters.size() > 0)
		LAN_Adapter = adapters.at(0);
	else
	{
		//LOGFILE << "No LAN adapter found!" << std::endl;
		ServiceCtrlHandler(SERVICE_CONTROL_STOP);
		return;
	}
	adapters.clear();

	// Find WIFI adapter
	QueryNetworkAdapters("WHERE NetConnectionID = 'Trådlös Nätverksanslutning' OR NetConnectionID = 'Wireless Network Connection'", adapters);
	if (adapters.size() > 0)
		WIFI_Adapter = adapters.at(0);
	else
	{
		std::cout << "No WIFI adapter found!" << std::endl;
		ServiceCtrlHandler(SERVICE_CONTROL_STOP);
		return;
	}
	adapters.clear();

	// Start the service
	m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	m_ServiceStatus.dwCheckPoint = 0;
	m_ServiceStatus.dwWaitHint = 0;
	if (!SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus))
	{
		return;
	}

	//LOGFILE << "LAN: " << LAN_Adapter->GetProperty("Description") << std::endl;
	//LOGFILE << "WIFI: " << WIFI_Adapter->GetProperty("Description") << std::endl;

	RUNNING = true;
	while (RUNNING)
	{
		LAN_Adapter->Query();
		WIFI_Adapter->Query();

		int WirelessConnectionStatus = WIFI_Adapter->GetVariantProperty("NetConnectionStatus").uintVal;
		bool WirelessEnabled = WIFI_Adapter->GetVariantProperty("NetEnabled").boolVal;
		int WiredConnectionStatus = LAN_Adapter->GetVariantProperty("NetConnectionStatus").uintVal;

		if (WiredConnectionStatus == 2 && WirelessEnabled)
		{
			std::cout << "Disabled wireless" << std::endl;
			WIFI_Adapter->Disable();
		}
		else if (WiredConnectionStatus != 2 && !WirelessEnabled)
		{
			std::cout << "Enabled wireless" << std::endl;
			WIFI_Adapter->Enable();
		}

		//LOGFILE << "Wireless connection:		" << WirelessConnectionStatus << std::endl;
		//LOGFILE << "Wireless enabled:		" << WirelessEnabled << std::endl;
		//LOGFILE << "Wired connection:		" << WiredConnectionStatus << std::endl;

		Sleep(3000);
	}

	delete LAN_Adapter;
	delete WIFI_Adapter;

	CoUninitialize();
}

void WINAPI ServiceCtrlHandler(DWORD Opcode)
{
  switch(Opcode)
  {
    case SERVICE_CONTROL_PAUSE: 
      m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
      break;
    case SERVICE_CONTROL_CONTINUE:
      m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
      break;
    case SERVICE_CONTROL_STOP:
      m_ServiceStatus.dwWin32ExitCode = 0;
      m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
      m_ServiceStatus.dwCheckPoint = 0;
      m_ServiceStatus.dwWaitHint = 0;

      SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
      RUNNING = false;
      break;
    case SERVICE_CONTROL_INTERROGATE:
      break; 
  }
}

bool InstallService()
{
	char strDir[1024];
	SC_HANDLE schSCManager, schService;
	GetCurrentDirectory(1024, strDir);
	strcat(strDir,"\\wlanauto4.exe"); 
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (schSCManager == NULL) 
	return false;
	LPCTSTR lpszBinaryPathName = strDir;

	schService = CreateService(schSCManager, "WlanAuto", 
		"WlanAuto", // service name to display
		SERVICE_ALL_ACCESS, // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_AUTO_START, // start type 
		SERVICE_ERROR_NORMAL, // error control type 
		lpszBinaryPathName, // service's binary 
		NULL, // no load ordering group 
		NULL, // no tag identifier 
		NULL, // no dependencies
		NULL, // LocalSystem account
		NULL); // no password

	if (schService == NULL)
	return false; 

	CloseServiceHandle(schService);
	return true;
}

bool UninstallService()
{
	SC_HANDLE schSCManager, hService;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (schSCManager == NULL)
		return false;
	hService = OpenService(schSCManager, "ServiceTest", SERVICE_ALL_ACCESS);
	if (hService == NULL)
		return false;
	if(DeleteService(hService) == 0)
		return false;
	if(CloseServiceHandle(hService) == 0)
		return false;

	return true;
}

