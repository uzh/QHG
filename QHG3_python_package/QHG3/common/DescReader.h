#ifndef __DESCREADER_H__
#define __DESCREADER_H__

#include "types.h"

typedef union {
    char      c;
    short int s;
    int       i;
    long long      l;
    float     f;
    double    d;
} unumber;

class DescReader {
public:
    DescReader(const char *pDesc);
    ~DescReader();
    char getNextItem();
    void reset();

    static size_t calcDataSize(char *pDataDesc);
    static unsigned char *data2AscFile(char *pDesc, FILE *fOut, unsigned char *p0);
    static int getValue(char *p, char cType, unumber *pval);

    static double *extractData(const char *pDesc, int iFieldNo, int iNumItems, unsigned char *p0);
    int getEffPos() {return m_iPos;};
    int getEffLen() {return m_iLen;};
    int getDataSize() {return m_iDataSize;};
protected:
    void initialize(const char *pDesc);

    char *m_pExp;
    const char *m_pCur;
    int m_iRepeat;


    uint m_iPos;
    uint m_iLen;
    uint m_iDataSize;

};

#endif
