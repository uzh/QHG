#ifndef __EQZONES_H__
#define __EQZONES_H__

#include <set>
#include <map>

class VertexLinkage;
class IcoNode;
class IcoGridNodes;
class BasicTile;

typedef std::set<gridtype> intset;
typedef std::map<gridtype, intset> intcoll;




class EQZones {
public:
    static EQZones *create(VertexLinkage *pVL, int iUseMask, int iVerbosity);
    ~EQZones();

    void findEdgeHaloForRegion(BasicTile *pTile, int iHalo, intcoll &sNodeIDs);
    IcoGridNodes *createGridForNodes(intcoll &sNodeIDs);

    int getNumNodes() { return m_iNumNodes; };
  
    static gridtype *flattenSets(intset &sEdge, intset &sHalo);
protected:
    EQZones(int iNumNodes, int iUseMask, int iVerbosity);
    int init(VertexLinkage *pVL);
    bool contains(intset &sNodeIDs, IcoNode *pBC);

    void expandRange(int iHalo, intset &sTotal, intset &sStart, intset &sAvoid);
    int m_iNumNodes;
    IcoNode **m_pNodes;
    std::map<gridtype, gridtype> m_mID2Index;
    int m_iUseMask;
    int m_iVerbosity;
};

#endif
