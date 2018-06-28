#include "PopLookUp.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
// constructor
//
PopLookUp::PopLookUp(double dMaxLevel, double dR, double dG, double dB, double dAlpha) 
    :   LookUp(0,dMaxLevel),
        m_dR(dR),
        m_dG(dG),
        m_dB(dB),
        m_dAlpha(dAlpha)  {
    
}

//-----------------------------------------------------------------------------
// getColor
//
void PopLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    dValue /= m_dMaxLevel;
    //dAlpha = m_dAlpha*dValue;
    //    dAlpha = m_dAlpha;
/*    
    if (dValue < 5/m_dMaxLevel) {
        dRed   = 0.0;
        dBlue  = 0.0;
        dGreen = 0.0;
        dAlpha = 0.0;
    } else if (dValue < 1) {
        dRed   = 0.0;
        dGreen = dValue;
        dBlue  = 1.0;
    } else if (dValue < 2) { 
		dRed   = 0.0;
		dGreen = 1.0;
		dBlue  = 2.0 - dValue;
	} else if (dValue < 3) {
        dRed   = dValue - 2.0;
        dGreen = 1.0;
        dBlue  = 0.0;
    } else if (dValue < 4) {
        dRed   = 1.0;
        dGreen = 4.0 - dValue;    
		dBlue  = 0;
    } else if (dValue < 5) {
        dRed   = 1.0;
        dGreen = 0.0;
        dBlue  = dValue - 4.0;
    } else {
        dRed   = 1.0;
        dGreen = 0.0;
        dBlue  = 1.0;
    }   
*/
/*
    if (dValue < 1/m_dMaxLevel) {
        dRed   = 0.0;
        dBlue  = 0.0;
        dGreen = 0.0;
        dAlpha = 0.0;
    } else if (dValue < 1) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = dValue;
    } else if (dValue < 2) { 
		dRed   = dValue-1.0;
		dGreen = 0.0;
		dBlue  = 1.0;
	} else if (dValue < 3) {
        dRed   = 1.0;
        dGreen = dValue - 2.0;
        dBlue  = 0.0;
    } else {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
    }   
*/

/*
    dValue *= 3;
    if (dValue < 1/m_dMaxLevel) {
        dRed   = 0.0;
        dBlue  = 0.0;
        dGreen = 0.0;
        dAlpha = 0.0;
    } else if (dValue < 1) {
        dRed   = dValue*m_dR;
        dGreen = dValue*m_dG;
        dBlue  = dValue*m_dB;
    } else if (dValue < 2) { 
        dRed   = (dValue-1.0)*m_dR;
        dGreen = (dValue-1.0)*m_dG;
        dBlue  = (dValue-1.0)*m_dB;
    } else if (dValue < 3) {
        dRed   = (dValue - 2.0)*m_dR;
        dGreen = (dValue - 2.0)*m_dG;
        dBlue  = (dValue - 2.0)*m_dB;
    } else {
        dRed   = m_dR;
        dGreen = m_dG;
        dBlue  = m_dB;
    }   
*/

   if (dValue <= 0) {
        dRed   = 0.0;
        dBlue  = 0.0;
        dGreen = 0.0;
        dAlpha = 0.0;
        //        dAlpha = 1.0;
    } else if (dValue < 1) {
        dRed   = dValue*m_dR;
        dGreen = dValue*m_dG;
        dBlue  = dValue*m_dB;
        dAlpha = 1.0;
    } else {
        dRed   = m_dR;
        dGreen = m_dG;
        dBlue  = m_dB;
        dAlpha = 1.0;
    }   
}
