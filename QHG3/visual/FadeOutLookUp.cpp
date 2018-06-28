#include "FadeOutLookUp.h"
#include <math.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// constructor
//
FadeOutLookUp::FadeOutLookUp(double dMinLevel, double dMaxLevel,
                             double dR, double dG, double dB, double dA)
 
    :   LookUp(dMinLevel, dMaxLevel),
        m_dR(dR),
        m_dG(dG),
        m_dB(dB),
        m_dA(dA) {
}


//-----------------------------------------------------------------------------
// getColor
//
void FadeOutLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    if (isnan(dValue)) {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 0.0;
     } else if (dValue <= m_dMinLevel) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 0.0;
        
    } else if (dValue >= m_dMaxLevel) {
        dRed   = m_dR;
        dGreen = m_dG;
        dBlue  = m_dB;
        dAlpha = m_dA;

    } else {
        
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;
        
        dRed   = m_dR*dValue;
        dGreen = m_dG*dValue;
        dBlue  = m_dB*dValue;
        dAlpha = m_dA*dValue;
        
    }
}


//-----------------------------------------------------------------------------
// getColor
//
void FadeOutLookUp::getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {
    double dRed;
    double dGreen;
    double dBlue;
    double dAlpha;

    getColor(dValue, dRed, dGreen, dBlue, dAlpha);
    uRed   = 255*dRed;
    uGreen = 255*dGreen;
    uBlue  = 255*dBlue;
    uAlpha = 255*dAlpha;
}


