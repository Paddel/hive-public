#pragma once

#include <core/array.hpp>
#include <core/protocol.h>
#include <core/wrapper.h>

class CDevice
{
	friend class CHive;

private:
	char m_aAddress[MAX_ADDR_LEN];
	int m_Port;
	long long m_LastLiveSignal;
	
	//NAT-Pushing
	unsigned int m_NumPortChanges;
	bool m_IsPushing;

public:
	CDevice();
};

class CPhone : public CDevice
{
	friend class CHive;

private:
	int m_ReceivedBattery;
	long m_LastBatteryReceived;
	float m_BatteryDecay;

public:
	CPhone();

	int EstimatedPhoneBattery();

	float GetBatteryDecay() const { return m_BatteryDecay; }
};

class CPi : public CDevice
{
	friend class CHive;

public:
	struct CPlug
	{
		char m_aName[64];
		bool m_Status;
	};

private:
	bool m_DeviceAtHome;
	CArray<CPlug *> m_lpPlugs;

public:
	CPi();

	CPlug *GetPlug(const char *pName);
	void OverridePlugs(CArray<CPlug *> lpPlugs);

	bool GetDeviceAtHome() const { return m_DeviceAtHome; }
	CArray<CPlug *> *GetPlugs() { return &m_lpPlugs; }
};