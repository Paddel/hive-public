#pragma once

#include <core/protocol.h>

enum
{
	ACTION_HOUSE=0,
	ACTION_EXPLORER,
	ACTION_REMOTE,
	ACTION_CONSOLE,
	ACTION_ALARM,
	ACTION_SECCAM,
	ACTION_LOCATE,
	ACTION_CLIPBOARD,
	ACTION_LOCK,
	ACTION_SHUTDOWN,
	NUM_ACTIONS,
};

static const char *g_apActionNames[NUM_ACTIONS] = {
	"house",
	"explorer",
	"remote",
	"console",
	"alarm",
	"seccam",
	"locate",
	"clipboard",
	"lock",
	"shutdown",
};

static bool ActionAvailable(char Device, int Action)
{
	if (Device == DEVICE_LAPTOP)
	{
		switch (Action)
		{
		case ACTION_LOCK: return true;
		case ACTION_SHUTDOWN: return true;
		case ACTION_EXPLORER: return true;
		case ACTION_CLIPBOARD: return true;
		case ACTION_REMOTE: return true;
		case ACTION_CONSOLE: return true;
		}
	}
	else if (Device == DEVICE_PHONE)
	{
		switch (Action)
		{
		case ACTION_LOCK: return true;
		case ACTION_ALARM: return true;
		case ACTION_LOCATE: return true;
		case ACTION_EXPLORER: return true;
		case ACTION_CLIPBOARD: return true;
		case ACTION_SECCAM: return true;
		}
	}
	else if (Device == DEVICE_TOWER)
	{
		switch (Action)
		{
		case ACTION_LOCK: return true;
		case ACTION_SHUTDOWN: return true;
		case ACTION_EXPLORER: return true;
		case ACTION_CLIPBOARD: return true;
		case ACTION_REMOTE: return true;
		case ACTION_CONSOLE: return true;
		}
	}
	else if (Device == DEVICE_PI)
	{
		switch (Action)
		{
		case ACTION_HOUSE: return true;
		case ACTION_EXPLORER: return true;
		case ACTION_CONSOLE: return true;
		}
	}
	else if (Device == DEVICE_CLOUD)
	{
		switch (Action)
		{
		case ACTION_EXPLORER: return true;
		case ACTION_CONSOLE: return true;
		}
	}
	else if (Device == DEVICE_MALE)
	{
		switch (Action)
		{
		case ACTION_EXPLORER: return true;
		case ACTION_CLIPBOARD: return true;
		case ACTION_REMOTE: return true;
		case ACTION_CONSOLE: return true;
		}
	}

	return false;
}