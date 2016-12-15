#include <stdio.h>
#include <math.h>
#include "DensityLookUp.h"

//-----------------------------------------------------------------------------
// constructor
//
DensityLookUp::DensityLookUp(double dMinLevel, double dMaxLevel, double dR, double dG, double dB) 
    :   LookUp(dMinLevel,dMaxLevel),
        m_dR(dR),
        m_dG(dG),
        m_dB(dB)  {
    printf("DensitiyLookUp with [%f, %f], (%f, %f, %f)\n", dMinLevel, dMaxLevel, dR, dG, dB);
}

//-----------------------------------------------------------------------------
// getColor
//
void DensityLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    dValue -= m_dMinLevel;
    dValue /= (m_dMaxLevel-m_dMinLevel);

    if (isnan(dValue)) {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 0.0;
    } else if (isinf(dValue)) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 0.0;
    } else if (dValue < 0) {
        dRed   = m_dR;
        dGreen = m_dG;
        dBlue  = m_dB;
        dAlpha = 0.0;
    } else if (dValue > 1) {
        dRed   = m_dR;
        dGreen = m_dG;
        dBlue  = m_dB;
        dAlpha = 1.0;
    } else {
        dRed   = m_dR;
        dGreen = m_dG;
        dBlue  = m_dB;
        dAlpha = dValue;
    }
}
