#ifndef __FADEOUT_H_
#define __FADEOUT_H_

#include "LookUp.h"

class FadeOutLookUp : public LookUp {

public:
    FadeOutLookUp(double dMin, double dMax,
                  double dR, double dG, double dB, double dA);

    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);

protected:
   double m_dR;
   double m_dG;
   double m_dB;
   double m_dA;
};



#endif
