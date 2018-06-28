#ifndef __STDSTRUTILS_H__
#define __STDSTRUTILS_H__

#include <string>
#include <vector>


void splitString(std::string sString, std::vector<std::string> &vParts, char cSep);

std::string trim(const std::string& str);

bool endsWith(const std::string sBig, const std::string sEnd);

bool strReplace(std::string &sBig, const std::string sEnd, const std::string sNew);


#endif
