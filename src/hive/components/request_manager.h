#pragma once

#include <core/array.hpp>
#include <core/wrapper.h>
#include <hive/webserver/webserver.h>
#include <hive/info_thread.h>

class CUnpacker;
class CTCPUnpacker;

class CRequest
{
public:
	enum
	{
		TYPE_RAW = 0,
		TYPE_UDP,
		TYPE_TCP,
		TYPE_REST,
	};

private:
	int m_Type;
	char m_Device;
	bool m_Error;
	char m_aErrorMessage[256];
	int m_RequestID;
	bool m_TimedOut;
	SEMAPHORE m_Signal;
	LOCK m_ErrorLock;

protected:
	int m_Timeout;

public:
	CRequest(int Type, int RequestID, char Device);
	virtual ~CRequest();

	void WaitForResponse();
	void OnError(const char *pErrorMessage);

	SEMAPHORE *Signal() { return &m_Signal; }
	char *ErrorMessage() { return m_aErrorMessage; }

	int GetType() const { return m_Type; }
	char GetDevice() const { return m_Device; }
	bool GetError() const { return m_Error; }
	int GetRequestID() const { return m_RequestID; }
	bool GetTimedOut() const { return m_TimedOut; }
};

class CRequestUDP : public CRequest
{
public:
	CRequestUDP(int RequestID, char Device);
	~CRequestUDP();
	CUnpacker *m_pUnpacker;
};

class CRequestTCP : public CRequest
{
public:
	CRequestTCP(int RequestID, char Device);
	~CRequestTCP();
	CTCPUnpacker *m_pUnpacker;
	int m_TCPSocket;
};

class CRequestREST : public CRequest
{
	SEMAPHORE m_ContentSignal;
public:
	CRequestREST(int RequestID);
	~CRequestREST();
	CWebserver::CRequestContent *m_pContent;
	CInfoTcpThread *m_pThreadInfo;
	
	void Done();
	void WaitForContent();
};

class CRequestManager
{
private:
	CArray<CRequest *> m_lpRequests;
	LOCK m_RequestLock;
	int m_RequstIDCounter;

	CRequest *CheckRequest(int RequestID, char Device, bool Error, CUnpacker *pUnpacker);

public:
	CRequestManager();

	CRequest *NewRequest(char Device);
	CRequestUDP *NewRequestUDP(char Device);
	CRequestTCP *NewRequestTCP(char Device);
	CRequestREST *NewRequestREST();
	
	void RespondRequestUDP(int RequestID, char Device, CUnpacker *pUnpacker, bool Error);
	void RespondRequestTCP(int RequestID, char Device, int Socket, CUnpacker *pUnpacker, bool Error);
	void RespondRequestREST(int RequestID, CInfoTcpThread *pThreadInfo, CWebserver::CRequestContent *pContent);
	

	CRequest *FindRequest(int RequestID);
	void RemoveRequest(CRequest *pRequest);
	void RemoveRequest(int Index);
};