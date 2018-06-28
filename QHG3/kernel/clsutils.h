#ifndef __CLSUTILS_H__
#define __CLSUTILS_H__

template<typename T>
int readKeyVal(char *pLine, const char *pKey, const char *pSep, T *pV);
template<typename T>
int readKeyArr(char *pLine, const char *pKey, const char *pSep, int iNum, T *pV);

#endif
