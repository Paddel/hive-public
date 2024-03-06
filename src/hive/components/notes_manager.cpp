#include <fstream>
#include <regex>
#include <tuple>

#include <core/array.hpp>
#include <core/helper/string_helper.hpp>
#include <core/wrapper.h>

#include "notes_manager.h"

#define TUPLE_CONVERSION std::pair<const char *, const char *>
#define REGEX_LIST_REMOVABLE "^\\[-\\]$"

const TUPLE_CONVERSION g_aConvLine[] = {
    TUPLE_CONVERSION("^###### (.*)", "<h6>$1</h6>"),
    TUPLE_CONVERSION("^##### (.*)", "<h5>$1</h5>"),
    TUPLE_CONVERSION("^#### (.*)", "<h4>$1</h4>"),
    TUPLE_CONVERSION("^### (.*)", "<h3>$1</h3>"),
    TUPLE_CONVERSION("^## (.*)", "<h2>$1</h2>"),
    TUPLE_CONVERSION("^# (.*)", "<h1>$1</h1>"),

    TUPLE_CONVERSION("^\\*\\*\\*+$", "<hr>"),
    TUPLE_CONVERSION("^---+$", "<hr>"),
    TUPLE_CONVERSION("^___+$", "<hr>"),

    TUPLE_CONVERSION("^\\[END\\]$", "</div>"),
    TUPLE_CONVERSION(REGEX_LIST_REMOVABLE, "<div class='checklist-removable unselectable'>"),
    //TUPLE_CONVERSION("^[+](.*)", "<p class='check'>$1</a>"),
};

const TUPLE_CONVERSION g_aConvInline[] = {
    TUPLE_CONVERSION("\\*\\*\\*(.*?)\\*\\*\\*", "<strong><em>$1</em></strong>"),
    TUPLE_CONVERSION("\\*\\*_(.*?)_\\*\\*", "<strong><em>$1</em></strong>"),
    TUPLE_CONVERSION("\\*\\*(.*?)\\*\\*", "<strong>$1</strong>"),
    TUPLE_CONVERSION("\\*(.*?)\\*", "<em>$1</em>"),

    TUPLE_CONVERSION("!\\[(.*?)\\]\\((.*?)\\)", "<img src=\"$2\" alt=\"$1\">"),
    TUPLE_CONVERSION("\\[(.*?)\\]\\((.*?)\\)", "<a href=\"$2\">$1</a>"),
};

bool CNotesManager::RegExCompare(std::string Line, std::string &Result, const char *pRegEx, const char *pFormat)
{
    const std::regex BaseRegex(pRegEx);
    Result = std::regex_replace(Line, BaseRegex, pFormat);
    return Result != Line;
}

std::string CNotesManager::ConvertMarkdownLine(const char *pLine)
{
    std::string Result = pLine;
    StringRemoveAll(Result, "\r");
    StringRemoveAll(Result, "\n");

    if(Result.size() == 0)
     return "<br>";

    int NumTuples = sizeof(g_aConvLine) / sizeof(TUPLE_CONVERSION);
    for(int i = 0; i < NumTuples; i++)
        if(RegExCompare(Result, Result, g_aConvLine[i].first, g_aConvLine[i].second))
            return Result;

    std::string ResultFormated = Result;
    int NumConvInline = sizeof(g_aConvInline) / sizeof(TUPLE_CONVERSION);
    for(int i = 0; i < NumConvInline; i++)
        RegExCompare(ResultFormated, ResultFormated, g_aConvInline[i].first, g_aConvInline[i].second);
    Result = std::string("<p>") + ResultFormated + std::string("</p>");
    
    return Result;
}

bool CNotesManager::LineRemove(const char *pFilePath, int ListNum, int LineNum, const char *pText)
{
    CArray<std::string> l_Lines;
    std::fstream File;
    if(ListNum < 0 || LineNum < 0)
        return false;

	File.open(pFilePath, std::ios::in | std::ios::binary);
	if(File.is_open() == false)
		return false;

    std::regex RegExList(REGEX_LIST_REMOVABLE);
    for(std::string Line; getline(File, Line); )
    {
        std::string LineCleaned = Line;
        StringRemoveAll(LineCleaned, "\r");
        StringRemoveAll(LineCleaned, "\n");

        if(ListNum == -1)
        {
            if(LineNum == 0)
                break;
            else
                LineNum--;
        }
        else if (std::regex_match(LineCleaned, RegExList))
            ListNum--;

        l_Lines += LineCleaned;
    }

    for(std::string Line; getline(File, Line); )
    {
        StringRemoveAll(Line, "\r");
        StringRemoveAll(Line, "\n");
        l_Lines += Line;
    }

    File.close();

    File.open(pFilePath, std::ios::out | std::ios::binary);
	if(File.is_open() == false)
		return false;
    
    for(int i = 0; i < l_Lines.Size(); i++)
        File << l_Lines[i] << "\r\n";

    File.close();
    return true;
}