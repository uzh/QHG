#ifndef __RAINBOW2LOOKUP_H__
#define __RAINBOW2LOOKUP_H__

#include "LookUp.h"

class Rainbow2LookUp : public LookUp {

public:
    Rainbow2LookUp(double dMinLevel, double dMaxLevel);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);


};



#endif
