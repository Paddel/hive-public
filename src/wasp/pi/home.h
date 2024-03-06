#pragma once

#include <core/array.hpp>

#include <wasp/pi/device.h>

class CWasp;

class CHome
{
	friend class CDevice;

public:
	class CPlug
	{
	private:
		char m_aName[64];
		char m_aImage[64];
		bool m_Status;

	public:
		CPlug(const char *pName, const char *pImage);

		void SetStatus(bool Status) { m_Status = Status; }

		bool GetStatus() const { return m_Status; }
		const char *GetName() { return m_aName; }
		const char *GetImage() { return m_aImage; }
	};

private:
	long long m_LastUpdate;
	CWasp *m_pWasp;
	class CMQTT *m_pMQTT;
	class CStatistics *m_pStatistics;
	CArray <CPlug *> m_pPlugs;
	CArray <CDevice *> m_pDevices;
	
	void Update();
	void SetStatPlug(CHome::CPlug *pPlug, bool Value);

	static void OnConnect(void *pUserData);
	static void OnMessage(void *pUserData, const char *pTopic, const void *pPayload, int PayloadLen);

public:
	CHome(CWasp *pWasp, CStatistics *pStatistics);

	void Init();
	void Tick();
	void Publish(const char *pTopic, const char *pPayload);
	void Publish(const char *pTopic, const void *pPayload, int PayloadSize);
	void HomeMessageRoutine(const char *pName, const char *pRoutine, const void *pData);
	void HomeMessagePower(CPlug *pPlug, const void *pPayload, int PayloadSize);
	void HomeMessagePower(const char *pName, const void *pPayload, int PayloadSize);
	void SwitchPlugPower(const char *pName, bool Value);
	void TogglePlugPower(const char *pName);
	void ToggleBot(const char *pAddress);
	CPlug *FindPlug(const char *pName);
	CDevice *FindDevice(const char *pName);
	void FillMsgStatus(CMsg *pMsg);

	int NumPlugs() const { return m_pPlugs.Size(); }
	CPlug *GetPlug(int Index) { return m_pPlugs[Index]; }
};