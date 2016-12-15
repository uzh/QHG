#ifndef __GEOLOOKUP_H__
#define __GEOLOOKUP_H__

#include "LookUp.h"

class GeoLookUp : public LookUp {

public:
    GeoLookUp(double dMinLevel, double dSeaLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    double m_dSeaLevel;
};



#endif
