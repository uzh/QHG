#ifndef __LONLATTILE__
#define __LONLATTILE__

#include "BasicTile.h"

class IcoNode;

class LonLatTile: public BasicTile {
public:
    LonLatTile(int iID, double dXMin, double dXMax, double dYMin, double dYMax);
    virtual ~LonLatTile() {};

    virtual bool contains(IcoNode *pBC);
    virtual void display();
    virtual unsigned char *serialize();
    virtual int deserialize(unsigned char *pBuffer);
    virtual int dataSize() { return (int)(BasicTile::dataSize()+4*sizeof(double)); };

    double m_dLonMin;
    double m_dLonMax;
    double m_dLatMin;
    double m_dLatMax;
};

#endif

