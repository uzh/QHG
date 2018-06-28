#ifndef __DBGPRINT_H__
#define __DBGPRINT_H__

#include <stdio.h>
#include <stdarg.h>

#define LL_NONE   0
#define LL_TOP    1
#define LL_INFO   2
#define LL_DETAIL 3

void dbgprintf(int iLogLevel, int iThisLevel, const char * pFormat, ...);



#endif
