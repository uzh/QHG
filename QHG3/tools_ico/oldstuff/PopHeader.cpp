#include <stdio.h>
#include <string.h>
#include "types.h"
#include "ids.h"
#include "strutils.h"
#include "LineReader.h"
#include "HeaderBase.h"
#include "PopHeader.h"

//-----------------------------------------------------------------------------
// constructor
//
PopHeader::PopHeader()
    : HeaderBase(MAGIC_POP) {
    
    clear();

}

//-----------------------------------------------------------------------------
// constructor
//
PopHeader::PopHeader(char *pVersion,
                     size iCoordType, int iCurStep, float fCurTime, 
                     const char *pDataDesc, bool bBinary,
                     const char *pIcoFile, bool bPreSel,
                     spcid iClass, char *pClass, 
                     spcid iSpecies, char *pSpecies) 
    : HeaderBase(MAGIC_POP, pVersion,
                 iCoordType, iCurStep, fCurTime,
                 pDataDesc, true,
                 pIcoFile, bPreSel),
      m_iClass(iClass),
      m_iSpecies(iSpecies) {
      
    if (pClass != NULL) {
        safeStrCpy(m_sClass, pClass, PH_MID_BUF);
    } else {
        bzero(m_sClass, PH_MID_BUF);
    }

    if (pSpecies != NULL) {
        safeStrCpy(m_sSpecies, pSpecies, PH_MID_BUF);
    } else {
        bzero(m_sSpecies, PH_MID_BUF);
    }


    calcDataSize();
}

