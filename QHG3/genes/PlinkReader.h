#ifndef __PLINKREADER_H__
#define __PLINKREADER_H__

#include "types.h"
#include "LineReader.h"

#include "SequenceProvider.h"
#include "IDSampler2.h"
#include "IDSample.h"

typedef std::map<idtype, ulong*> genomelist;
typedef std::map<idtype, std::string> id2string;
typedef std::map<std::string, idtype> string2id;

class PlinkReader : public SequenceProvider<ulong> {
public:
    static PlinkReader *createInstance(const char *pPlinkFile, int iGenomeSize);
    virtual ~PlinkReader();

    virtual int getSequenceSize() {return m_iGenomeSize;};
    virtual const ulong *getSequence(idtype iID);
    
    int init(const char *pPlinkFile);
    
    int readGenomes();
    int writeBin(const char *pOutput);
protected:
    idtype      m_iCurID;
    int         m_iGenomeSize;
    genomelist  m_mGenomeList;   // qhg id -> genome
    id2string   m_mID2Plink;     // qhg id -> plink id
    id2string   m_mPlinkLocs;    // qhg id -> plink family
    string2id   m_mPlink2ID;     // plink id -> qhg id
    LineReader *m_pLR;
    IDSample   *m_pIDSample;
    loc_data    m_mLocData;

    PlinkReader(int iGenomeSize);
    idtype registerID(std::string sPlinkID);
};

#endif
