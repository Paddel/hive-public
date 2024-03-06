#include <string>
#include <tuple>

#include <core/array.hpp>
#include <core/wrapper.h>

#include "helper.h"

char aNums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-'};

inline void ReduceString(char *str, int num)
{
	int len = str_length(str);
	for(int k=num; k < len-1; k++){
		str[k-num] = str[k+1];
	}
	str[len-1-num] = '\0';
}

char *GetSepStr(char SepChar, char **content)
{
	char *pContStr = *content;
	char *pOutStr  = 0;

	if(*content == 0x0)//string is empty
		return (char *)"";

	int len = str_length(pContStr);
	for(int i = 0; i < len; i++)
	{
		if(pContStr[i] && pContStr[i] == SepChar)
		{
			pContStr[i] = '\0';
			pOutStr = &pContStr[0];
			*content = &pContStr[i+1];
			if(i == len-1)
				*content = 0;
			break;
		}
	}

	if(!pOutStr)
	{
		pOutStr = pContStr;
		*content = 0;
	}

	return pOutStr;
}

int GetSepInt(char SepChar, char **content)
{
	char *pNumStr = GetSepStr(SepChar, content);

	if(!pNumStr)
		return -1;

	//Check chars
	for(unsigned int i = 0; i < str_length(pNumStr); i++)
	{
		bool check = false;
		for(unsigned int a = 0; a < str_length(aNums); a++)
		{
			if(pNumStr[i] == aNums[a])
			{//char is available
				check = true;
				break;
			}
		}

		if(!check)
		{//char is NOT available
			return -1;
		}
	}

	return str_toint(pNumStr);
}

inline bool IsAsciiStr(char *content)
{
	// trim right and set maximum length to 128 utf8-characters
	int Length = 0;
	const char *p = content;
	const char *pEnd = 0;
	while(*p)
 	{
		const char *pStrOld = p;
		int Code = str_utf8_decode(&p);

		// check if unicode is not empty
		if(Code > 0x20 && Code != 0xA0 && Code != 0x034F && (Code < 0x2000 || Code > 0x200F) && (Code < 0x2028 || Code > 0x202F) &&
			(Code < 0x205F || Code > 0x2064) && (Code < 0x206A || Code > 0x206F) && (Code < 0xFE00 || Code > 0xFE0F) &&
			Code != 0xFEFF && (Code < 0xFFF9 || Code > 0xFFFC))
		{
			pEnd = 0;
		}
		else if(pEnd == 0)
			pEnd = pStrOld;

		if(++Length >= 127)
		{
			return false;
		}
 	}
	if(pEnd != 0)
		return false;

	return true;
}

int ListDirectoryCollect(const char *pName, int IsDir, int Type, void *pUser)
{
    if((Type == LISTDIR_DIRECTORIES && IsDir == 0) || (Type == LISTDIR_FILES && IsDir == 1) ||
    	(IsDir == 1 && (str_comp(pName, ".") == 0 || str_comp(pName, "..") == 0)))
        return 0;
	CListDirectoryResult NewResult;
	NewResult.m_Name = pName;
	NewResult.m_IsDirectory = IsDir;
    CArray<CListDirectoryResult> *pResult = (CArray<CListDirectoryResult> *)pUser;
    pResult->Add(NewResult);
    return 0;
}

int CListDirectoryResult::operator -(const CListDirectoryResult& Other)
{
	if(m_IsDirectory && !Other.m_IsDirectory)
		return -1;
	else if(!m_IsDirectory && Other.m_IsDirectory)
		return 1;
	else
		return str_comp(m_Name.c_str(), Other.m_Name.c_str());

	if(m_IsDirectory != Other.m_IsDirectory)
		return (int)(Other.m_IsDirectory - m_IsDirectory);
	return str_comp(m_Name.c_str(), Other.m_Name.c_str());
}

void ListDirectory(void *pResult, const char *pPath, int Type)
{
    fs_listdir(pPath, ListDirectoryCollect, Type, pResult);
}

bool StringEndsWith(const char *pString, const char *pSuffix)
{
	int LenString = str_length(pString);
	int LenSuffix = str_length(pSuffix);
	if(LenString < LenSuffix)
		return false;
	return str_comp_num(pString + LenString - LenSuffix, pSuffix, LenSuffix) == 0;
}