//-----------------------------------------------------------------------------
// processLine
//
int PopHeader::processLine(char *pLine) {
    char *pCtx;
    char sTest[HL_MID_BUF];
    char sLineOrig[PH_LONG_BUF];

    strcpy(sLineOrig, pLine);

    int iResult = HeaderBase::processLine(pLine);
    if (iResult < 0) {
        if (strstr(pLine, POPKEY_CLASS) == pLine) {
            iResult = -1;
            pLine += strlen(POPKEY_CLASS);
            safeStrCpy(sTest, trim(pLine), PH_MID_BUF);
            
            char *p0 = strtok_r(sTest, " \t", &pCtx);
            if (p0 != NULL) {
                if (strToNum(trim(p0), &(m_iClass))) {
                    p0 = strtok_r(NULL, " \t", &pCtx);
                    safeStrCpy(m_sClass, trim(p0), PH_MID_BUF);
                    m_lNeeded &=~ BIT_CLASS;
                    iResult = 0;
                } else {
                    sprintf(m_sError, "[PopHeader::processLine] couldn't convert string to number [%s]\n", sLineOrig);
                }
            }
            if (iResult != 0) {
                sprintf(m_sError, "[PopHeader::processLine] error in class [%s]\n", sLineOrig);
            } else {
                printf("[PopHeader::processLine] class %d [%s]\n", m_iClass, m_sClass);
            }
        } else if (strstr(pLine, POPKEY_SPECIES) == pLine) {
            iResult = -1;
            pLine += strlen(POPKEY_SPECIES);
            safeStrCpy(sTest, pLine, PH_MID_BUF);
            
            char *p0 = strtok_r(sTest, " \t", &pCtx);
            if (p0 != NULL) {
                if (strToNum(trim(p0), &(m_iSpecies))) {
                    p0 = strtok_r(NULL, " \t", &pCtx);
                    if (p0 != NULL) {
                        safeStrCpy(m_sSpecies, trim(p0), PH_MID_BUF);
                        m_lNeeded &=~ BIT_SPECIES;
                        iResult = 0;
                    } else {
                        sprintf(m_sError, "[PopHeader::processLine] invalid SPECIES line [%s]\n", sLineOrig);
                    }
                }
            }
            if (iResult != 0) {
                sprintf(m_sError, "[PopHeader::processLine] error in species [%s]\n", sLineOrig);
            } else {
                printf("[PopHeader::processLine] species %d [%s]\n", m_iSpecies, m_sSpecies);
            }

        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// clear
//
void PopHeader::clear() {
    HeaderBase::clear();
    m_iClass = CLASS_NONE;
    bzero(m_sClass, PH_MID_BUF);
    m_iSpecies = SPC_NONE;
    bzero(m_sSpecies, PH_MID_BUF);
    *m_sError='\0';
}


//-----------------------------------------------------------------------------
// getHeaderSize
//  get size of header when serialized
//
ulong PopHeader::getHeaderSize() {
    return HeaderBase::getHeaderSize() + 
        strlen(m_sClass)+strlen(m_sSpecies)+ 2 +  //2: terminating '\0'
        2*sizeof(size_t)+2*sizeof(spcid);
}

//-----------------------------------------------------------------------------
// serializeSpecifics
//
unsigned char *PopHeader::serializeSpecifics(unsigned char *pBuffer) {

    unsigned char *p = pBuffer;
    
    p = putMem(p, &m_iClass, sizeof(spcid));
    size_t iS = strlen(m_sClass)+1;
    p = putMem(p, &iS, sizeof(size_t));
    p = putMem(p, m_sClass, iS);
    p = putMem(p, &m_iSpecies, sizeof(spcid));
    iS = strlen(m_sSpecies)+1;
    p = putMem(p, &iS, sizeof(size_t));
    p = putMem(p, m_sSpecies, iS);
 
    return p;         
}

//-----------------------------------------------------------------------------
// deserializeSpecifics
//   we keep the buffer pSerialized and use pointers into it 
//
unsigned char *PopHeader::deserializeSpecifics(unsigned char *pSerialized) {
    
    unsigned char *p = pSerialized;
    size_t iS;
    p = getMem(&m_iClass, p, sizeof(spcid));
    p = getMem(&iS, p, sizeof(size_t));
    safeStrCpy(m_sClass, (char *)p, PH_MID_BUF); p += iS;
    p = getMem(&m_iSpecies, p, sizeof(spcid));
    p = getMem(&iS, p, sizeof(size_t));
    safeStrCpy(m_sSpecies, (char *)p, PH_MID_BUF); p += iS;

    return p;
}

//-----------------------------------------------------------------------------
// checkMagic
//
int PopHeader::checkMagic(char *pLine) {
    return strcmp(pLine, MAGIC_POP);
}

//-----------------------------------------------------------------------------
// writeSpecifics
//
int PopHeader::writeSpecifics(FILE *fOut) {
    int iWritten = 1;
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %d %s\n", POPKEY_CLASS,    m_iClass, m_sClass);
    }
    if (iWritten > 0) {
        iWritten = fprintf(fOut, "%s %d %s\n", POPKEY_SPECIES,  m_iSpecies, m_sSpecies);
    }
    return iWritten;
}


//-----------------------------------------------------------------------------
// setSpecifics
//
void PopHeader::setSpecifics(const HeaderBase *pHB) {
    const PopHeader *pPH = static_cast<const PopHeader *>(pHB);
    safeStrCpy(m_sClass, pPH->m_sClass, HL_MID_BUF);
    m_iSpecies = pPH->m_iClass;
    safeStrCpy(m_sSpecies, pPH->m_sSpecies, HL_MID_BUF);
    m_iSpecies = pPH->m_iSpecies;
    calcDataSize();
}

//-----------------------------------------------------------------------------
// getNeededBits
//   returns a bit array with required elements
//
unsigned long PopHeader::getNeededBits() {
    return HeaderBase::getNeededBits() |
        BIT_CLASS    |
        BIT_SPECIES;
}
const char *PH_NAMES[] = {
    POPKEY_CLASS" ",    
    POPKEY_SPECIES" ",
};


//-----------------------------------------------------------------------------
// getItemName
//   returns a name corresponding to the given index
//
const char *PopHeader::getItemName(unsigned int iIndex) {
    const char *p = HeaderBase::getItemName(iIndex);
    if (*p == '\0') {
        iIndex -= BITPOS_POP; //1+number of headerbase items 
        if (iIndex < sizeof(PH_NAMES)/sizeof(char *)) {
            p = PH_NAMES[iIndex];
        }
    }
    return p;
}
    

//-----------------------------------------------------------------------------
// getPostUnNeededBits
//   returns a bit array with required elements
//
unsigned long PopHeader::getPostUnNeededBits() {
    return HeaderBase::getPostUnNeededBits();
}

