#pragma once

#include <core/json.hpp>

class CSettings
{
public:
	enum
	{
		SETTINGS_AUTOMISATION=0,
		SETTINGS_ROUTES,
		NUM_SETTINGS,
	};

private:
	nlohmann::json m_aData[CSettings::NUM_SETTINGS];

public:
	void Init();

	void LoadSettings(int Settings);
	const char *GetName(int Settings);
	std::string GetValue(int Settings, const char *pKey, const char *pDefault);
	bool GetValue(int Settings, const char *pKey, bool Default);
	int GetValue(int Settings, const char *pKey, int Default);
};