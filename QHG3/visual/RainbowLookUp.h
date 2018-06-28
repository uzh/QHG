#ifndef __RAINBOWLOOKUP_H__
#define __RAINBOWLOOKUP_H__

#include "LookUp.h"

class RainbowLookUp : public LookUp {

public:
    RainbowLookUp(double dMinLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);


};



#endif
