#include <time.h>
#include <stdarg.h>

#include <core/helper/string_helper.hpp>
#include <core/wrapper.h>
#include <hive/webserver/sitepacker.h>

#include "event_logger.h"

static const char *g_aEventName[CEventLogger::NUM_EVENTS] = {
    "Alerts",
    "Device Boots",
    "Failed Auths",
    "Logins",
    "Elevates",
    "Home",
};

static const char *g_aEventIcon[CEventLogger::NUM_EVENTS] = {
    "la-bell",
    "la-power-off",
    "la-times-circle",
    "la-sign-in-alt",
    "la-chevron-circle-up",
    "la-home",
};

CEventLogger::CEventLogger()
{
    m_BootTime = time(0);
    m_Attention = false;
}

CEventLogger::~CEventLogger()
{
    for(int i = 0; i < NUM_EVENTS; i++)
        m_lpEventBuffer[i].DeleteAll();
}

bool CEventLogger::AddEventEntry(int Event, CEvent *pEvent)
{
    if(Event < 0 || Event >= NUM_EVENTS)
        return false;

    pEvent->m_Time = time(0);
    m_lpEventBuffer[Event] += pEvent;
    return true;
}

void CEventLogger::AddEntryAlert(const char *pType, const char *pFormat, ...)
{
    char aContent[512];
	va_list Arguments;
	va_start(Arguments, pFormat);
	str_format_list(aContent, sizeof(aContent), pFormat, &Arguments);
	va_end(Arguments);

    CEventAlert *pEvent = new CEventAlert();
    str_copy(pEvent->m_aType, pType, sizeof(pEvent->m_aType));
    str_copy(pEvent->m_aContent, pType, sizeof(pEvent->m_aContent));
    AddEventEntry(EVENT_ALERT, pEvent);
}

void CEventLogger::AddEntryConnect(const char *pDeviceName, const char *pAddress)
{
    CEventDeviceConnected *pEvent = new CEventDeviceConnected();
    str_copy(pEvent->m_aDeviceName, pDeviceName, sizeof(pEvent->m_aDeviceName));
    str_copy(pEvent->m_aAddress, pAddress, sizeof(pEvent->m_aAddress));
    AddEventEntry(EVENT_DEVICE_CONNECTED, pEvent);
}

void CEventLogger::AddEntryAuthFail(const char *pAddress, const char *pUserAgent, const char *pAuth)
{
    CEventAuthFail *pEvent = new CEventAuthFail();
    str_copy(pEvent->m_aAddress, pAddress, sizeof(pEvent->m_aAddress));
    str_copy(pEvent->m_aUserAgent, pUserAgent, sizeof(pEvent->m_aUserAgent));
    int SizeAuth = str_length(pAuth) + 1;
    pEvent->m_pAuth = new char[SizeAuth];
    str_copy(pEvent->m_pAuth, pAuth, SizeAuth);
    AddEventEntry(EVENT_AUTH_FAIL, pEvent);
}

void CEventLogger::AddEntryLogin(const char *pAddress, const char *pUserAgent)
{
    CEventLogin *pEvent = new CEventLogin();
    str_copy(pEvent->m_aAddress, pAddress, sizeof(pEvent->m_aAddress));
    str_copy(pEvent->m_aUserAgent, pUserAgent, sizeof(pEvent->m_aUserAgent));
    AddEventEntry(EVENT_LOGIN, pEvent);
}

void CEventLogger::AddEntryElevate(const char *pAddress, const char *pUserAgent, int PermissionFlags)
{
    CEventElevate *pEvent = new CEventElevate();
    str_copy(pEvent->m_aAddress, pAddress, sizeof(pEvent->m_aAddress));
    str_copy(pEvent->m_aUserAgent, pUserAgent, sizeof(pEvent->m_aUserAgent));
    pEvent->m_Permissions = PermissionFlags;
    AddEventEntry(EVENT_ELEVATE, pEvent);
}

void CEventLogger::AddEntryHome(bool Home)
{
    CEventHome *pEvent = new CEventHome();
    pEvent->m_Home = Home;
    AddEventEntry(EVENT_HOME, pEvent);
}

void CEventLogger::Pack(class CSitePacker& Site, int Event)
{
    CArray<CEventLogger::CEvent *> *pEvents = GetEvents(Event);
	for(int i = pEvents->Size() -1; i >= 0; i--)
	{
		CEventLogger::CEvent *pEvent = pEvents->Get(i);

		char aTimeBuf[80];
		struct tm *pTimeStamp = localtime((const time_t*) &pEvent->m_Time);
		strftime(aTimeBuf, sizeof(aTimeBuf), "[%d.%m %H:%M]", pTimeStamp);

		Site << "<p>#" << i << " " << aTimeBuf << ": ";
		switch(Event)
		{
			case CEventLogger::EVENT_ALERT:
			{
				CEventLogger::CEventAlert *pEventAlert = (CEventLogger::CEventAlert *)pEvent;
				Site << pEventAlert->m_aType << " " << pEventAlert->m_aContent;
				break;
			}
			case CEventLogger::EVENT_DEVICE_CONNECTED:
			{
				CEventLogger::CEventDeviceConnected *pEventAuthFail = (CEventLogger::CEventDeviceConnected *)pEvent;
				Site << pEventAuthFail->m_aAddress << " " << pEventAuthFail->m_aDeviceName;
				break;
			}
			case CEventLogger::EVENT_AUTH_FAIL:
			{
				CEventLogger::CEventAuthFail *pEventAuthFail = (CEventLogger::CEventAuthFail *)pEvent;
				Site << pEventAuthFail->m_aAddress << " " << EscapeXssChars(pEventAuthFail->m_aUserAgent) << " " << EscapeXssChars(pEventAuthFail->m_pAuth);
				break;
			}
			case CEventLogger::EVENT_LOGIN:
			{
				CEventLogger::CEventLogin *pEventLogin = (CEventLogger::CEventLogin *)pEvent;
				Site << pEventLogin->m_aAddress << " " << EscapeXssChars(pEventLogin->m_aUserAgent);
				break;
			}
			case CEventLogger::EVENT_ELEVATE:
			{
				CEventLogger::CEventElevate *pEventElevate = (CEventLogger::CEventElevate *)pEvent;
				Site << pEventElevate->m_aAddress << " " << EscapeXssChars(pEventElevate->m_aUserAgent) << " " << pEventElevate->m_Permissions;
				break;
			}
			case CEventLogger::EVENT_HOME:
			{
				CEventLogger::CEventHome *pEventHome = (CEventLogger::CEventHome *)pEvent;
				Site << pEventHome->m_Home;
				break;
			}
		}
		Site << "</p>" << Site();
	}
}

CArray<CEventLogger::CEvent *> *CEventLogger::GetEvents(int Event)
{
    if(Event < 0 || Event >= NUM_EVENTS)
        return 0x0;
    return &m_lpEventBuffer[Event];
}

const char *CEventLogger::GetEventName(int Event)
{
    return g_aEventName[Event];
}

const char *CEventLogger::GetEventIcon(int Event)
{
    return g_aEventIcon[Event];
}