#ifndef __THRESHLOOKUP_H__
#define __THRESHLOOKUP_H__

#include "LookUp.h"

class ThreshLookUp : public LookUp {

public:
    ThreshLookUp(double dThresh);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);


};



#endif
