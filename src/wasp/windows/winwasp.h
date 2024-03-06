#pragma once

#include <wasp/extension.h>
#include <wasp/windows/atlas.h>
#include "scheduler.h"

class CWinWasp : public IExtension
{
private:
	CScheduler m_Scheduler;
	class CScraptcher *m_pScraptcher;
	CAtlas *m_pAtlas;

public:
	CWinWasp(CWasp *pWasp);

	virtual void Init();
	virtual bool ParsePacket(int MsgID, CUnpacker& Unpacker);
	virtual void Tick();
};