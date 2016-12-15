#include <stdio.h>

#include "utils.h"
#include "types.h"
#include "ranges.h"
#include "QMapUtils.h"


template<class T>
T **QMapUtils::createArray(int iW, int iH, T tDef) {
    T **aatData = new T*[iH];
    for (int i = 0; i < iH; i++) {
        aatData[i] = new T[iW];
        for (int j = 0; j < iW; j++) {
            aatData[i][j] = tDef;
        }
    }
    return aatData;
}


template<class T>
bool QMapUtils::writeArray(FILE *fOut, int iW, int iH, T **aatData) {
    bool bOK = true;
    for (int iY = 0; bOK && (iY < iH); iY++) {
        int iWritten = fwrite(aatData[iY], sizeof(T), iW, fOut);
        if (iWritten != iW) {
            printf("only wrote %d chars at line %d\n", iWritten, iY);
            bOK = false;
        }
    }
    return bOK;
}

template<class T>
void QMapUtils::deleteArray(int iW, int iH, T **aatData) {
    if (aatData != NULL) {
        for (int iY = 0; iY < iH; iY++) {
            if (aatData[iY] != NULL) {
                delete[] aatData[iY];
            }
        }
        delete[] aatData;
    }
}

template<class T>
T **QMapUtils::copyArray(int iW, int iH, T **aatData) {
    T **aatData1 = NULL;
    if (aatData != NULL) {
        aatData1 = new T*[iH];
        for (int i = 0; i < iH; i++) {
            aatData1[i] = new T[iW];
            memcpy(aatData1[i], aatData[i], iW*sizeof(T));
        }
    }
    return aatData1;
}
