#include "VegLookUp.h"
#include <stdio.h>


//-----------------------------------------------------------------------------
// constructor
//
VegLookUp::VegLookUp(double dMinLevel, double dMaxLevel, int iType) 
    :   LookUp(dMinLevel, dMaxLevel),
        m_iType(iType) {
    
}

//-----------------------------------------------------------------------------
// getColor
// type  | Low        | High       | over
//-------+------------+------------+--------
//  0    | brownish   | green      | cyan
//  1    | purplish   | dark blue  | orange
//  2    | dark cyan  | green      | pink
// type 1 : low=dark blue High= 
void VegLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    if (dValue <= m_dMinLevel) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.4;
        dAlpha = 0.1;
    } else if (dValue > m_dMaxLevel) { 
        dAlpha = 1.0;
        switch (m_iType) {
        case 0: // cyan
            dRed   = 0.0;
            dGreen = 1.0;
            dBlue  = 1.0;
            dAlpha = 1.0;
            break;
        case 1: // orange
            dRed   = 1.0;
            dGreen = 0.78;
            dBlue  = 0.0;
            dAlpha = 1.0;
            break;
        case 2: // pink
            dRed   = 1.0;
            dGreen = 0.58;
            dBlue  = 0.78;
            dAlpha = 1.0;
            break;
        }
    } else {
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;
        switch (m_iType) {
        case 0:
            dRed   = 0.78*(1-dValue);
            dGreen = 0.58 + dValue*0.42;
            dBlue  = 0.0;
            dAlpha = 1.0;
            break;
        case 1:
            dRed   = 0.78*(1-dValue);
            dGreen = 0.0;
            dBlue  = 0.58 + dValue*0.42;
            dAlpha = 1.0;
            break;
        case 2:
            dRed   = 0.0;
            dGreen = 0.58 + dValue*0.42;
            dBlue  = 0.78*(1-dValue);
            dAlpha = 1.0;
            break;
        }
    }
}
