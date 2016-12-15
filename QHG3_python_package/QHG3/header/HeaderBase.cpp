#include <stdio.h>
#include <string.h>

#include "ids.h"
#include "LineReader.h"
#include "DescReader.h"
#include "strutils.h"
#include "HeaderBase.h"

//-----------------------------------------------------------------------------
// constructor
//
HeaderBase::HeaderBase(const char *pMagic,
                       const char *pVersion, 
                       size iCoordType, int iCurStep, float fCurTime, 
                       const char *pDataDesc, bool bBinary,  
                       const char *pIcoFile, bool bPreSel) 
    : m_iStep(iCurStep),
      m_fTime(fCurTime),
      m_iCoordType(iCoordType),
      m_bBinary(bBinary),
      m_bPreSel(bPreSel) {

    if (pVersion != NULL) {
        safeStrCpy(m_sVersion, pVersion, HL_SHORT_BUF);
    } else {
        bzero(m_sVersion, HL_SHORT_BUF);
    }
    if (pDataDesc != NULL) {
        safeStrCpy(m_sDataDesc, pDataDesc, HL_DESC_BUF);
    } else {
        bzero(m_sDataDesc, HL_DESC_BUF);
    }
    if (pMagic != NULL) {
        safeStrCpy(m_sMagic, pMagic, HL_MAGIC_BUF);
    } else {
        bzero(m_sMagic, HL_MAGIC_BUF);
    }
    if (pIcoFile != NULL) {
        safeStrCpy(m_sIcoFile, pIcoFile, HL_LONG_BUF);
    } else {
        bzero(m_sIcoFile, HL_LONG_BUF);
    }

    calcDataSize();
}

//-----------------------------------------------------------------------------
// constructor
//
HeaderBase::HeaderBase(const char *pMagic)
    : m_iStep(-1),
      m_fTime(-1),
      m_bBinary(false),
      m_bPreSel(false) {

    
    clear();
    if (pMagic != NULL) {
        safeStrCpy(m_sMagic, pMagic, HL_MAGIC_BUF);
    } else {
        bzero(m_sMagic, HL_MAGIC_BUF);
    }

}

//-----------------------------------------------------------------------------
// destructor
//
HeaderBase::~HeaderBase() {
}

//-----------------------------------------------------------------------------
// setIcoFile
//
void HeaderBase::setIcoFile(const char *pIcoFile) {
    safeStrCpy(m_sIcoFile, pIcoFile, HL_LONG_BUF);
}


//-----------------------------------------------------------------------------
// clear
//
void HeaderBase::clear() {
    bzero(m_sVersion, HL_SHORT_BUF);
    m_iCoordType = COORD_GRID;
    m_iStep = -1;
    m_fTime = -1;
    bzero(m_sDataDesc, HL_LONG_BUF);
    m_iDataSize = 0;
    m_iAscHeaderSize = 0;
    bzero(m_sError, HL_MAGIC_BUF);
    bzero(m_sIcoFile, HL_LONG_BUF);

}

//-----------------------------------------------------------------------------
// writeBase
//
int HeaderBase::write(FILE *fOut, bool bBinary) {
    int iWritten = 1;
    if (iWritten > 0) {
       iWritten = fprintf(fOut, "%s\n",        m_sMagic);
    }
    if (iWritten > 0) {
       iWritten = fprintf(fOut, "%s %s\n",     HDRKEY_VERSION,  m_sVersion);
    }
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %s\n",    HDRKEY_COORD,    (m_iCoordType == COORD_NODE)?VAL_NODE:((m_iCoordType == COORD_GEO)?VAL_GEO:VAL_GRID));
    }
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %f %d\n", HDRKEY_TIME,     m_fTime, m_iStep);
    }
    if (iWritten > 0) {
       iWritten = fprintf(fOut, "%s %s\n",     HDRKEY_DATADESC, m_sDataDesc);
    }
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %s\n",    HDRKEY_STORAGE,  bBinary?VAL_BIN:VAL_ASC);
    }
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %s %s\n",  HDRKEY_ICOFILE,  m_sIcoFile,  (m_bPreSel)?VAL_PRESEL:VAL_NOSEL);
    }

    if (iWritten > 0) {
        iWritten = writeSpecifics(fOut);
    }
    
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s\n",       HDRKEY_HEADEREND);
    }

    return (iWritten > 0)?0:-1;
}

