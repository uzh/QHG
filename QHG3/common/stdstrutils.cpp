#include <string>
#include <vector>
#include "stdstrutils.h"

//----------------------------------------------------------------------------
// writeResumeConfig
//  write output
//
void splitString(std::string sString, std::vector<std::string> &vParts, char cSep) {

    size_t iPos0 = 0;
    if (sString.at(iPos0) == '\'') {
        iPos0++;
    }
    size_t iPos = iPos0;
    while (iPos != std::string::npos) {
        size_t iPos1 = sString.find(cSep, iPos+1);
        if (iPos1 != std::string::npos) {
            vParts.push_back(sString.substr(iPos, iPos1-iPos));
            iPos = iPos1+1;
        } else {
            std::string sTemp = sString.substr(iPos, std::string::npos);
            if (sTemp[sTemp.size()-1] == '\'') {
                sTemp = sTemp.substr(0, sTemp.size()-1);
            }
            
            vParts.push_back(sTemp);
            iPos = iPos1;
        }
    }
    
}

//----------------------------------------------------------------------------
// writeResumeConfig
//  write output
//
std::string trim(const std::string& str) {
    std::string sOut = "";
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        sOut = str;
    } else {
        size_t last = str.find_last_not_of(' ');
        sOut = str.substr(first, (last - first + 1));
    }
    return sOut;
}

//----------------------------------------------------------------------------
// writeResumeConfig
//  write output
//
bool endsWith(const std::string sBig, const std::string sEnd) {
    return sBig.find(sEnd) == sBig.size() - sEnd.size();
}

//----------------------------------------------------------------------------
// strReplace
//
bool strReplace(std::string &sBig, const std::string sEnd, const std::string sNew) {
    bool bReplaced = false;
    size_t iPos = sBig.find(sEnd);
    if (iPos != std::string::npos) {
        sBig = sBig.replace(iPos, sEnd.size(), sNew);
        bReplaced = true;
    }
    return bReplaced;
}
