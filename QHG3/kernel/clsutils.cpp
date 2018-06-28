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


//-----------------------------------------------------------------------------
// readKeyArr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int readKeyArr(char *pLine, const char *pKey, const char *pSep, int iNum,  T *pV) {
    int iResult = -1;
    char *pVal = readKeyString(pLine, pKey, pSep);
  
    if (pVal != NULL) {
        char *pCtx=NULL;
        int iC = 0;
        iResult = 0;
        char *pCur = strtok_r(pVal, " \t,", &pCtx);
        T tDummy;
        while ((iC < iNum) && (pCur != NULL) && (iResult == 0)) {
            
            if (strToNum(pCur, &tDummy)) {
                pV[iC++] = tDummy;
            } else {
                printf("Invalid number for \"%s\" [%s]\n", pKey, pVal);
                iResult = -1;
            }
            pCur = strtok_r(NULL, " \t,", &pCtx);
        }

    } else {
        iResult = -1;
    }
    return iResult;
}

#endif
