#ifndef __SYMBUF_H__
#define __SYMBUF_H__

class BufReader;

// symbols are NUM, OBR, DOT, CBR

#define SYM_ERR 0
#define SYM_NUL 1
#define SYM_NUM 2
#define SYM_OBR 3
#define SYM_DOT 4
#define SYM_CBR 5
#define SYM_LEV 6



#include "BufReader.h"

class symbuf {
public:
    symbuf(BufReader *pBR);
    int getNextSym();
    int getCurSym() { return m_iCurSym;};
    int getCurNum() { return m_iCurNum;};
    const char *getSymName(int iSym);
protected:
    BufReader *m_pBR;
    int   m_iCurSym;
    int   m_iCurNum;
};

#endif
