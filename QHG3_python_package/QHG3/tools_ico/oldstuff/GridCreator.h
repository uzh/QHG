#ifndef __GRIDCREATOR_H__
#define __GRIDCREATOR_H__

#include "types.h"
#include "icoutil.h"
#include "GridZones.h"

class VertexLinkage;
class GridProjection;
class IcoGridNodes;
class Region;
class RegionSplitter;


class GridCreator {
public:
     IcoGridNodes *getGrid(int iID);
    virtual ~GridCreator();

    std::map<gridtype, gridtype> &getIDTiledMapping() { return m_mID2RegID;};
    
    void setChecks(int iUseMask, bool bSanityCheck);
protected:
    GridCreator(int iHalo, bool bSuperCells);
    void createZones(Region **apTiles, PolarConv *pPC, bool bNodeOrder);
    void superCellOrder(int iShift);
    bool zoneSanity();

    int m_iHalo;
    int m_iNumTiles;
    VertexLinkage *m_pVL;
    GridProjection *m_pGP;

    GridZones *m_pGZ;
    intcoll   *m_asNodeIDs;
    
    bool m_bSuperCells;
    std::map<gridtype, gridtype> m_mID2RegID;
    
    int m_iUseMask;
    bool m_bSanityCheck;
};



#endif

