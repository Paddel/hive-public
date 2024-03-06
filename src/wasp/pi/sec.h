#pragma once

class CWasp;

class CSec
{
private:
    CWasp *m_pWasp;
    class CPipeRead *m_pPipe;
    int m_PID;

    void Start();
    void Stop();

    static bool OnPipePrint(class CPipeRead *pPipe, const void *pUser, const char *pLine);
    static void OnPipeDone(CPipeRead *pPipe, const void *pUser, bool Interrupted);

public:
    CSec(CWasp *pWasp);

    void OnHomeState(bool State);
};