//-----------------------------------------------------------------------------
// getHeaderSize
//  get size of header when serialized
//
ulong HeaderBase::getHeaderSize() {
    return strlen(m_sVersion)+strlen(m_sDataDesc)+
        + strlen(m_sMagic) + strlen(m_sIcoFile)+ 4 +  //4: terminating '\0'
        sizeof(int)+sizeof(size)+sizeof(float) +
        3*sizeof(size_t) +
        sizeof(bool);
}


//-----------------------------------------------------------------------------
// serialize
//
unsigned char *HeaderBase::serialize(ulong *piSize, unsigned char *pBuffer) {
    *piSize = getHeaderSize();
    if (pBuffer == NULL) {
        pBuffer = new unsigned char[*piSize];
    }

    unsigned char *p = pBuffer;
    
    p = putMem(p, m_sMagic, HL_MAGIC_BUF);
    size_t iS = strlen(m_sVersion)+1;
    p = putMem(p, &iS, sizeof(size_t));
    p = putMem(p, m_sVersion, iS);
    p = putMem(p, &m_iCoordType, sizeof(size));
    p = putMem(p, &m_iStep, sizeof(int));
    p = putMem(p, &m_fTime, sizeof(float));
    iS = strlen(m_sDataDesc)+1;
    p = putMem(p, &iS, sizeof(size_t));
    p = putMem(p, m_sDataDesc, iS);
    iS = strlen(m_sIcoFile)+1;
    p = putMem(p, &iS, sizeof(size_t));
    p = putMem(p, m_sIcoFile, iS);
    p = putMem(p, &m_bPreSel, sizeof(bool));

    //    printf("Check: size calc:%d, size real: %d\n", *piSize, p - pBuffer);
    p = serializeSpecifics(p);
    return pBuffer;         
}

//-----------------------------------------------------------------------------
// deserializeHeader
//   we keep the buffer pSerialized and use pointers into it 
//
int HeaderBase::deserialize(unsigned char *pSerialized, int iSize) {
    int iResult=0;
    
    printf("got serialized %p, L:%d\n", pSerialized, iSize);
    unsigned char *p = pSerialized;
    size_t iS;

    safeStrCpy(m_sMagic, (char *)p, HL_MAGIC_BUF); p += HL_MAGIC_BUF;
    p = getMem(&iS, p, sizeof(size_t));
    safeStrCpy(m_sVersion, (char *)p, HL_SHORT_BUF); p += iS;
    p = getMem(&m_iCoordType, p, sizeof(size));
    p = getMem(&m_iStep, p, sizeof(int));
    p = getMem(&m_fTime, p, sizeof(float));
    iS = strlen(m_sDataDesc)+1;
    p = getMem(&iS, p, sizeof(size_t));
    safeStrCpy(m_sDataDesc, (char *)p, HL_DESC_BUF); p += iS;
    p = getMem(&iS, p, sizeof(size_t));
    safeStrCpy(m_sIcoFile, (char *)p, HL_LONG_BUF); p += iS;
    p = getMem(&m_bPreSel, p, sizeof(bool));

    p = deserializeSpecifics(p);

    calcDataSize();
    return iResult;
}

//-----------------------------------------------------------------------------
// calcDataSize
//  calculate the size of an agent's data (without coords)
//
size_t HeaderBase::calcDataSize() {
    m_iDataSize = DescReader::calcDataSize(m_sDataDesc);
    //        printf("DataDesc is [%s] -> ItemSize : %d\n", m_sDataDesc, m_iDataSize);
    return m_iDataSize;
}




//-----------------------------------------------------------------------------
// read
//
int HeaderBase::read(const char *pDataFile, unsigned long lIgnoreBits) {
    int iResult = -1;
    printf("[HeaderBase::read] opening data file for reading [%s]\n", pDataFile);
    LineReader *pLR = LineReader_std::createInstance(pDataFile, "rt");
    if (pLR != NULL) {
        clear();
        iResult = read(pLR, lIgnoreBits);
        printf("[HeaderBase::read] finished reading [%s] with res %d\n", pDataFile,iResult);
    } else {
        sprintf(m_sError, "[HeaderBase::read] couldn't open data file for reading [%s]\n", pDataFile);
    }  
    delete pLR;
    return iResult;
}


