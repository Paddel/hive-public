
#include <core/md5/md5.h>
#include <hive/webserver/session.h>

#include "lock_manager.h"

#define ID_WHITELIST "WHITELIST"
#define ID_GLOBAL "GLOBAL"

CLockManager::CLockCount *CLockManager::FindLockCount(const char *pID)
{
	for (int i = 0; i < m_pLockCount.Size(); i++)
		if (str_comp(m_pLockCount[i]->m_aID, pID) == 0)
			return m_pLockCount[i];
	return 0x0;
}

CLockManager::CLockCount *CLockManager::FindCreateLockCount(const char *pID)
{
	CLockCount *pLock = FindLockCount(pID);
	if (pLock == 0x0)
	{
		pLock = new CLockCount();
		str_copy(pLock->m_aID, pID, sizeof(pLock->m_aID));
		pLock->m_Tries = 0;
		m_pLockCount += pLock;
	}

	return pLock;
}

void CLockManager::IncreaseLock(const char *pID)
{
	CLockCount *pLock = FindCreateLockCount(pID);
	pLock->m_Tries++;
	pLock->m_LastIncrease = timer_get();
}

bool CLockManager::IsLocked(const char *pID, int Type)
{
	CLockCount *pLock = FindLockCount(pID);
	if (pLock == 0x0)
		return false;
	return pLock->m_Tries >= MaxTries(Type);
}

inline int CLockManager::MaxTries(int Type)
{
	switch (Type)
	{
	case TYPE_GLOBAL: return 25;
	case TYPE_WHITELIST: return 20;
	case TYPE_ADDRESS: return 15;
	case TYPE_SESSION: return 10;
	}
	return 0;
}

void CLockManager::CheckLockCountTimeout()
{
	for (int i = 0; i < m_pLockCount.Size(); i++)
	{
		if (m_pLockCount[i]->m_LastIncrease + timer_freq() * 60 * 60 * 10 < timer_get())
		{
			m_pLockCount.Delete(i);
			i--;
		}
	}
}

void CLockManager::OnWrongAuthCode(CSession *pSession)
{
	char aClearText[1024];
	str_format(aClearText, sizeof(aClearText), "%s;%s", pSession->GetAddress(), pSession->GetUserAgent());
	
	IncreaseLock(md5(aClearText).c_str());
	IncreaseLock(md5(pSession->GetAddress()).c_str());
	IncreaseLock(ID_WHITELIST);
	IncreaseLock(ID_GLOBAL);
}

bool CLockManager::IsLocked(CSession *pSession)
{
	char aClearText[1024];
	str_format(aClearText, sizeof(aClearText), "%s;%s", pSession->GetAddress(), pSession->GetUserAgent());

	CheckLockCountTimeout();

	if (IsLocked(md5(aClearText).c_str(), TYPE_SESSION) == true)
		return true;
	if (IsLocked(md5(pSession->GetAddress()).c_str(), TYPE_ADDRESS) == true)
		return true;
	if (IsLocked(ID_WHITELIST, TYPE_WHITELIST) == true && pSession->IsMobile() == false)
		return true;
	if (IsLocked(ID_GLOBAL, TYPE_GLOBAL) == true)
		return true;
	return false;
}

bool CLockManager::IsGloballyLocked()
{
	return false;
}