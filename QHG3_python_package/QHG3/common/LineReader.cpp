#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "types.h"
#include "strutils.h"
#include "utils.h"
#include "LineReader.h"


/*******************  LineReader *********************************************/

//-----------------------------------------------------------------------------
// constructor
//
LineReader::LineReader(int iLineLen) 
    : m_pLine(NULL),
      m_iLineLen(iLineLen),
      m_bClose(false) {

    if (iLineLen > 0) {
        m_pLine = new char[iLineLen];
    }
}

//-----------------------------------------------------------------------------
// destructor
//
LineReader::~LineReader() {
    if (m_pLine != NULL) {
        delete[] m_pLine;
    }
}


/*******************  LineReader_std *****************************************/


//-----------------------------------------------------------------------------
// createInstance
//
LineReader *LineReader_std::createInstance(const char *pName, const char *pMode, int iLineLen) {
    LineReader *pLR = new LineReader_std(pName, pMode, iLineLen);
    if (!pLR->isReady()) {
        delete pLR;
        pLR = NULL;
    }
    return pLR;
}

//-----------------------------------------------------------------------------
// createInstance
//
LineReader *LineReader_std::createInstance(FILE *fIn, int iLineLen) {
    LineReader *pLR = new LineReader_std(fIn, iLineLen);
    if (!pLR->isReady()) {
        delete pLR;
        pLR = NULL;
    }
    return pLR;
}


//-----------------------------------------------------------------------------
// constructor
//
LineReader_std::LineReader_std(const char *pName, const char *pMode, int iLineLen)
    : LineReader(iLineLen),
      m_fIn(NULL) {
    
    m_fIn = fopen(pName, pMode);
    if (m_fIn == NULL) {
        fprintf(STDERR, "(%d)[LineReader_std::LineReader_std] Error opening file [%s] (%d):%s\n", getpid(), pName, errno, strerror(errno));
    }
    m_bClose = true;

}

//-----------------------------------------------------------------------------
// constructor
//
LineReader_std::LineReader_std(FILE *fIn, int iLineLen)
    : LineReader(iLineLen),
      m_fIn(fIn) {
    
    m_bClose = false;

}

//-----------------------------------------------------------------------------
// constructor
//
LineReader_std::~LineReader_std() {
    if (m_bClose && (m_fIn != NULL)) {
        int iResult = fclose(m_fIn);
        if (iResult != 0) {
            fprintf(STDERR, "(%d)[LineReader_std::~LineReader_std] Error closing file (%d):%s\n", getpid(), errno, strerror(errno));
        }
    }
}

//-----------------------------------------------------------------------------
// isReady
//
bool LineReader_std:: isReady() {
    return (m_fIn != NULL) && (m_pLine != NULL);
}

//-----------------------------------------------------------------------------
// getNextLine
//   ignore comments and blank lines
//
char *LineReader_std::getNextLine(int iMode) {

    char *p = NULL;
    bool bSearch = true;

    do {
        p = fgets(m_pLine, m_iLineLen, m_fIn);
        if (p != NULL) {
            if (iMode & GNL_TRIM) {
                p = trim(p);
            }
            if (*p == '\0') {
                bSearch = (iMode & GNL_IGNORE_BLANKS);
            } else if (*p == '#') {
                bSearch = (iMode & GNL_IGNORE_COMMENTS);
            } else if  (*p == '%') {
                bSearch = (iMode & GNL_IGNORE_MATCOMM);
            } else {
                bSearch = false;
            }
        } else {
            // eof or error
            bSearch = false;
        }
    } while (bSearch);
    return p;
}

//-----------------------------------------------------------------------------
// read
//
uint LineReader_std::read(void *pBuf, uint iSize) {

    return (uint) fread(pBuf, 1, iSize, m_fIn);
}


/*******************  LineReader_gz *****************************************/

//-----------------------------------------------------------------------------
// createInstance
//
LineReader *LineReader_gz::createInstance(const char *pName, const char *pMode, int iLineLen) {
    LineReader *pLR = new LineReader_gz(pName, pMode, iLineLen);
    if (!pLR->isReady()) {
        delete pLR;
        pLR = NULL;
    }
    return pLR;
}

//-----------------------------------------------------------------------------
// constructor
//
LineReader_gz::LineReader_gz(const char *pName, const char *pMode, int iLineLen)
    : LineReader(iLineLen),
      m_fIn(NULL) {

    m_bClose = true;
    m_fIn = gzopen(pName, pMode);
}

//-----------------------------------------------------------------------------
// constructor
//
LineReader_gz::LineReader_gz(gzFile fIn, int iLineLen)
    : LineReader(iLineLen),
      m_fIn(fIn) {

    m_bClose = false;

}