//-----------------------------------------------------------------------------
// read
//   loads header from newpopfile
//  Format:
// File           ::=  <header><data>
// header         ::=  <magicline><headerline>*
// magicline      ::=  "WRLD" <NL>
// headerline     ::=  <versionline> | <storageline> | 
//                     <timeline> | <datadescline> | <icoline>
// versionline    ::= "VERSION" <versionstring> <NL>
// timeline       ::= "TIME" <time> <step_no> <NL>
// storageline    ::= "STORAGE" <storagetype>
// datadescline   ::= "DATADESC" <datadescstring> <NL>
// icoline        ::= "ICOFILE" <icofile>  <preseltype> <NL>
//
// datadescstring ::= ("c"| "s" | "d" | "l" | "f" | "d")*
// storagetype    ::= "BIN"| "ASC"
// preseltype     ::= "PRESEL" | "NOSEL"
////
int HeaderBase::read(LineReader *pLR, unsigned long lIgnoreBits) {
    int iResult = 0;
    bool bGoOn = true;
    m_lNeeded = getNeededBits();
    m_lNeeded = m_lNeeded & ~lIgnoreBits;
    printf("Needed to begin with: 0x%08lx\n", (unsigned long)m_lNeeded);
    if (pLR != NULL) {
        char *pLine = trim(pLR->getNextLine());
        if (checkMagic(pLine) == 0) {

            while (bGoOn && !pLR->isEoF()) {
                pLine = pLR->getNextLine();
                pLine=trim(pLine);
                //                pLine = trim(pLR->getNextLine());
                iResult = processLine(pLine);
                if (iResult != 0) {
                    bGoOn = false;
                    if (iResult > 0) {
                        iResult =  0;
                    } else {
                        if (*m_sError == '\0') {
                            iResult = 0;
                            sprintf(m_sError, "[HeaderBase::read] unknown key [%s]", pLine);
                        }
                    }

                }
            }
        } else {
            sprintf(m_sError, "[HeaderBase::read] bad magic string!");
            iResult = -1;
        }

    } else {
        sprintf(m_sError, "[HeaderBase::read] line reader was not successfully initialized!");
        iResult = -1;
    }

    if (iResult == 0) {
        m_lNeeded &=~ getPostUnNeededBits();
        if (m_lNeeded != 0) {
            char sNames[1024];
            *sNames = '\0';
            sprintf(m_sError, "[HeaderBase::read] missing entries: %s", showNames(m_lNeeded, sNames));
            iResult = -1;
        } else {

            calcDataSize();
        //        printf("[HeaderBase::read] datadesc:[%s]->%d\n", m_sDataDesc, m_iDataSize);

            m_iAscHeaderSize = pLR->tell();
        }
    }
  

    return iResult;

}

