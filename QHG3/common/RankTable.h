#ifndef __RANKTABLE_H__
#define __RANKTABLE_H__

#include <map>
#include <vector>

#include "types.h"

class WELL512;
/*
static float s_fPairTime;
static float s_fRankTime;
static float s_fRank1Time;
static float s_fRank2Time;
*/
typedef std::pair<uint, uint> couple;
typedef std::vector<couple> couples;
class RankTable {
public:
    static RankTable *createInstance(uint iNumF, uint iNumM, WELL512 **apWell, float **ppRanks=NULL);
    ~RankTable();
    int makeAllPairings(float fCutOff, bool bPermute);
    const couples &getPairs() { return m_vPaired; };
    int setRank(uint iF, uint iM, float fRank);
    
    void setVerbosity(bool bVerbose) {m_bVerbose = bVerbose;};
    void display();


    float m_fPairTime;
    float m_fRankTime;
    float m_fRank1Time;
    float m_fRank2Time;

protected:
    RankTable(uint iF, uint iM, WELL512 **apWell);
    int init(float **ppRanks);
    void clearCross(uint iF, uint iM);
    int  pair(uint iF, uint iM);
    int  pairHighestRankers(float fCutOff, bool bPermute);
    int  randomPair(uint iF);

    
    uint       m_iNumF;
    uint       m_iNumM;
    WELL512  **m_apWELL;
    float    **m_aaRanks;
    couples    m_vPaired;
    float      m_fCurMax;

    uint  *m_aFPermutation;
    uint  *m_aFIndexes;
    uint  *m_aMIndexes;
    uint  m_iNumIndexes;
    uint m_iIndexSize;
    bool m_bVerbose;
};

#endif

