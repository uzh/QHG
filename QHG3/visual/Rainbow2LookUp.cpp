#include "Rainbow2LookUp.h"
#include <math.h>
#include <stdio.h>
//-----------------------------------------------------------------------------
// constructor
//
Rainbow2LookUp::Rainbow2LookUp(double dMinLevel, double dMaxLevel) 
    :   LookUp(dMinLevel, dMaxLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void Rainbow2LookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

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
    } else {
        
        dValue -= m_dMinLevel;
        dValue /= m_dMaxLevel - m_dMinLevel;

        if (dValue < 0) {
            // less than min: cyan
            dRed   = 0.5;
            dGreen = 0.0;
            dBlue  = 0.0;

        } else if (6*dValue < 1) {
            double z = 6*dValue;
            dRed   = 1.0; 
            dGreen = z*(2.0-z);
            dBlue  = 0.0;

        } else if (6*dValue < 2) {
            double z = 6*dValue - 1;
            dRed   = 1.0-z*z; 
            dGreen = 1.0;
            dBlue  = 0.0;
            
        } else if (6*dValue < 3) {
            double z = 6*dValue - 2;
            dRed   = 0.0;
            dGreen = 1.0;
            dBlue  = z*(2.0-z);

        } else if (6*dValue < 4) {
            double z = 6*dValue - 3;
            dRed   = 0.0;
            dGreen = 1.0-z*z;
            dBlue  = 1.0;

        } else if (6*dValue < 5) {
            double z = 6*dValue - 4;
            dRed   = z*(2.0-z);
            dGreen = 0.0;
            dBlue  = 1.0;

        } else {
            // more than max: magenta
            dRed   = 1.0;
            dGreen = 0.5;
            dBlue  = 1.0;
        }
        
        dAlpha = 1.0;
    }
}


//-----------------------------------------------------------------------------
// getColor
//
void Rainbow2LookUp::getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {
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

