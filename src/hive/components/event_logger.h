#pragma once

#include <core/array.hpp>

class CEventLogger
{
public:
    enum
    {
        EVENT_ALERT=0,
        EVENT_DEVICE_CONNECTED,
        EVENT_AUTH_FAIL,
        EVENT_LOGIN,
        EVENT_ELEVATE,
        EVENT_HOME,
        NUM_EVENTS,
    };

    struct CEvent
    {
        long long m_Time;
    };

    struct CEventAlert : public CEvent
    {
        char m_aType[64];
        char m_aContent[512];
    };

    struct CEventDeviceConnected : public CEvent
    {
        char m_aDeviceName[64];
        char m_aAddress[46];
    };

    struct CEventAuthFail : public CEvent
    {
        char m_aAddress[46];
        char m_aUserAgent[512];
        char *m_pAuth;
    };

    struct CEventLogin : public CEvent
    {
        char m_aAddress[46];
        char m_aUserAgent[512];
    };

    struct CEventElevate : public CEvent
    {
        char m_aAddress[46];
        char m_aUserAgent[512];
        int m_Permissions;
    };
    
    struct CEventHome : public CEvent
    {
        bool m_Home;
    };

private:
    CArray<CEvent *>m_lpEventBuffer[NUM_EVENTS];
    long long m_BootTime;
    bool m_Attention;

    bool AddEventEntry(int Event, CEvent *pEvent);

public:
    CEventLogger();
    ~CEventLogger();

    void AddEntryAlert(const char *pType, const char *pFormat, ...);
    void AddEntryConnect(const char *pDeviceName, const char *pAddress);
    void AddEntryAuthFail(const char *pAddress, const char *pUserAgent, const char *pAuth);
    void AddEntryLogin(const char *pAddress, const char *pUserAgent);
    void AddEntryElevate(const char *pAddress, const char *pUserAgent, int PermissionFlags);
    void AddEntryHome(bool Home);
    void Pack(class CSitePacker& Site, int Event);
    CArray<CEvent *> *GetEvents(int Event);
    void DisableAttention() { m_Attention = false; }
    bool GetAttention() const { return m_Attention; }
    const char *GetEventName(int Event);
    const char *GetEventIcon(int Event);
    long long GetBootTime() const { return m_BootTime; }
};