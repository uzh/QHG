#include <stdio.h>
#include <string.h>

#include "ids.h"
#include "HeaderBase.h"
#include "WorldHeader.h"


//-----------------------------------------------------------------------------
// constructor
//
WorldHeader::WorldHeader(const char *pVersion,
                         size iCoordType, int iCurStep, float fCurTime, 
                         const char *pDataDesc, bool bBinary,  
                         const char *pIcoFile, bool bPreSel) 
    : HeaderBase(MAGIC_WORLD, pVersion, 
                 iCoordType,iCurStep, fCurTime,
                 pDataDesc, bBinary,  
                 pIcoFile, bPreSel) {
}

//-----------------------------------------------------------------------------
// constructor
//
WorldHeader::WorldHeader()
    : HeaderBase(MAGIC_WORLD) {
    clear();
}

//-----------------------------------------------------------------------------
// clear
//
void WorldHeader::clear() {
    HeaderBase::clear();
}

//-----------------------------------------------------------------------------
// writeSpecifics
//  return positive number (e.g. number of written characters) if ok
//
int WorldHeader::writeSpecifics(FILE *fOut) {
    return 1;
}

//-----------------------------------------------------------------------------
// processLine
//
int WorldHeader::processLine(char *pLine) {
    int iResult = HeaderBase::processLine(pLine);
    if (iResult < 0) {
        // base's processLine was not successful: you try now
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// getHeaderSize
//  get size of header when serialized
//
ulong WorldHeader::getHeaderSize() {
    return HeaderBase::getHeaderSize() + 0;
}

//-----------------------------------------------------------------------------
// serializeSpecifics
//
unsigned char *WorldHeader::serializeSpecifics(unsigned char *pBuffer) {
    // generally: char *p = putMem(...)
    return pBuffer;
}

//-----------------------------------------------------------------------------
// deserializeSpecifics
//
unsigned char *WorldHeader::deserializeSpecifics(unsigned char *pSerialized) {
    // generally: char *p = getMem(...)
    return pSerialized;
}

//-----------------------------------------------------------------------------
// checkMagic
//
int WorldHeader::checkMagic(char *pLine) {
    return strcmp(pLine, MAGIC_WORLD);
}

//-----------------------------------------------------------------------------
// setSpecifics
//
void WorldHeader::setSpecifics(const HeaderBase *pHB) {
    // do nothing
}
