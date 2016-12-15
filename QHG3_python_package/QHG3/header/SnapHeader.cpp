#include <stdio.h>
#include <string.h>

#include "types.h"
#include "ids.h"
#include "LineReader.h"
#include "strutils.h"
#include "SnapHeader.h"

//-----------------------------------------------------------------------------
// constructor
//
SnapHeader::SnapHeader(const char *pVersion,
                       size iCoordType, int iCurStep, float fCurTime, 
                       const char *pDataDesc, 
                       const char *pIcoFile, bool bPreSel,
                       int iSelector, const char *pAttrName,
                       int iNumParams, double *pdParams)
    : HeaderBase(MAGIC_SNAP, pVersion,
                 iCoordType, iCurStep, fCurTime,
                 pDataDesc, true, 
                 pIcoFile,  bPreSel),
      m_iSelector(iSelector),
      m_iNumParams(0) {

    if (pAttrName != NULL) {
        safeStrCpy(m_sAttrName, pAttrName, SN_ATTR_BUF);
    } else {
        bzero(m_sAttrName, SN_ATTR_BUF);
    }

    m_iNumParams = (MAX_SNAP_PARAMS<iNumParams)?MAX_SNAP_PARAMS:iNumParams;
    memcpy(m_adParams, pdParams, m_iNumParams);


    calcDataSize();
}

//-----------------------------------------------------------------------------
// constructor
//
SnapHeader::SnapHeader()
    : HeaderBase(MAGIC_SNAP),
      m_iSelector(SEL_NONE),
      m_iNumParams(0) {
    clear();
    m_bBinary = true;
}

//-----------------------------------------------------------------------------
// constructor
//
void SnapHeader::clear() {
    HeaderBase::clear();
    m_iSelector = SEL_NONE;
    bzero(m_sAttrName, SN_ATTR_BUF);
    m_iNumParams = 0;
    bzero(m_adParams, MAX_SNAP_PARAMS*sizeof(double));
}

//-----------------------------------------------------------------------------
// writeHeader
//  return positive number (e.g. number of written characters) if ok
//
int SnapHeader::writeSpecifics(FILE *fOut) {
    int iWritten = 1;
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %d %s %ld\n", SNAPKEY_SELECTOR, m_iSelector, m_sAttrName, m_iDataSize);
    }
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %d", SNAPKEY_PARAMS, m_iNumParams);
        int i = 0;
        while ((iWritten > 0) && ( i < m_iNumParams)) {
            iWritten = fprintf(fOut, " %f",  m_adParams[i++]);
        }
        fprintf(fOut, "\n");
    }
    return iWritten;
}

