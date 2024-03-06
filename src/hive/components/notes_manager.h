#pragma once

#include <string>

class CNotesManager
{
private:
    bool ConvertMarkdownOne(std::string Line, std::string &Result, const char *pRegEx, const char *pFormat);
    bool RegExCompare(std::string Line, std::string &Result, const char *pRegEx, const char *pFormat);

public:
    std::string ConvertMarkdownLine(const char *pLine);
    bool LineRemove(const char *pFilePath, int ListNum, int LineNum, const char *pText);
};