#ifndef __CLSUTILS_CPP__
#define __CLSUTILS_CPP__

#include <stdio.h>
#include <string.h>

#include "strutils.h"
#include "clsutils.h"



//-----------------------------------------------------------------------------
// readKeyVal
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int readKeyVal(char *pLine, const char *pKey, const char *pSep, T *pV) {
    int iResult = -1;
    char *pVal = readKeyString(pLine, pKey, pSep);
  
    if (pVal != NULL) {
        if (strToNum(pVal, pV)) {
            iResult = 0;
        } else {
            printf("Invalid number for \"%s\" [%s]\n", pKey, pVal);
            iResult = -1;
        }
    } else {
        iResult = -1;
    }
    return iResult;
}


#endif
