/*============================================================================
| strutils
| 
|  Some string utilities. Amongst others,
|  - strToNum(): string to number conversion (for all number types)
|  - putMem(): copying any data to uchar buffer (e.g. writing binary files)
|  - getMem(): copying from buffer to variable (e.g. reading binary files)
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __STRUTILS_H__
#define __STRUTILS_H__

#include "types.h"



/// string utilities
char *trim(char *pString);
bool readHex(char *pString, int *pNum);

char *niceDir(const char *pDir, char *pNice);
char *searchFile(const char *pFile, const char *pDataDir, char *pFinal);

char *fillStringDouble(const char *pName, double dVal, char *pBuffer);

bool splitSizeString(char *pSize, int *piW, int *piH);
bool splitRangeString(char *pRange, double *pdMin, double *pdMax);
bool splitRangeString(char *pRange, float *pdMin, float *pdMax);


void safeStrCpy(char *pDest, const char *pSource, int iLen);
bool strToNum(const char *pData, char *c);
bool strToNum(const char *pData, short int *s);
bool strToNum(const char *pData, uint *u);
bool strToNum(const char *pData, size *s);
bool strToNum(const char *pData, coord *c);
bool strToNum(const char *pData, int *i);
bool strToNum(const char *pData, long *l);
bool strToNum(const char *pData, ulong *l);
bool strToNum(const char *pData, long long *l);
bool strToNum(const char *pData, float *f);
bool strToNum(const char *pData, double *d);

void show(unsigned char *p, int iSize, const char *pCaption);

uchar *putMem(unsigned char *p, const void *pData, ulong); 
uchar *getMem(void *pData, const uchar *p, ulong iSize); 
char  *putMem(char *p, const void *pData, ulong iSize); 
char  *getMem(void *pData, const char *p, ulong iSize); 

char *readKeyString(char *pLine, const char *pKey, const char *pSep);

char *nextWord(char **pString);

char *defaultIfNULL(const char *pGiven, const char *pDefault);

char *centerPad(const char *pString, char cPad, int iSize, char *pOut); 
#endif
