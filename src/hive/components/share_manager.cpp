#include <regex>

#include <core/helper/string_helper.hpp>
#include <core/helper/url_coder.h>
#include <core/md5/md5.h>
#include <hive/hive.h>
#include <hive/info_thread.h>
#include <hive/webserver/session.h>
#include <hive/webserver/webserver.h>

#include "share_manager.h"

#define SALT_CHECKSUM "a8edaeaae7051ae21379ae4b70f289e105807a0b1c68fca8ddee01d1b382ceca"

static const char *s_aShareTypeNames[CShareManager::NUM_SHARES] = {
	"Download",
	"Upload",
	"Remote",
	"Atlas",
};

CShareManager::CShareManager(CHive *pHive)
{
	m_pHive = pHive;
}

CShareManager::CShare::CShare(int Type, const char *pPrimaryLink, long long Time)
{
	m_Type = Type;
	m_Time = Time;
	m_PrimaryLink = pPrimaryLink;

	bool HasArgs = m_PrimaryLink.find('?') != std::string::npos;
	m_PrimaryLinkAsync = m_PrimaryLink + (HasArgs ? "&" : "?") + "async=1";
	m_NumWhitelistedURLs = 0;
}

void CShareManager::CShare::AddWhitelistedURL(const char *pURL, bool FormRegEx)
{
	if (m_NumWhitelistedURLs >= SHARE_MAX_WHITELIST_URLS || pURL == 0x0 || pURL[0] == '\0')
		return;
	std::string ClearedURL = pURL;
	if(FormRegEx == true)
	{
		EscapeRegExChars(ClearedURL);
		ClearedURL.insert(0, "^");
		ClearedURL.append("$");
	}
	m_aWhitelistedURLs[m_NumWhitelistedURLs++] = ClearedURL;
}

void CShareManager::GenerateUniqueCode(char *pCode)
{
	bool Unique = false;
	while (Unique == false)
	{
		for (int i = 0; i < SHARE_CODE_LEN; i++)
			pCode[i] = '0' + randInt() % 10;
		pCode[SHARE_CODE_LEN] = '\0';

		Unique = true;
		for (int i = 0; i < m_lpShares.Size(); i++)
		{
			if (str_comp(m_lpShares[i]->m_aCode, pCode) == 0)
			{
				Unique = false;
				break;
			}
		}
	}
}

bool CShareManager::Handle(const char *pCode, CInfoTcpThread *pThreadInfo, CSession *pSession)
{
	for (int i = 0; i < m_lpShares.Size(); i++)
	{
		if (str_comp(m_lpShares[i]->m_aCode, pCode) == 0)
		{
			m_pHive->Webserver()->RespondRedirect(pThreadInfo, m_lpShares[i]->m_PrimaryLink.c_str());
			return true;
		}
	}

	return false;
}

CShareManager::CShare *CShareManager::NewShare(int Type, const char *pPrimaryLink, std::string aWhitelistedURLs[SHARE_MAX_WHITELIST_URLS], long long Time, const char *pChecksum)
{
	CShare *pShare = new CShare(Type, url_decode(pPrimaryLink).c_str(), Time);	
	for (int i = 0; i < SHARE_MAX_WHITELIST_URLS; i++)
		pShare->AddWhitelistedURL(url_decode(aWhitelistedURLs[i]).c_str(), false);

	std::string ChecksumInput = std::to_string(pShare->m_Type) + pShare->m_PrimaryLink;
	for (int i = 0; i < pShare->m_NumWhitelistedURLs; i++)
		ChecksumInput += pShare->m_aWhitelistedURLs[i];
	ChecksumInput += SALT_CHECKSUM;

	if (md5(ChecksumInput) != pChecksum)
	{
		delete pShare;
		return 0x0;
	}

	GenerateUniqueCode(pShare->m_aCode);
	m_lpShares += pShare;
	return pShare;
}

void CShareManager::EmbedInSite(const char *pIndentation, CShare *pShare, CSitePacker *pSite, const char *pAuthName)
{
	*pSite << pIndentation << "<input type='hidden' name='type' value='" << pShare->m_Type << "'>" << (*pSite)();
	*pSite << pIndentation << "<input type='hidden' name='prim' value='" << url_encode(pShare->m_PrimaryLink) << "'>" << (*pSite)();
	for(int i = 0; i < pShare->m_NumWhitelistedURLs; i++)
		*pSite << pIndentation << "<input type='hidden' name='wl" << i << "' value='" << url_encode(pShare->m_aWhitelistedURLs[i]) << "'>" << (*pSite)();

	std::string ChecksumInput = std::to_string(pShare->m_Type) + pShare->m_PrimaryLink;
	for (int i = 0; i < pShare->m_NumWhitelistedURLs; i++)
		ChecksumInput += pShare->m_aWhitelistedURLs[i];
	ChecksumInput += SALT_CHECKSUM;
	*pSite << pIndentation << "<input type='hidden' name='checksum' value='" << md5(ChecksumInput) << "'>" << (*pSite)();

	*pSite << pIndentation << "<input class='input-" << pAuthName << "' type='number' name='time' min='1' max='499' value='5'>" << (*pSite)();
	*pSite << pIndentation << "<select class='input-" << pAuthName << "' name='timescale'>" << (*pSite)();
	*pSite << pIndentation << " <option value='1'>Minutes</option>" << (*pSite)();
	*pSite << pIndentation << " <option value='60'>Hours</option>" << (*pSite)();
	*pSite << pIndentation << " <option value='1440'>Days</option>" << (*pSite)();
	*pSite << pIndentation << "</select>" << (*pSite)();
	*pSite << pIndentation << "<a class='button button-" << pAuthName << "'>Share</a>" << (*pSite)();
}

void CShareManager::DeleteShare(const char *pCode)
{
	for (int i = 0; i < m_lpShares.Size(); i++)
	{
		if (str_comp(m_lpShares[i]->m_aCode, pCode) == 0)
		{
			m_lpShares.Delete(i);
			i--;
		}
	}
}

bool CShareManager::IsWhitelisted(const char *pURL)
{
	std::string UrlEnc = url_encode(pURL);

	for (int i = 0; i < m_lpShares.Size(); i++)
	{
		if (m_lpShares[i]->m_PrimaryLink == pURL || m_lpShares[i]->m_PrimaryLinkAsync == pURL)
			return true;

		for (int j = 0; j < m_lpShares[i]->m_NumWhitelistedURLs; j++)
		{
			const std::regex BaseRegex(m_lpShares[i]->m_aWhitelistedURLs[j].c_str());
			if (std::regex_match(pURL, BaseRegex) == true)
			{
				// print("Whitelisted: %s", pURL);
				return true;
			}
		}
	}

	return false;
}

void CShareManager::Tick()
{
	for (int i = 0; i < m_lpShares.Size(); i++)
	{
		if (m_lpShares[i]->m_Time < timer_get())
		{
			m_lpShares.Delete(i);
			i--;
		}
	}
}

const char *CShareManager::GetShareTypeName(int Type) { return s_aShareTypeNames[Type]; };