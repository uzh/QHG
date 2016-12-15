#ifndef __POPLOADER_H__
#define __POPLOADDER_H__

#include <map>
#include "types.h"
#include "PopHeader.h"

class LineReader;


typedef struct {
    char  sVersion[32];
    size  iGridX;
    size  iGridY;
    spcid iClass;
    char  sClass[64];
    spcid iSpecies;
    char  sSpecies[64];
    char  sDataDesc[256]; // sequence of type chars ('b', 's', 'i', 'l', 'f', 'f')
    bool  bBinary;
} popheaderdata;


typedef std::map<long,  std::vector<std::pair<size_t, unsigned char *> > > tLocalizedData;


class PopLoader {
public:
    static PopLoader *createInstance(const char *pPopFile);
    static PopLoader *createInstance();
    ~PopLoader();

    int processHeader();

    spcid getSpecies() const { return m_pPH->m_iSpecies; };
    const char *getSpeciesName() const { return  m_pPH->m_sSpecies; };
    size_t getDataSize() const { return m_iDataSize; };
    tLocalizedData &getData() { return m_mvtmp; };
    int readData();
    /*@@    int exportData(AgentDataCollection *pADC);*/
    
    int asc2Data(char *pLine);
    int bin2Data(FILE *fData);
    const char *getError() { return m_sError;};

    void setHeader(PopHeader *pPH) { m_pPH->setHeader(pPH);};
    PopHeader *getHeader() { return m_pPH;};
    unsigned char *serializeHeader(ulong *piSize, unsigned char *pBuffer) { return m_pPH->serialize(piSize, pBuffer);};
    int deserializeHeader(unsigned char *pSerialized, int iSize) { return m_pPH->deserialize(pSerialized, iSize);};


    bool userDeletes() { return (m_pBufferForBinaryRead == NULL); };
    const char *getDataDesc() { return m_pPH->m_sDataDesc; };
    long getFilePos() { return m_pPH->getAscHeaderSize();};
    bool isBinary() { return m_pPH->isBinary();};
protected:
    unsigned char *item2AscFile(FILE *fOut, unsigned char *p0, char **ppDesc);

    PopLoader();
    int setFile(const char *pPopFile);

    size_t calcSize();
    size_t      m_iDataSize;

    LineReader *m_pLR;
    PopHeader  *m_pPH;
    tLocalizedData  m_mvtmp;
    std::string m_sFileName;
    char        m_sError[512];
    unsigned char *m_pBufferForBinaryRead;

};

#endif
