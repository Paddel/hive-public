#include <fstream>
#include <string>

#include <core/detect.h>
#include <core/helper/string_helper.hpp>
#include <core/wrapper.h>

#if defined(CONF_FAMILY_WINDOWS)
#include <windows.h>
#define PIPE_OPEN _popen
#define PIPE_CLOSE _pclose
#elif defined(CONF_FAMILY_UNIX)
#include <unistd.h>
#include <signal.h>
#define PIPE_OPEN popen
#define PIPE_CLOSE pclose
#endif

#include "pipe.h"

void CPipeRead::PipeThread(void *pData)
{
    CPipeData *pPipeData = (CPipeData *)pData;

    char aBuffer[1024*8];
    FILE *pPipe = PIPE_OPEN(pPipeData->m_pCommand, "r");
    bool Interrupted = false;
    if (pPipe != 0x0)
    {
        while (fgets(aBuffer, sizeof(aBuffer), pPipe) != NULL)
        {
            std::string Line = aBuffer;
            StringRemoveAll(Line, "\r");
            StringRemoveAll(Line, "\n");

            if(pPipeData->m_CBPipeOut != 0x0 && pPipeData->m_CBPipeOut(pPipeData->m_pThis, pPipeData->m_pUser, Line.c_str()) == true)
            {
                Interrupted = true;
                break;
            }
        }
    }

    if(pPipeData->m_CBPipeDone != 0x0)
        pPipeData->m_CBPipeDone(pPipeData->m_pThis, pPipeData->m_pUser, Interrupted);

    if(Interrupted == false && pPipe != 0x0)
        PIPE_CLOSE(pPipe);

    delete[] pPipeData->m_pCommand;
    delete pPipeData->m_pThis;
    delete pPipeData;
}

CPipeRead::CPipeRead(const char *pCommand, const void *pUser, PIPE_OUT CBPipeOut, PIPE_DONE CBPipeDone)
{
    CPipeData *pData = new CPipeData();
    pData->m_pThis = this;
    pData->m_pUser = pUser;
    pData->m_CBPipeOut = CBPipeOut;
    pData->m_CBPipeDone = CBPipeDone;
    int CmdExtSize = str_length(pCommand) + 1;
    pData->m_pCommand = new char[CmdExtSize];
    str_format(pData->m_pCommand, CmdExtSize, "%s", pCommand);

    thread_create(PipeThread, pData);
}

CPipeWrite::CPipeWrite(const char *pCommand)
{
    m_pPipe = (FILE *)PIPE_OPEN(pCommand, "w");
}

void CPipeWrite::Write(const char *pLine)
{
    if(m_pPipe != 0x0)
    {
        fprintf((FILE *)m_pPipe, "%s\n", pLine);
        fflush((FILE *)m_pPipe);
    }
}

void CPipeWrite::Close()
{
    if(m_pPipe != 0x0)
    {
        PIPE_CLOSE((FILE *)m_pPipe);
        m_pPipe = 0x0;
    }
}