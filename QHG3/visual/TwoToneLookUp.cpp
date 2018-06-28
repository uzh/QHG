#include "TwoToneLookUp.h"

//-----------------------------------------------------------------------------
// constructor
//
TwoToneLookUp::TwoToneLookUp(double dSepLevel, 
                             double dR1, double dG1, double dB1, double dA1,
                             double dR2, double dG2, double dB2, double dA2) 
    :   LookUp(0,0),
        m_dSepLevel(dSepLevel),
        m_dR1(dR1),
        m_dG1(dG1),
        m_dB1(dB1),
        m_dA1(dA1),
        m_dR2(dR2),
        m_dG2(dG2),
        m_dB2(dB2),
        m_dA2(dA2) {
    
}

//-----------------------------------------------------------------------------
// getColor
//
void TwoToneLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    if (dValue < m_dSepLevel) {
        dRed   = m_dR1;
        dGreen = m_dG1;
        dBlue  = m_dB1;
        dAlpha = m_dA1;
    } else { 
        dRed   = m_dR2;
        dGreen = m_dG2;
        dBlue  = m_dB2;
        dAlpha = m_dA2;
    }
}

void TwoToneLookUp::getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {
    if (dValue < m_dSepLevel) {
        uRed   = 255*m_dR1;
        uGreen = 255*m_dG1;
        uBlue  = 255*m_dB1;
        uAlpha = 255*m_dA1;
    } else { 
        uRed   = 255*m_dR2;
        uGreen = 255*m_dG2;
        uBlue  = 255*m_dB2;
        uAlpha = 255*m_dA2;
    }
}