//-----------------------------------------------------------------------------
// processLine
//
int HeaderBase::processLine(char *pLine) {
    int iResult = 0;
    char *pCtx;
    char sTest[HL_LONG_BUF];

    if (strstr(pLine, HDRKEY_VERSION) == pLine) {
        pLine += strlen(HDRKEY_VERSION);
        safeStrCpy(m_sVersion, trim(pLine), HL_SHORT_BUF);
        m_lNeeded &=~ BIT_VERSION;

    } else if (strstr(pLine, HDRKEY_GRID) == pLine) {
        iResult = -1;
        pLine += strlen(HDRKEY_GRID);
        char *p0 = strchr(pLine, 'x');
        if (p0 != NULL) {
            *p0 = '\0';
            p0++;
            int iW;
            int iH;
            if (strToNum(pLine, &iW)) {
                if (strToNum(p0, &iH)) {
                    iResult = 0;
                    m_lNeeded &=~ BIT_GRID;
                } else {
                    sprintf(m_sError, "[HeaderBase::readHeader] couldn't convert string to number [%s]\n", p0);
                }
            } else {
                sprintf(m_sError, "[HeaderBase::readHeader] couldn't convert string to number [%s]\n", pLine);
            }
        }
        if (iResult != 0) {
            printf(m_sError, "[HeaderBase::readHeader] error in GRID specification [%s]\n", pLine);
        }

    } else if (strstr(pLine, HDRKEY_TIME) == pLine) {
        pLine += strlen(HDRKEY_TIME);
        safeStrCpy(sTest, pLine, HL_MID_BUF);
        char *p0 = strtok_r(sTest, " \t", &pCtx);
        if (p0 != NULL) {
            if (strToNum(trim(p0), &(m_fTime))) {
                p0 = strtok_r(NULL, " \t", &pCtx);
                if (p0 != NULL) {
                    if (strToNum(trim(p0), &m_iStep)) {
                        m_lNeeded &=~ BIT_TIME;
                        iResult = 0;
                    }
                }
            }
        }
        if (iResult != 0) {
            sprintf(m_sError, "[HeaderBase::processLine] error in time [%s]\n", pLine);
        }
    } else if (strstr(pLine, HDRKEY_DATADESC) == pLine) {
        pLine += strlen(HDRKEY_DATADESC);
        safeStrCpy(m_sDataDesc, trim(pLine), HL_DESC_BUF);
        m_lNeeded &=~ BIT_DATADESC;

    } else if (strstr(pLine, HDRKEY_STORAGE) == pLine) {
        pLine += strlen(HDRKEY_STORAGE);
        pLine = trim(pLine);
        if (strncasecmp(pLine, VAL_BIN, strlen(VAL_BIN)) == 0) {
            m_lNeeded &=~ BIT_STORAGE;
            m_bBinary = true;
        } else {
            m_lNeeded &=~ BIT_STORAGE;
            m_bBinary = false;
        }

    } else if (strstr(pLine, HDRKEY_ICOFILE) == pLine) {
        iResult = 0;
        printf("line is [%s]\n", pLine);
        pLine += strlen(HDRKEY_ICOFILE);
        pLine = trim(pLine);
        safeStrCpy(sTest, pLine, HL_LONG_BUF);
        char *p0 = strtok_r(sTest, " \t", &pCtx);
        
        if (p0 != NULL) {
            printf("first  is [%s]\n", pLine);
        
            safeStrCpy(m_sIcoFile, p0, HL_LONG_BUF);
            
            p0 = strtok_r(NULL, " \t", &pCtx);
            if (p0 != NULL) {
                if (strncasecmp(p0, VAL_PRESEL, 3) == 0) {
                    m_lNeeded &=~ BIT_ICOFILE;
                    m_bPreSel = true;
                } else if (strncasecmp(p0, VAL_NOSEL, 3) == 0){
                    m_lNeeded &=~ BIT_ICOFILE;
                    m_bPreSel = false;
                } else {
                    sprintf(m_sError, "[HeaderBase::processLine] no ico file name or presel settting");
                    iResult = 0*-1;
                }
                if (iResult == 0) {
                    printf("[HeaderBase::processLine] icofile:[%s], presel:%s\n", m_sIcoFile, m_bPreSel?"on":"off");
                }
            } else {
                iResult = 0*-1;
                sprintf(m_sError, "[HeaderBase::processLine] no presel setting");
            }
            
        } else {
            iResult = 0*-1;
            sprintf(m_sError, "[HeaderBase::processLine] no ico file name given [%s]", pLine);
        }
    } else if (strstr(pLine, HDRKEY_COORD) == pLine) {
        iResult = 0;
        pLine += strlen(HDRKEY_COORD);
        pLine = trim(pLine);
        if (strncasecmp(pLine, VAL_GEO, 3) == 0) {
            m_iCoordType = COORD_GEO;
        } else if (strncasecmp(pLine, VAL_GRID, 3) == 0) {
            m_iCoordType = COORD_GRID;
        } else if (strncasecmp(pLine, VAL_NODE, 3) == 0) {
            m_iCoordType = COORD_NODE;
        } else {
            iResult = -1;
            sprintf(m_sError, "[PopHeader::processLine] error in coord type [%s]\n", pLine);
        }
        
        if (iResult == 0) {
            m_lNeeded &=~ BIT_COORD;
        }
    } else if (strstr(pLine, HDRKEY_HEADEREND) == pLine) {
        iResult = 1;
        
// some legacy 
    } else if (strstr(pLine, HDRKEY_PROJT) == pLine) {
        m_lNeeded &=~ BIT_PROJT;
        // ignore without error
        iResult = 0;
    } else if (strstr(pLine, HDRKEY_PROJG) == pLine) {
        m_lNeeded &=~ BIT_PROJG;
        // ignore without error
        iResult = 0;

    } else {
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// setHeader
//
void HeaderBase::setHeader(const HeaderBase *pHB) {
    safeStrCpy(m_sVersion, pHB->m_sVersion, HL_SHORT_BUF);
    safeStrCpy(m_sDataDesc, pHB->m_sDataDesc, HL_MID_BUF);
    m_iCoordType = pHB->m_iCoordType;
    m_iStep = pHB->m_iStep;
    m_fTime = pHB->m_fTime;
    m_bBinary = pHB->m_bBinary;
    safeStrCpy(m_sIcoFile, pHB->m_sIcoFile, HL_LONG_BUF);
    m_bPreSel = pHB->m_bPreSel;

    setSpecifics(pHB);
}



void HeaderBase::setCoordType(size iNewType) {
    m_iCoordType = iNewType; 
}

/*
//-----------------------------------------------------------------------------
// createGeoProvider
//
GeoProvider *HeaderBase::createGeoProvider(HeaderBase *pPH) {
    int iResult = -1;
    GeoProvider *pGP = NULL;
    // create a GeoProvider from the header in the data
    ProjType *pPT = new ProjType();
    iResult = pPT->fromString(pPH->m_sProjType, true);
    if (iResult == 0) {
        ProjGrid *pPG = new ProjGrid();
        iResult = pPG->fromString(pPH->m_sProjGrid);
        if (iResult == 0) {
            printf("PT: %s\n", pPT->toString(true));
            printf("PG: %s\n", pPG->toString());
            Projector *pr = GeoInfo::instance()->createProjector(pPT);
            pGP = new PrGeoProvider(pPG, pr);
        } else {
            printf("[PopReader::createGeoProvider]Couldn't convert string [%s] to ProjGrid\n", pPH->m_sProjGrid);
        }
    } else {
        printf("[PopReader::createGeoProvider]Couldn't convert string [%s] to ProjType\n", pPH->m_sProjType);
    }
    return pGP;

}


//-----------------------------------------------------------------------------
// createGridProjection
//
GridProjection *HeaderBase::createGridProjection(HeaderBase *pPH) {
    int iResult = -1;
    GridProjection *pGP = NULL;
    // create a GeoProvider from the header in the data
    ProjType *pPT = new ProjType();
    iResult = pPT->fromString(pPH->m_sProjType, true);
    if (iResult == 0) {
        ProjGrid *pPG = new ProjGrid();
        iResult = pPG->fromString(pPH->m_sProjGrid);
        if (iResult == 0) {
            printf("PT: %s\n", pPT->toString(true));
            printf("PG: %s\n", pPG->toString());
            Projector *pr = GeoInfo::instance()->createProjector(pPT);
            pGP = new GridProjection(pPG, pr, true);
 
        } else {
            printf("[PopReader::createGeoProvider]Couldn't convert string [%s] to ProjGrid\n", pPH->m_sProjGrid);
        }
    } else {
        printf("[PopReader::createGeoProvider]Couldn't convert string [%s] to ProjType\n", pPH->m_sProjType);
    }
    return pGP;

}

*/
//-----------------------------------------------------------------------------
// getNeededBits
//   returns a bit array with required elements
//
unsigned long HeaderBase::getNeededBits() {
    return  BIT_VERSION   |
        //            BIT_GRID      |
            BIT_COORD     |
        //            BIT_PROJT     |
        //            BIT_PROJG     |
            BIT_TIME      |
            BIT_DATADESC  |
            BIT_STORAGE  |
            BIT_ICOFILE;
}
//-----------------------------------------------------------------------------
// getPostUnNeededBits
//   returns a bit array with required elements
//
unsigned long HeaderBase::getPostUnNeededBits() {
    return  0;
}

const char *HB_NAMES[] = {
    HDRKEY_VERSION" ", 
    HDRKEY_GRID" ",    
    HDRKEY_COORD" ",   
    HDRKEY_PROJT" ",   
    HDRKEY_PROJG" ",   
    HDRKEY_TIME" ",    
    HDRKEY_DATADESC" ",
    HDRKEY_STORAGE" ", 
    HDRKEY_ICOFILE" ",
};

//-----------------------------------------------------------------------------
// getItemName
//   returns a name corresponding to the given index
//
const char *HeaderBase::getItemName(unsigned int iIndex) {
    const char *p = "";
    if (iIndex < sizeof(HB_NAMES)/sizeof(char *)) {
        p = HB_NAMES[iIndex];
    }
    return p;
}

//-----------------------------------------------------------------------------
// showNames
//   returns all names corresponding to the set bits
//
char * HeaderBase::showNames(long lBits, char *pBuf) {
 int iMiss = 0;
 printf("bits to check 0x%08lx\n", (unsigned long)lBits);
    while (lBits != 0) {
        if ((lBits & 0x1) != 0) {
            printf("checking for %d (%s)\n", iMiss, getItemName(iMiss));
            strcat(pBuf, getItemName(iMiss));
        }
        lBits >>= 1;
        iMiss++;
    }
    return pBuf;
}
