#include <stdlib.h>

#include <core/algorithm.hpp>
#include <core/json.hpp>
#include <core/protocol.h>
#include <core/wrapper.h>
#include <wasp/pi/home.h>
#include <wasp/pi/statistics.h>

#include "device.h"

using json = nlohmann::json;

CDevice::CDevice(CHome *pHome, const char *pTopic)
{
    m_pHome = pHome;
    str_copy(m_aName, pTopic, sizeof(m_aName));

    m_lastUpdate = 0;
    m_pStatistics = pHome->m_pStatistics;
}

void CDevice::Publish(const char *pPayload)
{
    char aBuf[256];
    str_format(aBuf, sizeof(aBuf), GetPublishFormat(), m_aName);
    m_pHome->Publish(aBuf, pPayload);
}

void CDevice::Publish(const char *pPayload, int PayloadSize)
{
    char aBuf[256];
    str_format(aBuf, sizeof(aBuf), GetPublishFormat(), m_aName);
    m_pHome->Publish(aBuf, pPayload, PayloadSize);
}

void CDevice::AddRoutine(const char *pName, ROUTINE Routine)
{
    m_aRoutines[pName] = Routine;
}

void CDevice::CallRoutine(const char *pName, const void *pData)
{
    auto Routine = m_aRoutines.find(pName);
    if(Routine != m_aRoutines.end())
        Routine->second(this, pData);
}

void CDevice::OnMessage(const void *pPayload, int PayloadLen)
{
    OnUpdate(pPayload, PayloadLen);
    m_lastUpdate = timer_get();
}

CDeviceSensorPlant::CDeviceSensorPlant(CHome *pHome, const char *pTopic) : CDevice(pHome, pTopic)
{
    m_SoilMoisture = -1;
}

void CDeviceSensorPlant::OnUpdate(const void *pPayload, int PayloadLen)
{
    const char *pJson = (const char *)pPayload;
    if(json::accept(pJson) == false)
        return;
    json Payload = json::parse(pJson);
    m_SoilMoisture = Payload.value("soil_moisture", -1.0f);

    m_pStatistics->AddValue("soil_moisture", m_SoilMoisture);
}

void CDeviceSensorPlant::Tick()
{
    if(timer_get() - GetLastUpdate() > timer_freq() * 60 * 60 * 2)
        m_SoilMoisture = -1;
}

void CDeviceSensorPlant::FillMsg(CMsg *pMsg)
{
    pMsg->AddInteger(m_SoilMoisture);
}

CDeviceSensorRoom::CDeviceSensorRoom(CHome *pHome, const char *pTopic) : CDevice(pHome, pTopic)
{
    m_Humidity = -1;
    m_Temperature = -1;
    m_VOC = -1;
}

void CDeviceSensorRoom::OnUpdate(const void *pPayload, int PayloadLen)
{
    const char *pJson = (const char *)pPayload;
    if(json::accept(pJson) == false)
        return;
    json Payload = json::parse(pJson);
    
    m_Temperature = Payload.value("temperature", -1.0f);
    m_Humidity = Payload.value("humidity", -1.0f);
    m_VOC = Payload.value("voc", -1);

    m_pStatistics->AddValue("temperature", m_Temperature);
    m_pStatistics->AddValue("humidity", m_Humidity);
    m_pStatistics->AddValue("voc", m_VOC);
}

void CDeviceSensorRoom::Tick()
{
    if(timer_get() - GetLastUpdate() > timer_freq() * 60 * 30)
    {
        m_Humidity = -1;
        m_Temperature = -1;
        m_VOC = -1;
    }
}

void CDeviceSensorRoom::FillMsg(CMsg *pMsg)
{
    pMsg->AddInteger(m_Temperature * 100);
    pMsg->AddInteger(m_Humidity * 100);
    pMsg->AddInteger(m_VOC);
}

