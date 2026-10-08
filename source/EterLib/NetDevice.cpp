#include "StdAfx.h"
#include "NetDevice.h"

CNetworkDevice::CNetworkDevice()
{
	Initialize();
}

CNetworkDevice::~CNetworkDevice()
{
	Destroy();	
}

void CNetworkDevice::Initialize()
{
	m_isWSA=false;
}

void CNetworkDevice::Destroy()
{
#ifdef _WIN32
	if (m_isWSA)
	{
		WSACleanup();
		m_isWSA = false;
	}
#else
	m_isWSA = false;
#endif
}

bool CNetworkDevice::Create()
{
	Destroy();

	Initialize();

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
		return false;
#endif

	m_isWSA = true;
	
	return true;
}