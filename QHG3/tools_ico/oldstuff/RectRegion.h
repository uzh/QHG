#ifndef __RECTREGION__
#define __RECTREGION__

#include "Region.h"

class IcoNode;
class GridProjection;

class RectRegion: public Region {
public:
    RectRegion(int iID, GridProjection *pGP, double dH, double dXMin, double dXMax, double dYMin, double dYMax);
    virtual ~RectRegion() {};

    virtual bool contains(IcoNode *pBC);
    virtual void display();
    virtual unsigned char *serialize();
    virtual int deserialize(unsigned char *pBuffer);
    virtual int dataSize() { return (int)(Region::dataSize()+4*sizeof(double)); };

    GridProjection *m_pGP;
    double m_dH;
    int    m_iGridWidth;
    double m_dXMin;
    double m_dXMax;
    double m_dYMin;
    double m_dYMax;
};

#endif

