#include "GenLookUp.h"
#include <math.h>
#include <stdio.h>
//-----------------------------------------------------------------------------
// constructor
//
GenLookUp::GenLookUp(double dMinLevel, double dMaxLevel) 
    :   LookUp(dMinLevel, dMaxLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void GenLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    dValue -= m_dMinLevel;
    dValue /= (m_dMaxLevel - m_dMinLevel);
    
    int iValue = 0xffffff & (int)(0xffffff*dValue);
    dBlue = (1.0*(iValue & 0xff))/256;
    iValue >>= 8;
    dGreen = (1.0*(iValue & 0xff))/256;
    iValue >>= 8;
    dRed = (1.0*(iValue & 0xff))/256;
    dAlpha = 1.0;
}
