/*============================================================================
|  BufReader
|
|   A BufReader object provides buffered file input.
|   Single Characters or entire blocks can be read, without worrying about
|   the buffer size.
|   
\===========================================================================*/

#ifndef __BUFREADER_H__
#define __BUFREADER_H__
#include <stdio.h>
#include "types.h"

class BufReader {
public:
    static BufReader *createInstance(const char *pFile, uint iBufSize);
    static BufReader *createInstance(FILE *fIn, uint iBufSize);
    ~BufReader();
    
    char *getNextChar();
    int getBlock(char *p, uint iSize);
    int getBlock(char *p, uint *piSize, const char *pLim);
    char *skip(uint iSkip);
    long getPos() { return m_lBlockPos+m_iCurPos;};
    long getCurPos() { return m_iCurPos;};
    int loadFromCurrentPos();

protected:
    BufReader();
    int initialize(const char *pFile, uint iBufSize);
    int initialize(FILE *fIn, uint iBufSize, bool bCloseFile);
    
    char *m_pBuffer;
    uint   m_iBufSize;
    uint   m_iCurSize;
    uint   m_iCurPos;
    long   m_lBlockPos;
    FILE *m_fIn;
    bool  m_bCloseFile;
};

#endif
