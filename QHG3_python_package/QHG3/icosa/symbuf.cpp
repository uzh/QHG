#include "symbuf.h"
#include "BufReader.h"

static const char *asSymNames[] = {
    "ERR",
    "NUL",
    "NUM",
    "OBR",
    "DOT",
    "CBR",
    "LEV",
};
#define UNKNOWN_NAME "unknown symbol"

symbuf::symbuf(BufReader *pBR)
    : m_pBR(pBR),
      m_iCurSym(SYM_NUL),
      m_iCurNum(0) {
}
const char *symbuf::getSymName(int iSym) {
    const char *pC = UNKNOWN_NAME;
    if ((iSym >= SYM_ERR) && (iSym <= SYM_CBR)) {
        pC= asSymNames[iSym];
    }
    return pC;
}

int symbuf::getNextSym() {
    m_iCurSym = SYM_NUL;
    char *p = m_pBR->getNextChar();
    if (p != NULL) {
        char c = *p;
        if (c == '!') {
            m_iCurNum = (*(m_pBR->getNextChar()))-'0';
            m_iCurNum *= 10;
            m_iCurNum += (*(m_pBR->getNextChar()))-'0';
            c =*(m_pBR->getNextChar()); 
            if (c== ':') {
                m_iCurSym = SYM_NUM;
            } else {
                m_iCurSym = SYM_ERR;
            }
        } else if (c == '(') {
            m_iCurSym = SYM_OBR;
        } else if (c == '.') {
            m_iCurSym = SYM_DOT;
        } else if (c == ')') {
            m_iCurSym = SYM_CBR;
        } else if ((c & 0x80) != 0) {
            m_iCurSym = SYM_LEV;
            m_iCurNum = c & 0x7f;
        }
    }
    //    printf("---sb: %s\n", asSymNames[m_iCurSym]);
    return m_iCurSym;
}

