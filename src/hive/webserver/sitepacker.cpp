#include <stdarg.h>

#include <core/helper/string_helper.hpp>
#include <core/wrapper.h>
#include <hive/info_thread.h>

#include "webserver.h"

#include "sitepacker.h"

CSitePacker::CSitePacker(const char *pStatus, const char *pContentType)
{
	m_IsScriptSite = (pContentType != 0x0 && str_comp(pContentType, "text/javascript"));
	AddHeaderField(pStatus);
	if (pContentType != 0x0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Content-Type: %s", pContentType);
		AddHeaderField(aBuf);
	}
}

CSitePacker::~CSitePacker()
{
	for (int i = 0; i < NUM_BLOCKS; i++)
		m_lpBlocks[i].DeleteAll();
}

void CSitePacker::ExecuteAddContentText(void *pData, char *pText)
{
	((CSitePacker *)pData)->AddContent(pText);
}

void CSitePacker::ExecuteTextLength(void *pData, char *pText)
{
	long *pCounter = (long *)pData;
	*pCounter += str_length(pText);
}

void CSitePacker::ExecuteSend(void *pData, char *pText)
{
	CInfoTcpThread *pThreadInfo = (CInfoTcpThread *)pData;
	CWebserver *pWebServer = (CWebserver *)pThreadInfo->m_pThis;

	pWebServer->SendWeb(pThreadInfo, pText, str_length(pText), false);
}

void CSitePacker::ExecutePrint(void *pData, char *pText)
{
	print_raw(pText);
}

void CSitePacker::AddHeaderField(const char *pField)
{
	int BufSize = str_length(pField) + 3;
	char *pBuf = new char[BufSize];
	str_format(pBuf, BufSize, "%s\r\n", pField);
	m_lpBlocks[BLOCK_HEADERFIELDS] += pBuf;
}

void CSitePacker::AddHeaderFieldFormat(const char *pField, ...)
{
	char aHeader[512];
	va_list Arguments;
	va_start(Arguments, pField);
	str_format_list(aHeader, sizeof(aHeader), pField, &Arguments);
	va_end(Arguments);
	AddHeaderField(aHeader);
}

void CSitePacker::AddContentLengthHeader()
{
	char aBuf[128];
	long ContentLength = 0;
	m_lpBlocks[BLOCK_CONTENT].Execute(ExecuteTextLength, &ContentLength);
	str_format(aBuf, sizeof(aBuf), "Content-Length: %ld", ContentLength);
	AddHeaderField(aBuf);
}

void CSitePacker::AddContent(const char *pContent, bool Encode)
{
	std::string enc = Encode ? utf8_encode(pContent) : pContent;

	int BufSize = enc.length() + 1;
	char *pBuf = new char[BufSize];
	str_copy(pBuf, enc.c_str(), BufSize);
	m_lpBlocks[BLOCK_CONTENT] += pBuf;
}

void CSitePacker::AddContent(int Content)
{
	char aContent[5];
	str_format(aContent, sizeof(aContent), "%i", Content);
	AddContent(aContent);
}

void CSitePacker::AddContent(long long Content)
{
	char aContent[9];
	str_format(aContent, sizeof(aContent), "%lld", Content);
	AddContent(aContent);
}

void CSitePacker::AddContent(float Content)
{
	char aContent[5];
	str_format(aContent, sizeof(aContent), "%f", Content);
	AddContent(aContent);
}

void CSitePacker::AddContentFormat(const char *pAttr, ...)
{
	char aAttribute[512];
	va_list Arguments;
	va_start(Arguments, pAttr);
	str_format_list(aAttribute, sizeof(aAttribute), pAttr, &Arguments);
	va_end(Arguments);
	AddContent(aAttribute);
}

void CSitePacker::AddStyle(const char *pAttr)
{
	int BufSize = str_length(pAttr) + 1;
	char *pBuf = new char[BufSize];
	str_copy(pBuf, pAttr, BufSize);
	m_lpBlocks[BLOCK_STYLE] += pBuf;
}

void CSitePacker::AddStyleFormat(const char *pAttr, ...)
{
	char aAttribute[512];
	va_list Arguments;
	va_start(Arguments, pAttr);
	str_format_list(aAttribute, sizeof(aAttribute), pAttr, &Arguments);
	va_end(Arguments);
	AddStyle(aAttribute);
}

