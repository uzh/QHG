#ifndef __WORLDHEADER_H__
#define __WORLDHEADER_H__

#include "HeaderBase.h"

#define MAGIC_WORLD        "WRLD"


class WorldHeader : public HeaderBase {
public:
    WorldHeader();
    WorldHeader(const char *pVersion,
                size iCoordType, int iCurStep, float fCurTime, 
                const char *pDataDesc, bool bBinary,  
                const char *pIcoFile, bool bPreSel);

    virtual int writeSpecifics(FILE *fOut);
    virtual int processLine(char *pLine);
    virtual int checkMagic(char *pLine);
    virtual void clear();
    virtual unsigned char *serializeSpecifics(unsigned char *pBuffer);
    virtual unsigned char *deserializeSpecifics(unsigned char *pSerialized);
    virtual ulong getHeaderSize();
    virtual void setSpecifics(const HeaderBase *pHB);
};

#endif
