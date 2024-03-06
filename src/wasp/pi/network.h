#pragma once

class CWasp;
class CRaspiWasp;

class CNetwork
{
private:
    CWasp *m_pWasp;
    CRaspiWasp *m_pRaspiWasp;
    long long m_LastCheck;
    bool m_LastCheckResult;

    bool IsDeviceFound();

public:
    CNetwork(CWasp *pWasp, CRaspiWasp *pRaspiWasp);

    bool Check();
    void Tick();
};