void CSitePacker::StyleToContent()
{
	m_lpBlocks[BLOCK_STYLE].Execute(ExecuteAddContentText, this);
}

bool CSitePacker::HasStyle()
{
	return m_lpBlocks[BLOCK_STYLE].Size() > 0;
}

void CSitePacker::AddStyleFile(const char *pFile)
{
	int BufSize = str_length(pFile) + 1;
	char *pBuf = new char[BufSize];
	str_copy(pBuf, pFile, BufSize);
	m_lpBlocks[BLOCK_STYLEFILE] += pBuf;
}

const char *CSitePacker::GetStyleFile(int Index)
{
	return m_lpBlocks[BLOCK_STYLEFILE][Index];
}

int CSitePacker::NumStyleFiles()
{
	return m_lpBlocks[BLOCK_STYLEFILE].Size();
}

void CSitePacker::DeleteStyles()
{
	m_lpBlocks[BLOCK_STYLE].DeleteAll();
	m_lpBlocks[BLOCK_STYLEFILE].DeleteAll();
}

void CSitePacker::AddScript(const char *pLine)
{
	int BufSize = str_length(pLine) + 1;
	char *pBuf = new char[BufSize];
	str_format(pBuf, BufSize, "%s", pLine);
	m_lpBlocks[BLOCK_SCRIPT] += pBuf;
}

void CSitePacker::AddScriptFormat(const char *pLine, ...)
{
	char aLine[512];
	va_list Arguments;
	va_start(Arguments, pLine);
	str_format_list(aLine, sizeof(aLine), pLine, &Arguments);
	va_end(Arguments);
	AddScript(aLine);
}

void CSitePacker::AddScriptFormatSized(int MaxLength, const char *pLine, ...)
{
	char *pLineComposed = new char[MaxLength];
	va_list Arguments;
	va_start(Arguments, pLine);
	str_format_list(pLineComposed, MaxLength, pLine, &Arguments);
	va_end(Arguments);
	AddScript(pLineComposed);
	delete[] pLineComposed;
}

void CSitePacker::ScriptToContent()
{
	m_lpBlocks[BLOCK_SCRIPT].Execute(ExecuteAddContentText, this);
}

bool CSitePacker::HasScript()
{
	return m_lpBlocks[BLOCK_SCRIPT].Size() > 0;
}

void CSitePacker::AddScriptFile(const char *pFile)
{
	int BufSize = str_length(pFile) + 1;
	char *pBuf = new char[BufSize];
	str_copy(pBuf, pFile, BufSize);
	m_lpBlocks[BLOCK_SCRIPTFILE] += pBuf;
}

const char *CSitePacker::GetScriptFile(int Index)
{
	return m_lpBlocks[BLOCK_SCRIPTFILE][Index];
}

int CSitePacker::NumScriptFiles()
{
	return m_lpBlocks[BLOCK_SCRIPTFILE].Size();
}

void CSitePacker::DeleteScripts()
{
	m_lpBlocks[BLOCK_SCRIPT].DeleteAll();
	m_lpBlocks[BLOCK_SCRIPTFILE].DeleteAll();
}

void CSitePacker::Print()
{
	print("Printing Site <Start>");
	m_lpBlocks[BLOCK_HEADERFIELDS].Execute(ExecutePrint, 0x0);
	print_raw("\r\n");
	m_lpBlocks[BLOCK_CONTENT].Execute(ExecutePrint, 0x0);
	print_raw("\r\n");
	print("Printing Site <End>");
}

void CSitePacker::Send(CWebserver *pWebServer, CInfoTcpThread *pThreadInfo, bool Close)
{	
	m_lpBlocks[BLOCK_HEADERFIELDS].Execute(ExecuteSend, pThreadInfo);
	pWebServer->SendWeb(pThreadInfo, "\r\n", 2, false);
	m_lpBlocks[BLOCK_CONTENT].Execute(ExecuteSend, pThreadInfo);

	if (Close == true)
		pWebServer->CloseConnection(pThreadInfo);
}

const char *CSitePacker::operator()() const
{
	return "\r\n";
}