//-----------------------------------------------------------------------------
// processLine
//
int SnapHeader::processLine(char *pLine) {
    char *pCtx;
    char sTest[HL_LONG_BUF];

    int iResult = HeaderBase::processLine(pLine);
    if (iResult < 0) {
        // base's processLine was not successful: you try now
        iResult = -1;
        if (strstr(pLine, SNAPKEY_SELECTOR) == pLine) {
            iResult = -1;
            pLine += strlen(SNAPKEY_SELECTOR);
            safeStrCpy(sTest, pLine, HL_LONG_BUF);
            
            char *p0 = strtok_r(sTest, " \t", &pCtx);
            if (p0 != NULL) {
                if (strToNum(trim(p0), &m_iSelector)) {
                    p0 = strtok_r(NULL, " \t", &pCtx);
                    if (p0 != NULL) {
                        safeStrCpy(m_sAttrName, trim(p0), SN_ATTR_BUF);
                        p0 = strtok_r(NULL, " \t", &pCtx);
                        iResult = 0;
                        if (p0 != NULL) {
                            iResult = -1;
                            if (strToNum(trim(p0), &m_iDataSize)) {
                                iResult = 0;
                                m_lNeeded &=~ BIT_SELECTOR;
                            }
                        }
                    }
                }
            }
            if (iResult != 0) {
                printf("[SnapHeader::processLine] error in selector [%s]\n", pLine);
            }
        } else if (strstr(pLine, SNAPKEY_PARAMS) == pLine) {
            iResult = -1;
            pLine += strlen(SNAPKEY_PARAMS);
            safeStrCpy(sTest, pLine, HL_LONG_BUF);
            char *p0 = strtok_r(sTest, " \t", &pCtx);
            if (p0 != NULL) {
                int i = 0;
                if (strToNum(trim(p0), &m_iNumParams)) {
                    if (m_iNumParams > MAX_SNAP_PARAMS) {
                        m_iNumParams = MAX_SNAP_PARAMS;
                    }
                    iResult = 0;
                    while ((iResult == 0) && (p0 != NULL)) {
                        p0 = strtok_r(NULL, " \t", &pCtx);
                        if (i < m_iNumParams) {
                            if (strToNum(trim(p0), &(m_adParams[i++]))) {
                            } else {
                                iResult = -1;
                            }
                        }
                    }
                    if (iResult == 0) {
                        m_lNeeded &=~ BIT_PARAMS;
                    }
                }
            }
            if (iResult != 0) {
                printf("[SnapHeader::processLine] error in params [%s]\n", pLine);
            }

        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// getHeaderSize
//  get size of header when serialized
//
ulong SnapHeader::getHeaderSize() {
    return HeaderBase::getHeaderSize() + 
        strlen(m_sAttrName)+ 1 +  //1: terminating '\0'
        2*sizeof(int) +
        2*sizeof(size_t)+m_iNumParams*sizeof(double);


}



//-----------------------------------------------------------------------------
// serializeSpecifics
//
unsigned char *SnapHeader::serializeSpecifics(unsigned char *pBuffer) {
    unsigned char *p = pBuffer;
    
    p = putMem(p, &m_iSelector, sizeof(int));
    size_t iS = strlen(m_sAttrName)+1;
    p = putMem(p, &iS, sizeof(size_t));
    p = putMem(p, m_sAttrName, iS);
    p = putMem(p, &m_iNumParams, sizeof(int));
    p = putMem(p, m_adParams, m_iNumParams*sizeof(double));
     //    printf("Check: size calc:%d, size real: %d\n", *piSize, p - pBuffer);
    return p;         
}

//-----------------------------------------------------------------------------
// deserializeSpecifics
//
unsigned char *SnapHeader::deserializeSpecifics(unsigned char *pSerialized) {
    
    unsigned char *p = pSerialized;
    size_t iS;
    p = getMem(&m_iSelector, p, sizeof(int));
    p = getMem(&iS, p, sizeof(size_t));
    safeStrCpy(m_sAttrName, (char *)p, SN_ATTR_BUF); p += iS;
    p = getMem(&m_iNumParams, p, sizeof(int));
    p = getMem(m_adParams, p, m_iNumParams*sizeof(double));

    return p;
}

//-----------------------------------------------------------------------------
// checkMagic
//
int SnapHeader::checkMagic(char *pLine) {
    return strcmp(pLine, MAGIC_SNAP);
}

void SnapHeader::setSpecifics(const HeaderBase *pHB) {
    const SnapHeader *pSH = static_cast<const SnapHeader *>(pHB);
    m_iSelector = pSH->m_iSelector;
    m_iNumParams = pSH->m_iNumParams;
    safeStrCpy(m_sAttrName, pSH->m_sAttrName, SN_ATTR_BUF);
}


//-----------------------------------------------------------------------------
// getNeededBits
//   returns a bit array with required elements
//
unsigned long SnapHeader::getNeededBits() {
    return HeaderBase::getNeededBits() |
        BIT_SELECTOR  |
        BIT_PARAMS;
}
const char *SH_NAMES[] = {
    SNAPKEY_SELECTOR" ",    
    SNAPKEY_PARAMS" ",
};

//-----------------------------------------------------------------------------
// getItemName
//   returns a name corresponding to the given index
//
const char *SnapHeader::getItemName(unsigned int iIndex) {
    const char *p = HeaderBase::getItemName(iIndex);
    if (*p == '\0') {
        iIndex -= BITPOS_SNAP; //1+number of headerbase items 
        if (iIndex < sizeof(SH_NAMES)/sizeof(char *)) {
            p = SH_NAMES[iIndex];
        }
    }
    return p;
}
    


//-----------------------------------------------------------------------------
// getPostUnNeededBits
//   returns a bit array with required elements
//
unsigned long SnapHeader::getPostUnNeededBits() {
    return HeaderBase::getPostUnNeededBits();
}

