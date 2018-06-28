#ifndef __AGORACLE_H__
#define __AGORACLE_H__

#include <stdio.h>
#include <map>
#include <set>

#include "types.h"

class BufReader;
class AncGraphBase;
class RWAncGraph;

typedef std::pair<long, long>       ancrange;
typedef std::map<idtype, ancrange>  ancinfo;
typedef ancinfo::iterator           ancinfo_it;
typedef ancinfo::const_iterator     ancinfo_cit;

typedef struct {
    idtype iIDFirst;
    idset  sLocalIDs;
} blockset;

class AGOracle {
public:
    static AGOracle *createInstance(const char *pAGFile, uint iBlockSize);
    
    ~AGOracle();
    int loadNodes(AncGraphBase *pAGB, idset sNodeIDs);
    int loadNodesPar(AncGraphBase *pAGB, idset sNodeIDs);

    ulong getNumNodes() { return m_iNumNodes;};

    int loadBlock(AncGraphBase *pAGB, idtype iNodeID, idtype *piCurMinID, idtype *piCurMaxID);
    ancinfo &getList() {return m_mList;};
protected:
    AGOracle();
    int init(const char *pAGFile, uint iBlockSize);
    idtype scanNextNode();

    FILE      *m_fIn;
    BufReader *m_pBR;
    ulong      m_iNumNodes;
    uint       m_iBlockSize;
    ancinfo    m_mList;

    int            m_iNumThreads;
    int            m_iNumBlocks;
    blockset      *m_asBlockIDs;
    FILE         **m_afIn;
    BufReader    **m_apBR;
    RWAncGraph   **m_apAG;
    int           *m_aiIndexes;
    int           *m_aiCounts;

};


 
#endif
