#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>

#include <core/array.hpp>
#include <core/protocol.h>
//#include <core/packet.h>
#include <core/wrapper.h>
#include <wasp/wasp.h>

#include "network.h"
#include "raspiwasp.h"

#define CHECK_TIME 5
#define ADDRESS_NETWORK "192.168.188"


std::string Exec(const char* pCommand)
{
    std::array<char, 128> Buffer;
    std::string Result;

#ifndef CONF_DUMMY
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(pCommand, "r"), pclose);
    if (!pipe)
    {
        print("popen() failed!");
        return "";
    }
    while (fgets(Buffer.data(), Buffer.size(), pipe.get()) != nullptr) {
        Result += Buffer.data();
    }
#endif

    return Result;
}

CNetwork::CNetwork(CWasp *pWasp, CRaspiWasp *pRaspiWasp)
{
    m_pWasp = pWasp;
    m_pRaspiWasp = pRaspiWasp;
    m_LastCheck = 0;
    m_LastCheckResult = false;
}

bool CNetwork::IsDeviceFound()
{
    char aCommand[1024];
    //ping sweep
    CArray<std::string> lAddresses;
    str_format(aCommand, sizeof(aCommand), "for i in `seq 1 254`; do (ping -c 1 %s.\"$i\" | grep 'bytes from' &) ;done", ADDRESS_NETWORK);
    std::string ResultSweep = Exec(aCommand);
    std::stringstream StreamSweep(ResultSweep);
    const std::regex BaseRegexSweep("bytes from (.*):");
    for(std::string Line; std::getline(StreamSweep, Line, '\n'); )
    {
        std::smatch Match;
        if (std::regex_search(Line, Match, BaseRegexSweep) == true && Match.size() > 1)
            lAddresses += Match.str(1);
    }

    //arp table
    str_format(aCommand, sizeof(aCommand), "arp -a | grep '");
    int NumMacAddresses = sizeof(g_aMACAddress) / sizeof(*g_aMACAddress);
    for(int i = 0; i < NumMacAddresses; i++)
        str_fcat(aCommand, sizeof(aCommand), "%s%s", g_aMACAddress[i], (i != NumMacAddresses - 1 ? "\\|" : "'"));
    std::string ResultARP = Exec(aCommand);
    std::stringstream StreamARP(ResultARP);
    const std::regex BaseRegexARP("\\((.*)\\)");
    for(std::string Line; std::getline(StreamARP, Line, '\n'); )
    {
        std::smatch Match;
        if (std::regex_search(Line, Match, BaseRegexARP) == true && Match.size() > 1)
            for(int i = 0; i < lAddresses.Size(); i++)  
                if(lAddresses[i] == Match.str(1))
                    return true;
    }
    return false;
}

bool CNetwork::Check()
{
    bool Result = IsDeviceFound();
    //check twice to prevent false negative
    if(m_LastCheckResult == true && Result == false)
        for(int i = 0; i < 3 && Result == false; i++)
            Result = IsDeviceFound();

    CMsg Msg = m_pWasp->NewMsg(MSG_INFO);
    Msg.AddInteger(INFO_DEVICE_AT_HOME);
    Msg.AddByte(Result);
    m_pWasp->SendMsg(&Msg);

    m_pRaspiWasp->OnHomeState(Result);

    m_LastCheck = timer_get();
    m_LastCheckResult = Result;
    return Result;
}

void CNetwork::Tick()
{
    if(m_LastCheck + timer_freq() * 60 * CHECK_TIME < timer_get())
        Check();
}