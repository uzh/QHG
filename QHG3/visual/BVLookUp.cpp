#include "BVLookUp.h"
#include <math.h>
#include <stdio.h>
//-----------------------------------------------------------------------------
// constructor
//
BVLookUp::BVLookUp(double dMinLevel, double dMaxLevel) 
    :   LookUp(dMinLevel, dMaxLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void BVLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

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
    } else if (dValue <= m_dMinLevel) {
        dRed   = 0.0;
        dGreen = 1.0;

        dBlue  = 1.0;
        dAlpha = 1.0;
        
    } else if (dValue >= m_dMaxLevel) {
        dRed   = 1.0;
        dGreen = 0.0;
        dBlue  = 1.0;
        dAlpha = 1.0;
    } else {
        
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;
        if (3*dValue < 1) {

            dRed   = 3*dValue;
            dGreen = 0.6;
            dBlue  = 0.0;
        } else if (3*dValue < 2) {

            dRed   = 1.0; 
            dGreen = 1.2-1.8*dValue;
            dBlue  = 0.0;
        } else {

            dRed   = 3 - 3*dValue;
            dGreen = 0.0;
            dBlue  = 3*dValue - 2;
        }
        
        dAlpha = 1.0;
    }
}
