#pragma once

#include <string>

#include <core/array.hpp>

#define SHARE_MAX_WHITELIST_URLS 10
#define SHARE_CODE_LEN 3

class CHive;

class CShareManager
{
public:

	class CShare
	{
		friend class CShareManager;
	private:
		std::string m_PrimaryLink;
		std::string m_PrimaryLinkAsync;
		std::string m_aWhitelistedURLs[SHARE_MAX_WHITELIST_URLS];
		int m_NumWhitelistedURLs;
		char m_aCode[SHARE_CODE_LEN + 1];
		int m_Type;
		long long m_Time;

	public:
		CShare(int Type, const char *pPrimaryLink, long long Time = 0);

		void AddWhitelistedURL(const char *pURL, bool FormRegEx = true);

		const char *GetCode() { return m_aCode; }
		int GetType() const { return m_Type; }
		long long GetTime() const { return m_Time; }

		friend inline CShare &operator<<(CShare &Out, const char *pURL) { Out.AddWhitelistedURL(pURL); return Out; }
	};

	enum
	{
		SHARE_DOWNLOAD=0,
		SHARE_UPLOAD,
		SHARE_REMOTE,
		SHARE_ATLAS,
		NUM_SHARES,
	};

private:
	CHive *m_pHive;
	CArray<CShare *> m_lpShares;

	void GenerateUniqueCode(char *pCode);

public:
	CShareManager(CHive *pHive);

	bool Handle(const char *pCode, struct CInfoTcpThread *pThreadInfo, class CSession *pSession);
	CShare *NewShare(int Type, const char *pPrimaryLink, std::string aWhitelistedURLs[SHARE_MAX_WHITELIST_URLS], long long Time, const char *pChecksum);
	void EmbedInSite(const char *pIndentation, CShare *pShare, class CSitePacker *pSite, const char *pAuthName);
	void DeleteShare(const char *pCode);
	bool IsWhitelisted(const char *pURL);
	void Tick();
	const char *GetShareTypeName(int Type);
	CShare *GetShare(int Index) { return m_lpShares[Index]; };

	int NumShares() { return m_lpShares.Size(); }
};