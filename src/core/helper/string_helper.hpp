#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <core/wrapper.h>

static const std::string BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static std::string charToHex(char c) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
    return ss.str();
}

static std::string utf8_encode(const char *pStr)
{
	//return std::string(pStr);
	std::string builder;
	int len = str_length(pStr);
	for (int i = 0; i < len; i++)
	{
		char aUTF8Char[5] = {};
		str_utf8_encode(aUTF8Char, (unsigned char)pStr[i]);
		builder += aUTF8Char;
	}
	return builder;
}

static std::string utf8_decode(const char *pStr)
{
	//return std::string(pStr);
	std::string builder;
	while (*pStr)
		builder += str_utf8_decode(&pStr);

	return builder;
}

static void StringReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static void StringRemoveAll(std::string& str, const std::string& from)
{
    StringReplaceAll(str, from, "");
}

static std::string EscapeJsonChars(std::string Input)
{
	StringReplaceAll(Input, "\\", "\\\\");
	StringReplaceAll(Input, "\"", "\\\"");
    StringReplaceAll(Input, "\n", "\\n");
    StringReplaceAll(Input, "\r", "\\r");
    StringReplaceAll(Input, "\t", "\\t");
	return Input;
}

static std::string EscapeXssChars(std::string Input)
{
	StringReplaceAll(Input, "&", "&amp");
	StringReplaceAll(Input, "\"", "&quot");
	StringReplaceAll(Input, "\'", "&#39");
	StringReplaceAll(Input, "<", "&lt");
	StringReplaceAll(Input, ">", "&gt");
	return Input;
}

static std::string EscapeRegExChars(std::string Input)
{
    StringReplaceAll(Input, "/", "\\/");
    StringReplaceAll(Input, "\\", "\\\\");
    StringReplaceAll(Input, "^", "\\^");
    StringReplaceAll(Input, "$", "\\$");
    StringReplaceAll(Input, ".", "\\.");
    StringReplaceAll(Input, "|", "\\|");
    StringReplaceAll(Input, "?", "\\?");
    StringReplaceAll(Input, "*", "\\*");
    StringReplaceAll(Input, "+", "\\+");
    StringReplaceAll(Input, "(", "\\(");
    StringReplaceAll(Input, ")", "\\)");
    StringReplaceAll(Input, "[", "\\[");
    StringReplaceAll(Input, "]", "\\]");
    StringReplaceAll(Input, "{", "\\{");
    StringReplaceAll(Input, "}", "\\}");
	return Input;
}

static std::vector<uint8_t> stringToBytes(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

static std::string Base64Encode(const std::string& str) {
    std::vector<uint8_t> data = stringToBytes(utf8_encode(str.c_str()));
    std::string result;
    int len = data.size();
    int i = 0;

    while (i < len) {
        int a = i < len ? data[i++] : 0;
        int b = i < len ? data[i++] : 0;
        int c = i < len ? data[i++] : 0;

        int value = (a << 16) | (b << 8) | c;

        result.push_back(BASE64_CHARS[(value >> 18) & 0x3F]);
        result.push_back(BASE64_CHARS[(value >> 12) & 0x3F]);
        result.push_back((a || i > len + 1) ? BASE64_CHARS[(value >> 6) & 0x3F] : '=');
        result.push_back((b || i > len) ? BASE64_CHARS[value & 0x3F] : '=');
    }

    return result;
}

static std::string Base64Decode(const std::string& base64_data) {
    if (base64_data.empty()) return std::string();

    std::vector<int> base64_decoding(256, -1);
    for (int i = 0; i < BASE64_CHARS.size(); ++i) {
        base64_decoding[BASE64_CHARS[i]] = i;
    }

    int len = base64_data.size();
    std::string result;
    int i = 0;

    while (i < len) {
        int a = base64_decoding[base64_data[i++]];
        int b = base64_decoding[base64_data[i++]];
        int c = base64_decoding[base64_data[i++]];
        int d = base64_decoding[base64_data[i++]];

        int value = (a << 18) | (b << 12) | (c << 6) | d;

        result.push_back((value >> 16) & 0xFF);
        if (base64_data[i - 2] != '=') result.push_back((value >> 8) & 0xFF);
        if (base64_data[i - 1] != '=') result.push_back(value & 0xFF);
    }

    return result;
}