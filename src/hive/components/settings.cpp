#include <fstream>

#include <core/wrapper.h>

#include "settings.h"

using json = nlohmann::json;

static const char *s_aSettingsFiles[CSettings::NUM_SETTINGS] {
    "automisation",
    "routes",
};

void CSettings::Init()
{
    for(int i = 0; i < NUM_SETTINGS; i++)
        LoadSettings(i);
}

void CSettings::LoadSettings(int Settings)
{
    if(Settings < 0 || Settings >= CSettings::NUM_SETTINGS)
        return;
    char aBuf[256];
    str_format(aBuf, sizeof(aBuf), "settings/%s.json", s_aSettingsFiles[Settings]);
    std::ifstream file(aBuf);
    if(file.is_open() == false)
    {
        print("Settings: failed to load file %s.json", s_aSettingsFiles[Settings]);
        return;
    }

    file >> m_aData[Settings];
    file.close();
}

const char *CSettings::GetName(int Settings)
{
    return s_aSettingsFiles[Settings];
}

std::string CSettings::GetValue(int Settings, const char *pKey, const char *pDefault)
{
    if(Settings < 0 || Settings >= CSettings::NUM_SETTINGS || m_aData[Settings].empty() == true)
        return pDefault;
    return m_aData[Settings].value(pKey, pDefault);
}

bool CSettings::GetValue(int Settings, const char *pKey, bool Default)
{
    if(Settings < 0 || Settings >= CSettings::NUM_SETTINGS || m_aData[Settings].empty() == true)
        return Default;
    return m_aData[Settings].value(pKey, Default);
}

int CSettings::GetValue(int Settings, const char *pKey, int Default)
{
    if(Settings < 0 || Settings >= CSettings::NUM_SETTINGS || m_aData[Settings].empty() == true)
        return Default;
    return m_aData[Settings].value(pKey, Default);
}