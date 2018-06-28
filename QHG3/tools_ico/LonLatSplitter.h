#ifndef __LONLATSPLITTER_H__
#define __LONLATSPLITTER_H__

#include "types.h"
#include "BasicTile.h"
#include "BasicSplitter.h"
#include "EQsahedron.h"


class LonLatSplitter : public BasicSplitter {
public:
    LonLatSplitter(EQsahedron *pEQ, int iNX, int iNY, double dCapSize);
    LonLatSplitter(int iNumTiles, double dCapSize, bool bFree);

    //    virtual ~RegionSplitter()
    virtual BasicTile **createTiles(int *piNumTiles);
protected:
    BasicTile **createTilesBalanced();
    BasicTile **createTilesBalancedCaps();
    int  m_iNX;
    int  m_iNY;
    double m_dCapSize;
    bool m_bFree;
    EQsahedron          *m_pEQ;

};


#endif

