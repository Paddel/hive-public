#pragma once

#include <map>
#include <string>

class CHome;

class CDevice
{
    typedef void(*ROUTINE)(CDevice *pThis, const void *pData);

private:
    long long m_lastUpdate;
    char m_aName[64];
    std::map<std::string, ROUTINE> m_aRoutines;

protected:
    CHome *m_pHome;
    class CStatistics *m_pStatistics;

    void Publish(const char *pPayload);
    void Publish(const char *pPayload, int PayloadSize);
    void AddRoutine(const char *pName, ROUTINE Routine);
    virtual void OnUpdate(const void *pPayload, int PayloadLen) { }
    virtual const char *GetPublishFormat() { return "%s"; }

public:
    CDevice(CHome *pHome, const char *pName);

    void OnMessage(const void *pPayload, int PayloadLen);
    void CallRoutine(const char *pName, const void *pData);

    virtual void Tick() { }
    virtual void FillMsg(CMsg *pMsg) { }

    const char *GetName() { return m_aName; }
    long long GetLastUpdate() { return m_lastUpdate; }
};

class CDeviceSensorPlant : public CDevice
{
private:
    int m_SoilMoisture;

public:
    CDeviceSensorPlant(CHome *pHome, const char *pName);

    void OnUpdate(const void *pPayload, int PayloadLen);
    void Tick();
    void FillMsg(CMsg *pMsg);

    int GetSoilMoisture() const { return m_SoilMoisture; }
};

class CDeviceSensorRoom : public CDevice
{
private:
    float m_Humidity;
	float m_Temperature;
	int m_VOC;

public:
    CDeviceSensorRoom(CHome *pHome, const char *pName);

    void OnUpdate(const void *pPayload, int PayloadLen);
    void Tick();
    void FillMsg(CMsg *pMsg);
    
    float GetHumidity() const { return m_Humidity; }
    float GetTemperatur() const { return m_Temperature; }
    int GetVOC() const { return m_VOC; }
};

class CDeviceQube : public CDevice
{
public:
    CDeviceQube(CHome *pHome, const char *pName) : CDevice(pHome, pName) {}
    
    void OnUpdate(const void *pPayload, int PayloadLen);
};

class CDeviceIris : public CDevice
{
private:
    bool m_State;
    char m_Brightness;
    int m_Temperature;
    char m_aColor[8];

    static void RoutineOn(CDevice *pThis, const void *pData);
    static void RoutineOff(CDevice *pThis, const void *pData);
    static void RoutineToggle(CDevice *pThis, const void *pData);
    static void RoutineColor(CDevice *pThis, const void *pData);
    static void RoutineBrightness(CDevice *pThis, const void *pData);
    static void RoutineTemperature(CDevice *pThis, const void *pData);
    static void RoutineEffect(CDevice *pThis, const void *pData);

public:
    CDeviceIris(CHome *pHome, const char *pName);
    
    void OnUpdate(const void *pPayload, int PayloadLen);
    void FillMsg(CMsg *pMsg);
    const char *GetPublishFormat() { return "zigbee2mqtt/%s/set"; }

    int GetBrightness() const { return m_Brightness; }
    int GetTemperature() const { return m_Temperature; }
};

class CDeviceBlaster : public CDevice
{
private:
    static void RoutineSend(CDevice *pThis, const void *pData);

public:
    CDeviceBlaster(CHome *pHome, const char *pName);
    
    const char *GetPublishFormat() { return "zigbee2mqtt/%s/set"; }
};