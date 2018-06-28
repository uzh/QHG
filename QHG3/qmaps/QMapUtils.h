#ifndef __QMAPUTILS_H__
#define __QMAPUTILS_H__

#include <stdio.h>

class ValReader;
       
namespace QMapUtils {
#ifndef NULL
#define NULL 0
#endif
    ValReader *createValReader(const char *pFile, bool bInterp,int *piDataType=NULL);
    ValReader *createValReader(const char *pFile, const char *pRange, bool bPrepareArrays, bool bInterp, int *piDataType=NULL);

    template<class T>
    T **createArray(int iW, int iH, T tDef);


    template<class T>
    bool writeArray(FILE *fOut, int iW, int iH, T **aatData);

    template<class T>
    void deleteArray(int iW, int iH, T **aatData); 

    template<class T>
    T **copyArray(int iW, int iH, T **aatData); 

}

#endif

