#ifndef __LONLATREGION__
#define __LONLATREGION__

#include "Region.h"

class IcoNode;

class LonLatRegion: public Region {
public:
    LonLatRegion(int iID, double dXMin, double dXMax, double dYMin, double dYMax);
    virtual ~LonLatRegion() {};

    virtual bool contains(IcoNode *pBC);
    virtual void display();
    virtual unsigned char *serialize();
    virtual int deserialize(unsigned char *pBuffer);
    virtual int dataSize() { return (int)(Region::dataSize()+4*sizeof(double)); };

    double m_dLonMin;
    double m_dLonMax;
    double m_dLatMin;
    double m_dLatMax;
};

#endif

