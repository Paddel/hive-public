#pragma once

#include <fstream>

class CPipeRead
{
    typedef bool(*PIPE_OUT)(CPipeRead *pPipe, const void *pUser, const char *pLine);
    typedef void(*PIPE_DONE)(CPipeRead *pPipe, const void *pUser, bool Interrupted);
    
    struct CPipeData
    {
        CPipeRead *m_pThis;
        char *m_pCommand;
        const void *m_pUser;
        PIPE_OUT m_CBPipeOut;
        PIPE_DONE m_CBPipeDone;
    };

private:
    static void PipeThread(void *pData);

public:
    CPipeRead(const char *pCommand, const void *pUser = 0x0, PIPE_OUT CBPipeOut = 0x0, PIPE_DONE CBPipeDone = 0x0);
};

class CPipeWrite
{
private:
    FILE *m_pPipe;

public:
    CPipeWrite(const char *pCommand);

    void Write(const char *pLine);
    void Close();
};