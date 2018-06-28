#ifndef __GEO2LOOKUP_H__
#define __GEO2LOOKUP_H__

#include "LookUp.h"

class Geo2LookUp : public LookUp {

public:
    Geo2LookUp(double dMinLevel, double dSeaLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    double m_dSeaLevel;
};



#endif
