#ifndef __GRIDREGION_H__
#define __GRIDREGION_H__

#include "Region.h"

class IcoNod;

class GridRegion : public Region {
public:
    GridRegion();
    GridRegion(int iID, int iW, int iH, int iOffX, int iOffY, int iTW, int iTH);
    virtual ~GridRegion(){};
    virtual bool contains(IcoNode *pBC);
    virtual void display();
    virtual unsigned char *serialize();
    virtual int deserialize(unsigned char *pBuffer);
    virtual int dataSize();
    int m_iW;
    int m_iH;
    int m_iOffX;
    int m_iOffY;
    int m_iTW;
    int m_iTH;
};

#endif
