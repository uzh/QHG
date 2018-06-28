#ifndef __SUNLOOKUP_H__
#define __SUNLOOKUP_H__

#include "LookUp.h"

class SunLookUp : public LookUp {

public:
    SunLookUp(double dMinLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);


};



#endif
