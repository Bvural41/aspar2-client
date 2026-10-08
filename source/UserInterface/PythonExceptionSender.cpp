#include "StdAfx.h"
#include "PythonExceptionSender.h"

void CPythonExceptionSender::Send()
{
	if (m_strExceptionString.empty())
		return;

	DWORD dwCRC32 = GetCaseCRC32(m_strExceptionString.c_str(), m_strExceptionString.length());
	if (m_kSet_dwSendedExceptionCRC.end() != m_kSet_dwSendedExceptionCRC.find(dwCRC32))
		return;

	TraceError("%s", m_strExceptionString.c_str());
	m_kSet_dwSendedExceptionCRC.insert(dwCRC32);
}

CPythonExceptionSender::CPythonExceptionSender()
{
}
CPythonExceptionSender::~CPythonExceptionSender()
{
}
