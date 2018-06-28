#ifndef __POPLOOKUP_H__
#define __POPLOOKUP_H__

#include "LookUp.h"

class PopLookUp : public LookUp {

public:
    PopLookUp(double dMaxLevel, double dR, double dG, double dB, double dAlpha=0.5);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
   double m_dR;
   double m_dG;
   double m_dB;
   double m_dAlpha;
};



#endif
