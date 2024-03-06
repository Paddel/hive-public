#include <core/wrapper.h>

#include "cert_renewer.h"

CCertRenewer::CCertRenewer(CHive *pHive)
{
    m_pHive = pHive;

    m_pCode = 0x0;
}

CCertRenewer::~CCertRenewer()
{
    if(m_pCode != 0x0)
        delete[] m_pCode;
}

void CCertRenewer::Activate(const char *pCode)
{
    if(IsActive() == true)
        return;

    int Size = str_length(pCode) + 1;
    m_pCode = new char[Size];
    str_copy(m_pCode, pCode, Size);
}

void CCertRenewer::Deactivate()
{
    if(m_pCode != 0x0)
        delete[] m_pCode;
    m_pCode = 0x0;
}