
#include <core/packet.h>
#include <core/protocol.h>
#include <core/tcp_unpacker.h>

#include "request_manager.h"

#define TIMEOUT 30

CRequest::CRequest(int Type, int RequestID, char Device)
{
	m_Type = Type;
	m_RequestID = RequestID;
	m_Device = Device;
	m_Error = false;
	m_aErrorMessage[0] = '\0';
	semaphore_init(&m_Signal);
	m_Timeout = TIMEOUT;
	m_TimedOut = false;
	m_ErrorLock = lock_create();
}
CRequest::~CRequest()
{
	semaphore_destroy(&m_Signal);
	lock_destroy(m_ErrorLock);
}

void CRequest::WaitForResponse()
{
	if (semaphore_wait(&m_Signal, m_Timeout) == false)
	{
		m_TimedOut = true;
		OnError("Request timed out");
	}
}

void CRequest::OnError(const char *pErrorMessage)
{
	lock_wait(m_ErrorLock);
	if (m_Error == false)
	{
		m_Error = true;
		str_copy(m_aErrorMessage, pErrorMessage, sizeof(m_aErrorMessage));
		print("Request error: %s", m_aErrorMessage);
	}
	lock_unlock(m_ErrorLock);
}

CRequestUDP::CRequestUDP(int RequestID, char Device)
	: CRequest(CRequest::TYPE_UDP, RequestID, Device)
{
	m_pUnpacker = 0x0;
}

CRequestUDP::~CRequestUDP()
{
	if (m_pUnpacker != 0x0)
		delete m_pUnpacker;
}

CRequestTCP::CRequestTCP(int RequestID, char Device)
	: CRequest(CRequest::TYPE_TCP, RequestID, Device)
{
	m_pUnpacker = 0x0;
}

CRequestTCP::~CRequestTCP()
{
	if (m_pUnpacker != 0x0)
		delete m_pUnpacker;
}

CRequestREST::CRequestREST(int RequestID)
	: CRequest(CRequest::TYPE_REST, RequestID, DEVICE_UNDEFINED)
{
	m_pContent = 0x0;
	semaphore_init(&m_ContentSignal);
	m_Timeout = 60 * 5;
}

CRequestREST::~CRequestREST()
{
	semaphore_destroy(&m_ContentSignal);
}

void CRequestREST::Done()
{
	semaphore_signal(&m_ContentSignal);
}

void CRequestREST::WaitForContent()
{
	if (semaphore_wait(&m_ContentSignal, m_Timeout) == false)
		OnError("Request timed out");
}

CRequestManager::CRequestManager()
{
	m_RequstIDCounter = 0;
	m_RequestLock = lock_create();
}

CRequest *CRequestManager::CheckRequest(int RequestID, char Device, bool Error, CUnpacker *pUnpacker)
{
	CRequest *pRequest = FindRequest(RequestID);
	if (pRequest == 0x0)
		return 0x0;

	if (pRequest->GetDevice() != Device)
	{
		pRequest->OnError("Wrong Device");
		semaphore_signal(pRequest->Signal());
		return 0x0;
	}

	if (Error)
	{
		const char *pErrorMsg = pUnpacker->GetString();
		if(pUnpacker->GetError() == false)
			pRequest->OnError(pErrorMsg);
		else
			pRequest->OnError("Unknown Device Error");

		semaphore_signal(pRequest->Signal());
		return 0x0;
	}

	return pRequest;
}

CRequest *CRequestManager::NewRequest(char Device)
{
	lock_wait(m_RequestLock);
	CRequest *pNewRequest = new CRequest(CRequest::TYPE_RAW, m_RequstIDCounter++, Device);
	m_lpRequests += pNewRequest;
	lock_unlock(m_RequestLock);
	return pNewRequest;
}

CRequestUDP *CRequestManager::NewRequestUDP(char Device)
{
	lock_wait(m_RequestLock);
	CRequestUDP *pNewRequest = new CRequestUDP(m_RequstIDCounter++, Device);
	m_lpRequests += pNewRequest;
	lock_unlock(m_RequestLock);
	return pNewRequest;
}

CRequestTCP *CRequestManager::NewRequestTCP(char Device)
{
	lock_wait(m_RequestLock);
	CRequestTCP *pNewRequest = new CRequestTCP(m_RequstIDCounter++, Device);
	m_lpRequests += pNewRequest;
	lock_unlock(m_RequestLock);
	return pNewRequest;
}

CRequestREST *CRequestManager::NewRequestREST()
{
	lock_wait(m_RequestLock);
	CRequestREST *pNewRequest = new CRequestREST(m_RequstIDCounter++);
	m_lpRequests += pNewRequest;
	lock_unlock(m_RequestLock);
	return pNewRequest;
}

void CRequestManager::RespondRequestUDP(int RequestID, char Device, CUnpacker *pUnpacker, bool Error)
{
	CRequestUDP *pRequestUDP = (CRequestUDP *)CheckRequest(RequestID, Device, Error, pUnpacker);
	if (pRequestUDP == 0x0)
		return;

	int RestSize = pUnpacker->GetRestSize();
	pRequestUDP->m_pUnpacker = new CUnpacker(pUnpacker->GetRest(), RestSize, false);
	semaphore_signal(pRequestUDP->Signal());
}

void CRequestManager::RespondRequestTCP(int RequestID, char Device, int Socket, CUnpacker *pUnpacker, bool Error)
{
	CRequestTCP *pRequestTCP = (CRequestTCP *)CheckRequest(RequestID, Device, Error, pUnpacker);
	if (pRequestTCP == 0x0)
		return;

	int RestSize = pUnpacker->GetRestSize();
	pRequestTCP->m_pUnpacker = new CTCPUnpacker(*(CTCPUnpacker *)pUnpacker);
	pRequestTCP->m_TCPSocket = Socket;
	semaphore_signal(pRequestTCP->Signal());
}

void CRequestManager::RespondRequestREST(int RequestID, CInfoTcpThread *pThreadInfo, CWebserver::CRequestContent *pContent)
{
	CRequestREST *pRequestREST = (CRequestREST *)FindRequest(RequestID);
	if (pRequestREST == 0x0)
	{
		print("REST-Request not found %i", RequestID);
		return;
	}

	pRequestREST->m_pContent = pContent;
	pRequestREST->m_pThreadInfo = pThreadInfo;
	semaphore_signal(pRequestREST->Signal());
	pRequestREST->WaitForContent();
}

CRequest *CRequestManager::FindRequest(int RequestID)
{
	CRequest *pRequest = 0x0;
	lock_wait(m_RequestLock);
	for (int i = 0; i < m_lpRequests.Size(); i++)
		if (m_lpRequests[i]->GetRequestID() == RequestID)
			pRequest = m_lpRequests[i];
	lock_unlock(m_RequestLock);
	return pRequest;
}

void CRequestManager::RemoveRequest(CRequest *pRequest)
{
	lock_wait(m_RequestLock);
	m_lpRequests.Delete(pRequest);
	lock_unlock(m_RequestLock);
}

void CRequestManager::RemoveRequest(int Index)
{
	lock_wait(m_RequestLock);
	m_lpRequests.Delete(Index);
	lock_unlock(m_RequestLock);
}