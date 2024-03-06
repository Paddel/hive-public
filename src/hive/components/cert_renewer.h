#pragma once

class CHive;

class CCertRenewer
{
private:
    CHive *m_pHive;
    char *m_pCode;

public:
    CCertRenewer(CHive *pHive);
    ~CCertRenewer();

    void Activate(const char *pCode);
    void Deactivate();

    bool IsActive() const {return m_pCode != 0x0; }
    const char *GetCode() const { return m_pCode; }
};