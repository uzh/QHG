#include <stdio.h>
#include <string.h>
#include "BufReader.h"


//----------------------------------------------------------------------------
// constructor
//
BufReader::BufReader() 
    : m_pBuffer(NULL),
      m_iBufSize(0),
      m_iCurSize(0),
      m_iCurPos(0),
      m_lBlockPos(0),
      m_fIn(NULL),
      m_bCloseFile(false) {
}


//----------------------------------------------------------------------------
// destructor
//
BufReader::~BufReader() {
    if (m_bCloseFile && (m_fIn != NULL)) {
        fclose(m_fIn);
    }

    if (m_pBuffer != NULL) {
        delete[] m_pBuffer;
    }
}


//----------------------------------------------------------------------------
// createInstance
//
BufReader *BufReader::createInstance(const char *pFile, uint iBufSize) {
    BufReader *pBR = new BufReader();
    int iResult = pBR->initialize(pFile,iBufSize);
    if (iResult != 0) {
        delete pBR;
        pBR = NULL;
    }
    return pBR;
}


//----------------------------------------------------------------------------
// createInstance
//
BufReader *BufReader::createInstance(FILE *fIn, uint iBufSize) {
    BufReader *pBR = new BufReader();
    int iResult = pBR->initialize(fIn, iBufSize, false);
    if (iResult != 0) {
        delete pBR;
        pBR = NULL;
    }
    return pBR;
}


//----------------------------------------------------------------------------
// initialize
//
int BufReader::initialize(FILE *fIn, uint iBufSize, bool bCloseFile) {
    int iResult = -1;
    m_fIn = fIn;
    m_bCloseFile = bCloseFile;
    if (m_fIn != NULL) {
        iResult = 0;
        m_iBufSize = iBufSize;
        m_pBuffer = new char[m_iBufSize];
        m_lBlockPos = ftell(m_fIn);
        m_iCurSize = (uint)fread(m_pBuffer, 1, m_iBufSize, m_fIn);
        if (m_iCurSize != m_iBufSize) {
            if (!feof(m_fIn)) {
                iResult = -1;
            } 
         } 
    } else {
        iResult = -1;
        printf("[BufReader] Couldn't open file for reading\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// initialize
//
int BufReader::initialize(const char *pFile, uint iBufSize) {
    int iResult = -1;
    FILE *fIn = fopen(pFile, "rb");
    iResult = initialize(fIn, iBufSize, true);
    return iResult;
}


//----------------------------------------------------------------------------
// loadFromCurrentPos
//  load the buffer from current position
//
int BufReader::loadFromCurrentPos() {
    int iResult = 0;
    m_lBlockPos = ftell(m_fIn);
    m_iCurSize = (uint)fread(m_pBuffer, 1, m_iBufSize, m_fIn);
    if (m_iCurSize != m_iBufSize) {
        if (!feof(m_fIn)) {
            iResult = -1;
        } 
    }
    m_iCurPos = 0;
    return iResult; 
}


//----------------------------------------------------------------------------
// getNextChar
//
char *BufReader::getNextChar() {
    bool bOK = true;
    if (m_iCurPos >= m_iCurSize) {
        //        printf("reloading %d\n", m_iBufSize);
        m_lBlockPos = ftell(m_fIn);
        m_iCurSize = (uint)fread(m_pBuffer, 1, m_iBufSize, m_fIn);
        //        printf("  got %d\n", m_iCurSize);

        m_iCurPos = 0;
        
        // error if bad number of elements and no file error
        if (m_iCurSize != m_iBufSize) {
            if ((m_iCurSize == 0) || !feof(m_fIn)) { 
                // m_iCurPos = -1;
                bOK = false;
            } 
        } 
    }

    char *piC = NULL;
    //    if (m_iCurPos >= 0) {
    if (bOK) {
        piC = m_pBuffer+m_iCurPos;
        m_iCurPos++;
    }
    return piC;
}


//----------------------------------------------------------------------------
// getBlock
//
int BufReader::getBlock(char *p, uint iSize) {
    int iResult =0;
    
    for (uint i = 0; (i < iSize) && (iResult == 0); i++) {
        char *pC =getNextChar();
        if (pC != NULL) {
            p[i] = *pC;
        } else {
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getBlock
//
int BufReader::getBlock(char *p, uint *piSize, const char *pLim) {
    int iResult =0;
    bool bSearching = true;;
    uint iComPos = 0;
    uint iComMax = (uint)strlen(pLim);
    char *pC = NULL;
    uint i = 0;
    while (bSearching &&(i < *piSize) && (iResult == 0)) {
        pC = getNextChar();
        p[i++] = *pC;
        if (pC != NULL) {
            if (*pC == pLim[iComPos]) {
                iComPos++;
                if (iComPos >= iComMax) {
                    bSearching = false;
                }
            } else if (iComPos > 0) {
	    	if (*pC == *pLim) {
                    iComPos = 1;
                } else {
                    iComPos = 0;
                }
            }
        } else {
            // buffer finished before string was found
            iResult = -1;
	}
    }
    *piSize=i;
    // if no error occurred and string has not been found, return 1
    iResult = (bSearching && (iResult == 0))?1:iResult;
    return iResult;
}


//----------------------------------------------------------------------------
// skip
//
char *BufReader::skip(uint iSkip) {
    
    int iJump = iSkip - (m_iCurSize-m_iCurPos-1);
    if (iJump > 0) {
        fseek(m_fIn, iJump-1, SEEK_CUR);

        m_lBlockPos = ftell(m_fIn);
        m_iCurSize = (uint)fread(m_pBuffer, 1, m_iBufSize, m_fIn);
        m_iCurPos = 0;
        
        // error if bad number of elements and no file error
        if (m_iCurSize != m_iBufSize) {
            if ((m_iCurSize == 0) || !feof(m_fIn)) {
                m_iCurPos = -1;
            } 
        } 
    } else {
        m_iCurPos += iSkip;
    }

    return NULL;
}
