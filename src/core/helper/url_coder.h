#pragma once

#include <string>

std::string utf8_encode(const char *pStr);
std::string utf8_decode(const char *pStr);
std::string url_encode(std::string str);
std::string url_decode(std::string str);
void url_clear(char *pStr);