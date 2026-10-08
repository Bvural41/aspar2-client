#pragma once

#include "NetSocket.h"

class CNetDatagramReceiver
{
	public:
		CNetDatagramReceiver();
		virtual ~CNetDatagramReceiver();

		void Clear();
		BOOL Bind(DWORD dwAddress, WORD wPortIndex);
		BOOL isBind();

		BOOL Process();
		BOOL Recv(void * pBuffer, int iSize);
		BOOL Peek(void * pBuffer, int iSize);

		void SetRecvBufferSize(int recvBufSize);

	protected:
		BOOL m_isBind;

		DWORD m_dwPortIndex;

		socket_t m_Socket;
		SOCKADDR_IN m_SockAddr;

		int		m_recvBufCurrentPos;
		int		m_recvBufCurrentSize;

		char*	m_recvBuf;
		int		m_recvBufSize;
};