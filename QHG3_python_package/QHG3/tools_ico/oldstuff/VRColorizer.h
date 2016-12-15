#ifndef __VRCOLORIZER_H__
#define __VRCOLORIZER_H__

#include "IcoColorizer.h"

class ValReader;

class VRColorizer : public IcoColorizer {
public:
    VRColorizer(ValReader *pVR);
    virtual void getCol(double dLon, double dLat, float fCol[4]); 

    ValReader *m_pVR;
};


#endif

