#include <core/wrapper.h>

#include "url_coder.h"

std::string url_encode(std::string str)
{
	std::string new_str = "";
	unsigned char c;
	int ic;
	const char* chars = str.c_str();
	char bufHex[10];
	int len = str_length(chars);

	for (int i = 0; i<len; i++) {
		c = chars[i];
		ic = c;
		// uncomment this if you want to encode spaces with +
		if (c==' ') new_str += '+';
		else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
		else {
			sprintf(bufHex, "%X", c);
			if (ic < 16)
				new_str += "%0";
			else
				new_str += "%";
			new_str += bufHex;
		}
	}
	return new_str;
}

std::string url_decode(std::string str)
{
	std::string ret;
	char ch;
	int i, ii, len = str.length();

	for (i = 0; i < len; i++) {
		if (str[i] != '%') {
			if (str[i] == '+')
				ret += ' ';
			else
				ret += str[i];
		}
		else {
			sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
	}
	return ret;
}

void url_clear(char *pStr)
{
	int length = str_length(pStr);
	for (int i = 1; i < length; i++)
	{
		if (pStr[i] == '/' && pStr[i - 1] == '/')
			mem_copy(pStr + i - 1, pStr + i, length-- - i + 1);
	}

	if (pStr[length - 1] == '/' && length > 1)
		pStr[length - 1] = '\0';
}