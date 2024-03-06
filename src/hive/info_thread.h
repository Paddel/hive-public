#pragma once

#include <core/wrapper.h>

struct CInfoBaseThread
{
	void *m_pThis;
	void *m_pThread;
	bool m_Done;
	int m_Type;

	enum
	{
		THREAD_TCP=0,
		THREAD_UDP,
	};

	CInfoBaseThread(int Type, void *pThis)
	{
		m_Type = Type;
		m_pThis = pThis;
		m_pThread = 0x0;
		m_Done = false;
	}
};

struct CInfoTcpThread : public CInfoBaseThread
{	
	char m_aAddress[256];
	int m_Port;
	int m_Socket;
	void *m_pSSL;
	bool m_WantSSL;
	long long m_TimeLastUpdate;

	CInfoTcpThread(void *pThis, char *pAddress, int Port, int Socket, bool WantSSL = false)
		: CInfoBaseThread(THREAD_TCP, pThis)
	{
		str_copy(m_aAddress, pAddress, sizeof(m_aAddress));
		m_Port = Port;
		m_Socket = Socket;
		m_WantSSL = WantSSL;

		m_pSSL = 0x0;
		m_TimeLastUpdate = timer_get();
	}
};

struct CInfoUdpThread : public CInfoBaseThread
{
	class CUnpacker *m_pUnpacker;
	char m_MsgID;
	char m_Device;
	
	CInfoUdpThread(void *pThis, class CUnpacker *pUnpacker, char MsgID, char Device)
		: CInfoBaseThread(THREAD_UDP, pThis)
	{
		m_pUnpacker = pUnpacker;
		m_MsgID = MsgID;
		m_Device = Device;
	}
};