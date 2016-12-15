#ifndef __POPHEADER_H__
#define __POPHEADER_H__

#include "types.h"
#include "HeaderBase.h"
class LineReader;

#define POPKEY_MAGIC     "MAGIC"
#define POPKEY_CLASS     "CLASS"
#define POPKEY_SPECIES   "SPECIES"

#define BITPOS_POP 9
#define BIT_CLASS       0x0000200
#define BIT_SPECIES     0x0000400



#define MAGIC_POP "POP1"

#define PH_SHORT_BUF  16
#define PH_MID_BUF    32
#define PH_LONG_BUF  256 // must be larger than the maximum number of items

class PopHeader : public HeaderBase {
public:
    PopHeader();
    PopHeader(char *pVersion,
              size iCoordType, int iCurStep, float fCurTime, 
              const char *pDataDesc, bool bBinary, 
              const char *pIcoFile, bool bPreSel,
              spcid iClass, char *pClass, 
              spcid iSpecies, char *pSpecies);

    virtual int processLine(char *pLine);
    virtual int checkMagic(char *pLine);

    virtual int writeSpecifics(FILE *fOut);

    virtual void clear();
    virtual unsigned char *serializeSpecifics(unsigned char *pBuffer);
    virtual unsigned char *deserializeSpecifics(unsigned char *pSerialized);
    virtual ulong getHeaderSize();

    /*
    void copy(PopHeader *pPH);
    */
    void setSpecifics(const HeaderBase *pPH);

    ulong getDataSize() { return m_iDataSize;};
    int getSpecies() { return m_iSpecies;};
    char *getSpeciesName() { return m_sSpecies;};


    spcid m_iClass;
    char  m_sClass[PH_MID_BUF];
    spcid m_iSpecies;
    char  m_sSpecies[PH_MID_BUF];

    virtual unsigned long getNeededBits();
    virtual unsigned long getPostUnNeededBits();
    virtual const char *getItemName(unsigned int iIndex);

};


#endif