//-----------------------------------------------------------------------------
// destructor
//
LineReader_gz::~LineReader_gz() {
    if (m_bClose && (m_fIn != NULL)) {
        int iResult = gzclose(m_fIn);
        if (iResult != Z_OK) {
            fprintf(STDERR, "(%d)[LineReader_gz::~LineReader_gz] Error closing files (%d)\n", getpid(), iResult);
        }
    }
}


//-----------------------------------------------------------------------------
// isReady
//
bool LineReader_gz:: isReady() {
    return (m_fIn != NULL) && (m_pLine != NULL);
}

//-----------------------------------------------------------------------------
// getNextLine
//   ignore comments and blank lines
//
char *LineReader_gz::getNextLine(int iMode) {

    char *p = NULL;
    bool bSearch = true;
    do {
        p = gzgets(m_fIn, m_pLine, m_iLineLen);
        if (p != NULL) {

            p = trim(p);
            if (*p == '\0') {
                bSearch = (iMode & GNL_IGNORE_BLANKS);
            } else if (*p == '#') {
                bSearch = (iMode & GNL_IGNORE_COMMENTS);
            } else {
                bSearch = false;
            }
        } else {
            // eof or error
            bSearch = false;
        }
    } while (bSearch);
    return p;
}


//-----------------------------------------------------------------------------
// read
//
uint LineReader_gz::read(void *pBuf, uint iSize) {

    return (uint) gzread(m_fIn, pBuf, iSize);
}

/*******************  LineReader_str *****************************************/

//-----------------------------------------------------------------------------
// createInstance
//
LineReader *LineReader_str::createInstance(const char *pLines[], unsigned int iNum, int iLineLen) {
    LineReader *pLR = new LineReader_str(pLines, iNum, iLineLen);
    if (!pLR->isReady()) {
        delete pLR;
        pLR = NULL;
    }
    return pLR;
}

//-----------------------------------------------------------------------------
// createInstance
//
LineReader *LineReader_str::createInstance(const char *pLines, int iLineLen) {
    LineReader *pLR = new LineReader_str(pLines,iLineLen);
    if (!pLR->isReady()) {
        delete pLR;
        pLR = NULL;
    }
    return pLR;
}



//-----------------------------------------------------------------------------
// constructor
//
LineReader_str::LineReader_str(const char *pLines[], unsigned int iNum, int iLineLen)
    : LineReader(iLineLen),
      m_iCurrentIndex(0) {
    for (unsigned int i = 0; i < iNum; i++) {
        addLines(pLines[i]);
    }
}

//-----------------------------------------------------------------------------
// constructor
//
LineReader_str::LineReader_str(const char *pLines, int iLineLen)
    : LineReader(iLineLen),
      m_iCurrentIndex(0) {
    addLines(pLines);
}
  
//-----------------------------------------------------------------------------
// destructor
//
LineReader_str::~LineReader_str() {
    for (unsigned int i = 0; i < m_vMem.size(); i++) {
        if (m_vMem[i] != NULL) {
            free(m_vMem[i]);
        }
    }
}

//-----------------------------------------------------------------------------
// getNextLine
//
char *LineReader_str::getNextLine(int iMode) {
    char *pLine = NULL;
    while ((pLine == NULL) && (m_iCurrentIndex < m_vLines.size())) {
        pLine = trim(m_vLines[m_iCurrentIndex++]);
        if (((iMode & GNL_IGNORE_COMMENTS) && (*pLine == '#')) || ((iMode & GNL_IGNORE_BLANKS) && (*pLine == '\0'))) {
            pLine = NULL;
        }
    }
    if (pLine == NULL) {
        *m_pLine = '\0';
    } else {
        strcpy(m_pLine, pLine);
    }
    return pLine;
}

//-----------------------------------------------------------------------------
// read
//
uint LineReader_str::read(void *pBuf, uint iSize) {
    int iResult = -1;
    // not supported (yet)
    return iResult;
}

//-----------------------------------------------------------------------------
// seek
//
int LineReader_str::seek(long offset, int whence) {
    int iResult = 0;
    uint iNewPos = 0;
    switch(whence) {
    case SEEK_SET:
        iNewPos = (uint) offset;
        break;
    case SEEK_CUR:
        iNewPos = (uint) (m_iCurrentIndex+offset);
        break;
    case SEEK_END:
        iNewPos = (uint) (m_vLines.size()-offset-1);
        break;
    default:
        iResult = -1;
    }
    if (iResult == 0) {
        if ((iNewPos >= 0) && (iNewPos < m_vLines.size())) {
            m_iCurrentIndex = (unsigned int) iNewPos;
        } else {
            iResult = -1;
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// addLines
//
void LineReader_str::addLines(const char *pLines) {
    char *pCtx;
    char *pMem = strdup(pLines);
    char *p = strtok_r(pMem, "\n", &pCtx);
    while (p != NULL) {
        m_vLines.push_back(p);
        p = strtok_r(NULL, "\n", &pCtx);
    }
    m_vMem.push_back(pMem);
}
