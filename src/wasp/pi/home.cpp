#include <core/protocol.h>
#include <core/wrapper.h>
#include <wasp/wasp.h>

#include "mqtt.h"

#include "home.h"

#define UPDATE_TIME 5

CHome::CHome(CWasp *pWasp, CStatistics *pStatistics)
{
	m_pWasp = pWasp;
	m_pStatistics = pStatistics;

	m_LastUpdate = 0;
	m_pPlugs += new CPlug("plug0", "lamp");
	m_pPlugs += new CPlug("plug1", "aquarium");
	m_pPlugs += new CPlug("plug2", "stereo");
	m_pPlugs += new CPlug("plug3", "charger");
	m_pPlugs += new CPlug("vplug0", "lamp");

	m_pDevices += new CDeviceQube(this, "0x00158d0002e99749");
	m_pDevices += new CDeviceSensorRoom(this, "0x54ef4410001a1fcf");
	m_pDevices += new CDeviceSensorPlant(this, "0x00124b001803fb24");
	m_pDevices += new CDeviceIris(this, "0x001788010cba45b6");
	m_pDevices += new CDeviceBlaster(this, "0x385b44fffe3311b5");

#ifndef CONF_DUMMY
	m_pMQTT = new CMQTT(this, OnMessage, OnConnect);
#endif
}

CHome::CPlug::CPlug(const char *pName, const char *pImage)
{
	str_copy(m_aName, pName, sizeof(m_aName));
	str_copy(m_aImage, pImage, sizeof(m_aImage));
	m_Status = false;
}

void CHome::Update()
{
	CMsg Msg = m_pWasp->NewMsg(MSG_INFO);
    Msg.AddInteger(INFO_HOME_STATUS);
    Msg.AddInteger(m_pPlugs.Size());
	for (int i = 0; i < m_pPlugs.Size(); i++)
	{
		CHome::CPlug *pPlug = m_pPlugs[i];
		Msg.AddByte(pPlug->GetStatus());
		Msg.AddString(pPlug->GetName());
	}
    m_pWasp->SendMsg(&Msg);

	m_LastUpdate = timer_get();
}

void CHome::SetStatPlug(CHome::CPlug *pPlug, bool Value)
{
	CMsg Msg(MSG_EVENT_HOME_STATUS, m_pWasp->WhoAmI());
	Msg.AddString(pPlug->GetName());
	Msg.AddByte(Value);
	m_pWasp->SendMsg(&Msg);

	Update();

	pPlug->SetStatus(Value);
}

void CHome::OnConnect(void *pUserData)
{
	CHome *pThis = (CHome *)pUserData;
	char aTopicComp[128];
#ifndef CONF_DUMMY
	for (int i = 0; i < pThis->m_pPlugs.Size(); i++)
	{
		str_format(aTopicComp, sizeof(aTopicComp), "stat/%s/POWER", pThis->m_pPlugs[i]->GetName());
		pThis->m_pMQTT->Subscribe(aTopicComp, 0);
	}

	pThis->m_pMQTT->Subscribe("zigbee2mqtt/#", 0);
#endif
}

void CHome::OnMessage(void *pUserData, const char *pTopic, const void *pPayload, int PayloadLen)
{
	CHome *pThis = (CHome *)pUserData;
	char aTopicComp[128];
	for (int i = 0; i < pThis->m_pPlugs.Size(); i++)
	{
		CHome::CPlug *pPlug = pThis->m_pPlugs[i];
		str_format(aTopicComp, sizeof(aTopicComp), "stat/%s/POWER", pPlug->GetName());
		if (str_comp(pTopic, aTopicComp) == 0)
		{
			if (mem_mem(pPayload, PayloadLen, "ON", 2) != 0)
				pThis->SetStatPlug(pPlug, true);
			else if (mem_mem(pPayload, PayloadLen, "OFF", 3) != 0)
				pThis->SetStatPlug(pPlug, false);
			return;
		}
	}

	if(str_comp_num(pTopic, "zigbee2mqtt/", 12) == 0)
	{
		const char *pDeviceName = pTopic + 12;
		CDevice *pDevice = pThis->FindDevice(pDeviceName);
		if(pDevice != 0x0)
			pDevice->OnMessage(pPayload, PayloadLen);
		return;
	}
	
	print("MQTT: Unknown Message topic=%s", pTopic);
}

void CHome::Init()
{
#ifndef CONF_DUMMY
	m_pMQTT->Init("E79k9tXFnzx5CqYJ", "YU4JgvYdExN59pRE");
#endif
}

void CHome::Tick()
{
	if(m_LastUpdate + timer_freq() * 60 * UPDATE_TIME < timer_get())
        Update();

	for(int i = 0; i < m_pDevices.Size(); i++)
		m_pDevices[i]->Tick();
}

void CHome::Publish(const char *pTopic, const char *pPayload)
{
	Publish(pTopic, (const void *)pPayload, str_length(pPayload));
}

void CHome::Publish(const char *pTopic, const void *pPayload, int PayloadSize)
{
#ifndef CONF_DUMMY
	m_pMQTT->Publish(pTopic, pPayload, PayloadSize);
#endif
}

void CHome::HomeMessageRoutine(const char *pName, const char *pRoutine, const void *pData)
{
	for(int i = 0; i < m_pDevices.Size(); i++)
	{
		CDevice *pDevice = m_pDevices[i];
		if(str_comp(pDevice->GetName(), pName) == 0)
		{
			pDevice->CallRoutine(pRoutine, pData);
			return;
		}
	}
}

void CHome::HomeMessagePower(CPlug *pPlug, const void *pPayload, int PayloadSize)
{
	char aTopicComp[128];
	str_format(aTopicComp, sizeof(aTopicComp), "cmnd/%s/POWER", pPlug->GetName());
	Publish(aTopicComp, pPayload, PayloadSize);
}

void CHome::HomeMessagePower(const char *pName, const void *pPayload, int PayloadSize)
{
	CPlug *pPlug = FindPlug(pName);
	if (pPlug == 0x0)
		return;

	HomeMessagePower(pPlug, pPayload, PayloadSize);
}

void CHome::SwitchPlugPower(const char *pName, bool Value)
{
	HomeMessagePower(pName, (Value == true ? "1" : "0"), 1);
}

void CHome::TogglePlugPower(const char *pName)
{
	CPlug *pPlug = FindPlug(pName);
	if (pPlug == 0x0)
		return;

	HomeMessagePower(pPlug, (pPlug->GetStatus() ? "0" : "1"), 1);
}

void CHome::ToggleBot(const char *pAddress)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "python3 extern/home/bot.py %s", pAddress);
	if (system(aBuf) != 0)
		print("Waking Phone failed!");
}

CHome::CPlug *CHome::FindPlug(const char *pName)
{
	for (int i = 0; i < m_pPlugs.Size(); i++)
		if (str_comp(m_pPlugs[i]->GetName(), pName) == 0)
			return m_pPlugs[i];
	return 0x0;
}

CDevice *CHome::FindDevice(const char *pName)
{
	for(int i = 0; i < m_pDevices.Size(); i++)
		if(str_comp(m_pDevices[i]->GetName(), pName) == 0)
			return m_pDevices[i];
	return 0x0;
}

void CHome::FillMsgStatus(CMsg *pMsg)
{
	for(int i = 0; i < m_pDevices.Size(); i++)
		m_pDevices[i]->FillMsg(pMsg);
}