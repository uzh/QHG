#ifndef __DYNANCGRAPH_H__
#define __DYNANCGRAPH_H__

#include <stdio.h>
#include <map>
#include <vector>
#include <set>

#include "types.h"

#include "AncGraphBase.h"

class BufWriter;
class AncestorNode;
class RGeneration;

typedef ulong filepos;
typedef std::map<idtype, std::vector<idtype> > childmap;
typedef std::map<idtype, filepos> oracle;

class DynAncGraph : public AncGraphBase {
public:    
    static DynAncGraph *createInstance();
    static DynAncGraph *createInstance(const char *pAncData, ulong iBlockSize);
    static DynAncGraph *createInstance(const char *pAncData, ulong iBlockSize, const char *pOracleFile, unsigned char *aChecksum, int iCSLen);
    
    int createGraph(idset &sInitial, const char *pTempName, intset &sSavePoints);
    int createGraph2(idset &sInitial, const char *pTempName, intset &sSavePoints);
    int createGraph3(idset &sInitial, const char *pTempName, intset &sSavePoints);
    int createGraph4(idset &sInitial, const char *pTempName, intset &sSavePoints);
    int createGraph7(idset &sInitial, const char *pTempName, intset &sSavePoints);
   ~DynAncGraph();

    int saveBin(const char *pFileName);
    //    static int saveNode(BufWriter *pBW, AncestorNode *pAN);

    const ancnodelist& getMap(){ return m_mIndex;};
    const idset& getProgenitors(){ return m_sRoots;};

    int calcKillPoints(idset &sInitial, int iType, int iLatency=5);
    int createNodesForIds(const idset &sCur);

    int writeOracle(const char *pOutput, unsigned char *aChecksum, int iL);


    int loadAncs(idset &sItems, std::map<idtype, std::pair<idtype,idtype> > &mAncs);

protected:
    DynAncGraph();
    int init(const char *pAncData, ulong iBlockSize);
    int init(const char *pAncData, ulong iBlockSize, const char *pOracleFile, unsigned char *aChecksum, int iCSLen);

    int loadOracle(const char *pOracleFile, unsigned char *aChecksum, uint iCSLen);
    int createOracle();
    int findLowerBound(int iLow, int iNum, int iResolution, idtype iVal);

    int saveBin(BufWriter *pBW, bool bWriteHeader, bool bWriteNodes, bool bWriteFooter);

    int saveAndDeleteSet(const char *pFileName, int iBufSize, idset &sIDs);
    int saveAndDeleteFragement(const char *pFileBody, int iGen, int iBufSize, const intset &sGenSets);

    int backReach(idset &sInitial);
    int backReach2(idset &sInitial);
    int backReach3(idset &sInitial);
    int backReach4(idset &sInitial, int iLatency);
    int backReach7(idset &sInitial);

    FILE *m_fAnc;
    ulong       m_lFileSize;
    long        m_lListOffset;
    oracle      m_mOracle;

    int         m_iBlockSize;
    idtype     *m_aAncBuf;

    bool m_bKillPoints;
   
    std::vector<idset> m_vGenerations; // m_vGenerations[i]: set of ids of generation i
    std::vector<uint>    m_vBackReach;   // m_vBackReach[i] : earliest generation referenced in generation i
    std::map<int, intset> m_mKillPoints; // m_vKillPoints[i]: set of generation indexes that can be deleted in generation i 

    std::vector<RGeneration *> m_vRGenerations; // m_vGenerations[i]: set of ids of generation i
    bool m_bVer4;
    int m_iBR;
};

#endif
