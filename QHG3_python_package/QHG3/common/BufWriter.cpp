#include <stdio.h>
#include <string.h>

#include "BufWriter.h"


//----------------------------------------------------------------------------
// constructor
//
BufWriter::BufWriter() 
    : m_pBuffer(NULL),
      m_iBufSize(0),
      m_iCur(0),
      m_fOut(NULL),
      m_lPos(0) {
}

//----------------------------------------------------------------------------
// destructor
//
BufWriter::~BufWriter() {
    if (m_fOut != NULL) {
        dump();
        fclose(m_fOut);
    }

    if (m_pBuffer != NULL) {
        delete[] m_pBuffer;
    }
}


//----------------------------------------------------------------------------
// createInstance
//
BufWriter *BufWriter::createInstance(const char *pOut, int iBufSize) {
    BufWriter *pBW = new BufWriter();
    int iResult = pBW->initialize(pOut,iBufSize);
    if (iResult != 0) {
        delete pBW;
        pBW = NULL;
    }
    return pBW;
}

//----------------------------------------------------------------------------
// initialize
//
int BufWriter::initialize(const char *pOut, int iBufSize) {
    int iResult = -1;
    m_fOut = fopen(pOut, "wb");
    if (m_fOut != NULL) {
        m_iBufSize = iBufSize;
        m_pBuffer = new char[m_iBufSize];
        iResult = 0;
    } else {
        iResult = -1;
        printf("[BufWriter] Couldn't open file for writing [%s]\n", pOut);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// dump
//
int BufWriter::dump() {
    int iResult =0;

    if (m_iCur > 0) {
        size_t iWritten = fwrite(m_pBuffer, m_iCur, 1, m_fOut);
        if (iWritten == 1) {
            m_iCur = 0;
        } else {
            iResult = -1;
            printf("[BufWriter] Error while writing to file\n");
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// addChars
//
int BufWriter::addChars(const char *pData, int iNum) {
    int iResult = 0;

    m_lPos += iNum;
    while ((iResult == 0) &&(iNum > 0)) {
        int iNumCopy;
        if (iNum > m_iBufSize-m_iCur) {
            iNumCopy = m_iBufSize-m_iCur;
        } else {
            iNumCopy = iNum;
        }
        iNum -= iNumCopy;

        memcpy(m_pBuffer+m_iCur, pData, iNumCopy);
        m_iCur += iNumCopy;
        pData += iNumCopy;
        if (m_iCur >= m_iBufSize) {
            iResult = dump();
        }
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// addChar
//
int BufWriter::addChar(char c) {
    int iResult = 0;
    
    m_pBuffer[m_iCur] = c;
    m_iCur++;
    m_lPos++;
    if (m_iCur >=  m_iBufSize) {
        iResult = dump();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// addLine
//
int BufWriter::addLine(const char *pLine) {
    int iResult = 0;
    const char *p = pLine;

    while ((*p != '\0') && (iResult == 0)) {
        iResult = addChar(*p);
        p++;
    }
    return iResult;
}
