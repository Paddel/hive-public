#pragma once

#include <core/array.hpp>

class CLockManager
{
private:
	enum
	{
		TYPE_GLOBAL=0,
		TYPE_WHITELIST,
		TYPE_ADDRESS,
		TYPE_SESSION,
	};

	struct CLockCount
	{
		char m_aID[33];
		int m_Tries;
		long long m_LastIncrease;
	};
	
private:
	CArray<CLockCount *> m_pLockCount;

	CLockCount *FindLockCount(const char *pID);
	CLockCount *FindCreateLockCount(const char *pID);
	void IncreaseLock(const char *pID);
	bool IsLocked(const char *pID, int Type);
	inline int MaxTries(int Type);
	void CheckLockCountTimeout();

public:
	void OnWrongAuthCode(class CSession *pSession);
	bool IsLocked(class CSession *pSession);
	bool IsGloballyLocked();

};