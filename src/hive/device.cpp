#include <algorithm>

#include "device.h"

CDevice::CDevice()
{
    m_aAddress[0] = '\0';
    m_Port = 0;
    m_LastLiveSignal = 0;
    m_NumPortChanges = 0;
    m_IsPushing = false;
}

CPhone::CPhone()
{
    m_ReceivedBattery = 0;
    m_LastBatteryReceived = 0;
    m_BatteryDecay = 0.0f;
}

int CPhone::EstimatedPhoneBattery()
{
	float minPassed = (timer_get() - m_LastBatteryReceived) / (float)timer_freq() / 60.0f;
	int Delta = std::min(std::max(m_BatteryDecay * minPassed, -5.0f), 5.0f);
	return std::min(std::max(m_ReceivedBattery + Delta, 0), 100);
}

CPi::CPi()
{
    m_DeviceAtHome = false;
}

CPi::CPlug *CPi::GetPlug(const char *pName)
{
    for(int i = 0; i < m_lpPlugs.Size(); i++)
        if(str_comp(m_lpPlugs[i]->m_aName, pName) == 0)
            return m_lpPlugs[i];
    return 0x0;
}

void CPi::OverridePlugs(CArray<CPi::CPlug *> lpPlugs)
{
    m_lpPlugs.DeleteAll();
    m_lpPlugs = lpPlugs;
}