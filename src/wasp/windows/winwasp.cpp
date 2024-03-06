#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include <windows.h>

#include <core/protocol.h>
#include <wasp/wasp.h>
#include "scraptcher.h"
#include "shortcuts.h"

#include "winwasp.h"

CWinWasp::CWinWasp(CWasp *pWasp) : IExtension(pWasp)
{
	m_pScraptcher = new CScraptcher(pWasp);
	m_pAtlas = new CAtlas(pWasp);
}

void CWinWasp::Init()
{
#ifndef CONF_DUMMY
	if (m_Scheduler.Init() == false)
		exit(0);
#endif

	CShortCuts::ShortCuts().Init();
	m_pScraptcher->Init();
}

bool CWinWasp::ParsePacket(int MsgID, CUnpacker& Unpacker)
{
	return true;
}

void CWinWasp::Tick()
{
	m_pAtlas->Tick();
}