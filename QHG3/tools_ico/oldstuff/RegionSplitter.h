#ifndef __REGIONSPLITTER_H__
#define __REGIONSPLITTER_H__

#include "icoutil.h"

class Region;

class RegionSplitter {
public:
    RegionSplitter();
    virtual ~RegionSplitter();
    virtual Region **createRegions(int *piNumTiles)= 0;
    void setBox( tbox *pBox) { m_pBox = pBox; };
    tbox *getBox() { return m_pBox; };
    int getNumRegions() { return m_iNumRegions;};
protected:
    int      m_iNumRegions;
    Region **m_apRegions;
    tbox *m_pBox;
};


#endif