void CDeviceQube::OnUpdate(const void *pPayload, int PayloadLen)
{
    
    static const char *s_pInfoFilter = "\"level\": \"info\"";
    static int s_InfoFilterLen = str_length(s_pInfoFilter);
    if(mem_mem(pPayload, PayloadLen, s_pInfoFilter, s_InfoFilterLen) != 0)
        return;

    const char *pJson = (const char *)pPayload;
    if(json::accept(pJson) == false)
        return;
    json Payload = json::parse(pJson);

    std::string Action = Payload.value("action", "");
    int Side = Payload.value("side", -1);

    if(Action == "rotate_left" || Action == "rotate_right")
    {
        int Angle = Payload.value("action_angle", 0);
        if(Side == 0)
        {
            CDeviceIris *pIris = (CDeviceIris *)m_pHome->FindDevice("0x001788010cba45b6");
            const int NewTemperature = clamp((int)(pIris->GetTemperature() - Angle / 2), 153, 500);
            m_pHome->HomeMessageRoutine("0x001788010cba45b6", "temperature", &NewTemperature);
        }
        else if(Side == 1)
        {
            m_pHome->HomeMessageRoutine("0x385b44fffe3311b5", "send", g_sCodeNames[Angle > 0 ? CODE_CHAIN_DIM_UP : CODE_CHAIN_DIM_DOWN]);
        }
        else if(Side == 2)
        {
            CDeviceIris *pIris = (CDeviceIris *)m_pHome->FindDevice("0x001788010cba45b6");
            const int NewBrightness = clamp((int)(pIris->GetBrightness() + Angle / 2), 0, 254);
            m_pHome->HomeMessageRoutine("0x001788010cba45b6", "brightness", &NewBrightness);
        }
        else if(Side == 4)
        {
            if(Action == "rotate_left")
            {
                system("pkill -9 vlc");
                m_pHome->SwitchPlugPower("plug2", 0);
            }
            else
            {
                system("pkill -9 vlc");
                // system("bash /home/pi/sounds/ambiente/start.sh &");
                system("cvlc /home/forest_cosy.m4a --gain=8 &");
                m_pHome->SwitchPlugPower("plug2", 1);
            }
        }
        else if(Side == 5)
        {
            m_pHome->HomeMessageRoutine("0x385b44fffe3311b5", "send", g_sCodeNames[Angle < 0 ? CODE_RECEIVER_VOLUME_UP : CODE_RECEIVER_VOLUME_DOWN]);
        }
    }

    /*static const char *s_pFlip90Filter = "\"action\":\"flip90\"";
    static int s_pFlip90FilterLen = str_length(s_pFlip90Filter);
    if(mem_mem(pPayload, PayloadLen, s_pFlip90Filter, s_pFlip90FilterLen) != 0)
    {
        system("pkill -9 vlc");
        // system("bash /home/pi/sounds/ambiente/start.sh &");
        system("cvlc /home/forest_cosy.m4a --gain=8 &");
        m_pHome->SwitchPlugPower("plug2", 1);
    }*/

    /*static const char *s_pFallFilter = "\"action\":\"flip180\"";
    static int s_pFallFilterLen = str_length(s_pFallFilter);
    if(mem_mem(pPayload, PayloadLen, s_pFallFilter, s_pFallFilterLen) != 0)
    {
        system("pkill -9 vlc");
        m_pHome->SwitchPlugPower("plug2", 0);
    }*/

    static const char *s_pShakeFilter = "\"action\":\"shake\"";
    static int s_pShakeFilterLen = str_length(s_pShakeFilter);
    if(mem_mem(pPayload, PayloadLen, s_pShakeFilter, s_pShakeFilterLen) != 0)
    {
        CHome::CPlug *pPlugLamp = m_pHome->FindPlug("plug0");
        CHome::CPlug *pPlugTower = m_pHome->FindPlug("plug1");
        if(pPlugLamp->GetStatus() == true || pPlugTower->GetStatus() == true)
        {
            m_pHome->SwitchPlugPower("plug0", 0);
            m_pHome->SwitchPlugPower("plug1", 0);

            m_pHome->HomeMessageRoutine("0x001788010cba45b6", "off", 0x0);
            m_pHome->HomeMessageRoutine("0x385b44fffe3311b5", "send", g_sCodeNames[CODE_CHAIN_OFF]);
        }
        else
        {
            m_pHome->SwitchPlugPower("plug0", 1);
            m_pHome->SwitchPlugPower("plug1", 1);

            m_pHome->HomeMessageRoutine("0x001788010cba45b6", "on", 0x0);
            m_pHome->HomeMessageRoutine("0x385b44fffe3311b5", "send", g_sCodeNames[CODE_CHAIN_ON]);
        }
    }

    /*static const char *s_pSlideFilter = "\"action\":\"slide\"";
    static int s_pSlideFilterLen = str_length(s_pSlideFilter);
    if(mem_mem(pPayload, PayloadLen, s_pSlideFilter, s_pSlideFilterLen) != 0)
    {
        
    }*/
}

