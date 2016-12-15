#ifndef __DENSITYLOOKUP_H__
#define __DENSITYLOOKUP_H__

#include "LookUp.h"

class DensityLookUp : public LookUp {

public:
    DensityLookUp(double dMinLevel, double dMaxLevel, double dR, double dG, double dB);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
   double m_dR;
   double m_dG;
   double m_dB;
};



#endif
