#ifndef __LONLATSPLITTER_H__
#define __LONLATSPLITTER_H__

#include "types.h"
#include "RegionSplitter.h"

class Region;


class LonLatSplitter : public RegionSplitter {
public:
    LonLatSplitter(int iNX, int iNY, double dCapSize);
    LonLatSplitter(int iNumTiles, double dCapSize, bool bFree);

    //    virtual ~RegionSplitter()
    virtual Region **createRegions(int *piNumTiles);
protected:
    Region **createTilesBalanced();
    Region **createTilesBalancedCaps();
    int  m_iNX;
    int  m_iNY;
    double m_dCapSize;
    bool m_bFree;

};


#endif

