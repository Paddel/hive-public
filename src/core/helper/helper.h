#pragma once

char *GetSepStr(char SepChar, char **content);
int GetSepInt(char SepChar, char **content);

struct CListDirectoryResult
{
    std::string m_Name;
    bool m_IsDirectory;

    int operator -(const CListDirectoryResult& Other);// { return str_comp(m_aName, Other.m_aName); };
};

enum
{
    LISTDIR_FILES = 0,
    LISTDIR_DIRECTORIES,
    LISTDIR_BOTH,
};

void ListDirectory(void *pResult, const char *pPath, int Type = 0);

bool StringEndsWith(const char *pString, const char *pSuffix);