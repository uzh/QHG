#ifndef __BUFWRITER_H__
#define __BUFWRITER_H__

#include <stdio.h>

#define DEF_BUF_SIZE 2048
class BufWriter {
public:
    static BufWriter *createInstance(const char *pOut, int iBufSize=DEF_BUF_SIZE);

    ~BufWriter();
    int addChars(const char *pData, int iNum);
    int addChar(char c);
    int addLine(const char *pLine);
    long getPos() { return m_lPos;};

protected:
    BufWriter();

    int initialize(const char *pOut, int iBufSize);
    int dump();
    
    char *m_pBuffer;
    int   m_iBufSize;
    int   m_iCur;
    FILE *m_fOut;
    long  m_lPos;
};


#endif
