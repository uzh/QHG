#ifndef __GENLOOKUP_H__
#define __GENLOOKUP_H__

#include "LookUp.h"

class GenLookUp : public LookUp {

public:
    GenLookUp(double dMinLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);


};



#endif
