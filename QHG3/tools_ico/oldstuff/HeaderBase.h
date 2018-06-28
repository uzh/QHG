#ifndef __HEADERBASE_H__
#define __HEADERBASE_H__

#include "types.h"
class LineReader;

#define HDRKEY_VERSION   "VERSION"
#define HDRKEY_GRID      "GRID"
#define HDRKEY_COORD     "COORD"
#define HDRKEY_PROJT     "PROJT"
#define HDRKEY_PROJG     "PROJG"
#define HDRKEY_TIME      "TIME"
#define HDRKEY_DATADESC  "DATADESC"
#define HDRKEY_STORAGE   "STORAGE"
#define HDRKEY_ICOFILE   "ICOFILE"
#define HDRKEY_HEADEREND "HEADER_END"

#define BIT_VERSION    0x00000001
#define BIT_GRID       0x00000002
#define BIT_COORD      0x00000004
#define BIT_PROJT      0x00000008
#define BIT_PROJG      0x00000010
#define BIT_TIME       0x00000020
#define BIT_DATADESC   0x00000040
#define BIT_STORAGE    0x00000080
#define BIT_ICOFILE    0x00000100


#define HL_MAGIC_BUF   5
#define HL_SHORT_BUF  16
#define HL_MID_BUF    64
#define HL_LONG_BUF  256 // must be larger than the string rep of projt or pojg
#define HL_DESC_BUF  256 // must be larger than the maximum number of data items
#define HL_MESS_BUF  512 // must be larger than the maximum number of data items

#define VAL_BIN "BIN"
#define VAL_ASC "ASC"
#define VAL_PRESEL "PRESEL"
#define VAL_NOSEL  "NOSEL"

#define VAL_GRID  "GRID"
#define VAL_GEO   "GEO"
#define VAL_NODE  "NODE"

#define COORD_GRID  0
#define COORD_GEO   1
#define COORD_NODE  2

class HeaderBase {
public:
    HeaderBase(const char *pMagic);
    HeaderBase(const char *pMagic, const char *pVersion,
               size iCoordType,  int iCurStep, float fCurTime, 
               const char *pDataDesc, bool bBinary,               
               const char *pIcoFile, bool bPreSel);

    virtual ~HeaderBase();

    int read(const char *pDataFile, unsigned long lIgnoreBits=0);
    int read(LineReader *pLR, unsigned long lIgnoreBits=0);
    virtual int processLine(char *pLine);
    virtual int checkMagic(char *pLine)=0;

    int write(FILE *fOut, bool bBinary);
    virtual int writeSpecifics(FILE *fOut)=0;
    virtual void setHeader(const HeaderBase *pHB);
    virtual void setSpecifics(const HeaderBase *pHB)=0;
    virtual void clear();
    size_t calcDataSize();
    unsigned char *serialize(ulong *piSize, unsigned char *pBuffer);
    int deserialize(unsigned char *pSerialized, int iSize);
    virtual unsigned char *serializeSpecifics(unsigned char *pBuffer)=0;
    virtual unsigned char *deserializeSpecifics(unsigned char *pSerialized)=0;
    virtual ulong getHeaderSize();
    void setIcoFile(const char *pIcoFile); 
    void setCoordType(size iNewType);// COORD_GRID or COORD_GEO

    char *getErrMess() { return m_sError; };

    bool isBinary() { return m_bBinary; };
    ulong getAscHeaderSize() { return m_iAscHeaderSize;};
    char     m_sMagic[HL_MAGIC_BUF];
    char     m_sVersion[HL_SHORT_BUF];
    int      m_iStep;
    float    m_fTime;
    char     m_sDataDesc[HL_DESC_BUF];
    ulong    m_iDataSize;
    int      m_iCoordType;
    bool     m_bBinary;

    char     m_sIcoFile[HL_LONG_BUF];
    bool     m_bPreSel;

    ulong    m_iAscHeaderSize;
    char     m_sError[HL_MESS_BUF];

    virtual unsigned long getNeededBits();
    virtual unsigned long getPostUnNeededBits();
    char *showNames(long lBits, char *pBuf);
    virtual const char *getItemName(unsigned int iIndex);
    unsigned long m_lNeeded;

};
















#endif
