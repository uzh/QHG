#ifndef __TWOTONELOOKUP_H__
#define __TWOTONELOOKUP_H__

#include "LookUp.h"

class TwoToneLookUp : public LookUp {

public:
    TwoToneLookUp(double dSepLevel, 
                  double dR1, double dG1, double dB1, double dA1,
                  double dR2, double dG2, double dB2, double dA2);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);

protected:
   double m_dSepLevel;
   double m_dR1;
   double m_dG1;
   double m_dB1;
   double m_dA1;
   double m_dR2;
   double m_dG2;
   double m_dB2;
   double m_dA2;
};



#endif
