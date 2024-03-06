#pragma once

#include <core/list.hpp>
#include <core/wrapper.h>

class CSitePacker
{
	enum
	{
		BLOCK_HEADERFIELDS=0,
		BLOCK_CONTENT,
		BLOCK_STYLE,
		BLOCK_STYLEFILE,
		BLOCK_SCRIPT,
		BLOCK_SCRIPTFILE,

		NUM_BLOCKS,
	};

private:
	CList<char *> m_lpBlocks[NUM_BLOCKS];
	bool m_IsScriptSite;

	static void ExecuteAddContentText(void *pData, char *pText);
	static void ExecuteTextLength(void *pData, char *pText);
	static void ExecuteSend(void *pData, char *pText);
	static void ExecutePrint(void *pData, char *pText);

public:
	CSitePacker(const char *pStatus = "HTTP/1.1 200 OK", const char *pContentType = "text/html");
	~CSitePacker();

	void AddHeaderField(const char *pField);
	void AddHeaderFieldFormat(const char *pField, ...);
	void AddContentLengthHeader();
	void AddContent(const char *pContent, bool Encode = true);
	void AddContent(int Content);
	void AddContent(long long Content);
	void AddContent(float Content);
	void AddContentFormat(const char *pAttr, ...);
	void AddStyle(const char *pAttr);
	void AddStyleFormat(const char *pAttr, ...);
	void StyleToContent();
	bool HasStyle();
	void AddStyleFile(const char *pFile);
	const char *GetStyleFile(int Index);
	int NumStyleFiles();
	void DeleteStyles();
	void AddScript(const char *pLine);
	void AddScriptFormat(const char *pLine, ...);
	void AddScriptFormatSized(int MaxLength, const char *pLine, ...);
	void ScriptToContent();
	bool HasScript();
	void AddScriptFile(const char *pFile);
	const char *GetScriptFile(int Index);
	int NumScriptFiles();
	void DeleteScripts();
	void Print();
	void Send(class CWebserver *pWebServer, struct CInfoTcpThread *pThreadInfo, bool Close = true);
	const char *operator()() const;

	friend inline CSitePacker &operator<<(CSitePacker &Out, const char *pContent) { Out.AddContent(pContent); return Out; }
	friend inline CSitePacker &operator<<(CSitePacker &Out, int Content) { Out.AddContent(Content); return Out; }
	friend inline CSitePacker &operator<<(CSitePacker &Out, long long Content) { Out.AddContent(Content); return Out; }
	friend inline CSitePacker &operator<<(CSitePacker &Out, float Content) { Out.AddContent(Content); return Out; }
	friend inline CSitePacker &operator<<(CSitePacker &Out, std::string Content) { Out.AddContent(Content.c_str()); return Out; }
};