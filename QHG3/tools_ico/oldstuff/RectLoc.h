#ifndef __RECTLOC_H__
#define __RECTLOC_H__

#include "types.h"
#include "icoutil.h"
#include "IcoLoc.h"

class GridProjection;

class RectLoc: public IcoLoc {
public:
    RectLoc(GridProjection *pGrPr);
    virtual ~RectLoc() {};
    virtual gridtype findNode(double dLon, double dLat);
    virtual bool findCoords(int iNodeID,double *pdLon, double *pdLat);

protected:
    GridProjection *m_pGrPr;
    int m_iW;
};
#endif
