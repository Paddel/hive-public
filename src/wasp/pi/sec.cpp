#include <stdlib.h>

#include <core/pipe.h>
#include <core/protocol.h>
#include <wasp/wasp.h>

#include "sec.h"

CSec::CSec(CWasp *pWasp)
{
    m_pWasp = pWasp;

    m_pPipe = 0x0;
    m_PID = -1;
}

bool CSec::OnPipePrint(CPipeRead *pPipe, const void *pUser, const char *pLine)
{
    CSec *pThis = (CSec *)pUser;
    if(pThis->m_PID == -1)
    {
        int PipePID = str_toint(pLine);
        if(PipePID <= 0)
            return true;
        pThis->m_PID = PipePID;
        print("SEC: PID=%i", PipePID);
    }
    else if(str_comp(pLine, ".\n") == 0)
    {//motion detected
        CMsg Msg(MSG_EVENT_SEC_MOTION, pThis->m_pWasp->WhoAmI(), MSG_FLAG_MECHANICAL);
        pThis->m_pWasp->SendMsg(&Msg);
        print("Motion Detected");
    }

    return false;
}

void CSec::Stop()
{
    if(m_PID == -1)
        return;

    print("SEC: Stopped");
    char aCmd[32];
    str_format(aCmd, sizeof(aCmd), "kill -9 %i > /dev/null", m_PID);
    system(aCmd);
    m_PID = -1;
    m_pPipe = 0x0;
}

void CSec::OnPipeDone(CPipeRead *pPipe, const void *pUser, bool Interrupted)
{
    ((CSec *) pUser)->Stop();
}

void CSec::Start()
{
    if(m_pPipe != 0x0)
        return;
    
    m_PID = -1;
    m_pPipe = new CPipeRead("python3 extern/home/motion.py", this, OnPipePrint, OnPipeDone);
    print("SEC: Started");
}

void CSec::OnHomeState(bool State)
{
    State == true ? Stop() : Start();
}