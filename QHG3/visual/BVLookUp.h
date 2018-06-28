#ifndef __BVLOOKUP_H__
#define __BVLOOKUP_H__

#include "LookUp.h"

class BVLookUp : public LookUp {

public:
    BVLookUp(double dMinLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);


};



#endif