CDeviceIris::CDeviceIris(CHome *pHome, const char *pTopic) : CDevice(pHome, pTopic)
{
    m_State = false;
    m_Brightness = 0;
    m_Temperature = 0;
    str_copy(m_aColor, "#ffffff", sizeof(m_aColor));

    AddRoutine("on", RoutineOn);
    AddRoutine("off", RoutineOff);
    AddRoutine("toggle", RoutineToggle);
    AddRoutine("color", RoutineColor);
    AddRoutine("brightness", RoutineBrightness);
    AddRoutine("temperature", RoutineTemperature);
    AddRoutine("effect", RoutineEffect);
}

void CDeviceIris::RoutineOn(CDevice *pThis, const void *pData)
{
    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish("{\"state\": \"ON\"}");

    pIris->m_State = !pIris->m_State;
}

void CDeviceIris::RoutineOff(CDevice *pThis, const void *pData)
{
    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish("{\"state\": \"OFF\"}");

    pIris->m_State = !pIris->m_State;
}

void CDeviceIris::RoutineToggle(CDevice *pThis, const void *pData)
{
    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish("{\"state\": \"TOGGLE\"}");

    pIris->m_State = !pIris->m_State;
}

void CDeviceIris::RoutineColor(CDevice *pThis, const void *pData)
{
    char aPayload[64];
    const char *pColor = (const char *)pData;
    str_format(aPayload, sizeof(aPayload), "{\"color\":{\"hex\":\"%s\"}, \"transition\":1}", pColor);

    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish(aPayload);

    str_copy(pIris->m_aColor, pColor, sizeof(pIris->m_aColor));
}

void CDeviceIris::RoutineBrightness(CDevice *pThis, const void *pData)
{
    char aPayload[64];
    int Level = *(int *)pData;
    str_format(aPayload, sizeof(aPayload), "{\"brightness\":%i, \"transition\":1}", Level);

    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish(aPayload);

    pIris->m_Brightness = Level;
}

void CDeviceIris::RoutineTemperature(CDevice *pThis, const void *pData)
{
    char aPayload[64];
    int Level = *(int *)pData;
    str_format(aPayload, sizeof(aPayload), "{\"color_temp\":%i, \"transition\":1}", Level);

    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish(aPayload);

    pIris->m_Temperature = Level;
}

void CDeviceIris::RoutineEffect(CDevice *pThis, const void *pData)
{
    char aPayload[64];
    const char *pName = (const char *)pData;
    str_format(aPayload, sizeof(aPayload), "{\"effect\":%s}", pName);

    CDeviceIris *pIris = (CDeviceIris *)pThis;
    pIris->Publish(aPayload);
}

void CDeviceIris::OnUpdate(const void *pPayload, int PayloadLen)
{
    const char *pJson = (const char *)pPayload;
    if(json::accept(pJson) == false)
        return;
        
    json Payload = json::parse(pJson);
    m_Brightness = Payload.value("brightness", -1);
    m_Temperature = Payload.value("color_temp", -1);
    m_State = Payload.value("state", "OFF") == "ON";
}

void CDeviceIris::FillMsg(CMsg *pMsg)
{
    pMsg->AddByte(m_State);
    pMsg->AddInteger(m_Brightness);
    pMsg->AddString(m_aColor);
}


CDeviceBlaster::CDeviceBlaster(CHome *pHome, const char *pTopic) : CDevice(pHome, pTopic)
{
    AddRoutine("send", RoutineSend);
}

void CDeviceBlaster::RoutineSend(CDevice *pThis, const void *pData)
{
    CDeviceBlaster *pIris = (CDeviceBlaster *)pThis;
    const char *pCodeName = (const char *)pData;

    const char *pCode = 0x0;
    const int NumCodes = sizeof(g_sCodes) / sizeof(g_sCodes[0]);
    for(int i = 0; i < NumCodes; i++)
    {
        if(str_comp(g_sCodeNames[i], pCodeName) == 0)
        {
            pCode = g_sCodes[i];
            break;
        }
    }

    if(pCode == 0x0)
    {
        print("Code not found: %s", pCodeName);
        return;
    }

    char aPayload[512];
    str_format(aPayload, sizeof(aPayload), "{\"ir_code_to_send\": \"%s\"}", pCode);
    pIris->Publish(aPayload);
}