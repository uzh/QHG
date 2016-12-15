#ifndef __GRIDZONES_H__
#define __GRIDZONES_H__

#include <set>
#include <map>

class VertexLinkage;
class IcoNode;
class IcoGridNodes;
class Region;
class GridProjection;

typedef std::set<gridtype> intset;
typedef std::map<gridtype, intset> intcoll;

#define DIST_ICO 0
#define DIST_FLAT_SPHERE 1
#define DIST_FLAT_LINEAR 2

class PolarConv {
public:
    virtual void conv2Polar(Vec3D *pV, double *pdLon, double *pdLat)=0;
    virtual ~PolarConv() {};
};




class GridZones {
public:
    static GridZones *create(VertexLinkage *pVL, PolarConv *pPC, GridProjection *pGP, bool bNodeOrder, int iUseMask=4);
    ~GridZones();

    void findEdgeHaloForRegion(Region *pR, int iHalo, intcoll &sNodeIDs);
    IcoGridNodes *createGridForNodes(intcoll &sNodeIDs);

    int getNumNodes() { return m_iNumNodes; };
    //   IcoNode *getNode(int i) { return m_pNodes[i]; };
    //IcoNode **getNodes() { return m_pNodes;};
    static gridtype *flattenSets(intset &sEdge, intset &sHalo);
protected:
    GridZones(int iNumNodes, int iUseMask);
    int init(VertexLinkage *pVL, PolarConv *pPC, GridProjection *pGP);
    int orderNodes();
    void expandRange(int iHalo, intset &sTotal, intset &sStart, intset &sAvoid);
    int m_iNumNodes;
    IcoNode **m_pNodes;
    std::map<gridtype, gridtype> m_mID2Index;
    int m_iUseMask;

};

#endif
