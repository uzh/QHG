#ifndef __SNAPHEADER_H__
#define __SNAPHEADER_H__

#include "types.h"
#include "HeaderBase.h"

#define MAGIC_SNAP        "ISNP"

#define SNAPKEY_SELECTOR  "SELECTOR"
#define SNAPKEY_PARAMS    "PARAMS"

#define MAX_SNAP_PARAMS  5
#define SN_ATTR_BUF     64

#define BITPOS_SNAP 12
#define BIT_SELECTOR   0x00001000
#define BIT_PARAMS     0x00002000

class SnapHeader : public HeaderBase {
public:
    SnapHeader();
    SnapHeader(const char *pVersion,
               size iCoordType, int iCurStep, float fCurTime, 
               const char *pDataDesc, 
               const char *pIcoFile, bool bPreSel,
               int iSelector, const char *pAttrName,
               int iNumParams, double *pdParams);

    virtual int writeSpecifics(FILE *fOut);
    virtual int processLine(char *pLine);
    virtual int checkMagic(char *pLine);
    virtual void clear();
    virtual unsigned char *serializeSpecifics(unsigned char *pBuffer);
    virtual unsigned char *deserializeSpecifics(unsigned char *pSerialized);
    virtual ulong getHeaderSize();
    void setSpecifics(const HeaderBase *pHB);

    virtual unsigned long getNeededBits();
    virtual unsigned long getPostUnNeededBits();
    virtual const char *getItemName(unsigned int iIndex);


    int      m_iSelector;
    int      m_iNumParams;
    char     m_sAttrName[SN_ATTR_BUF];
    double   m_adParams[MAX_SNAP_PARAMS];

};

#endif
