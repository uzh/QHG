#include "ThreshLookUp.h"
#include <math.h>
#include <stdio.h>
//-----------------------------------------------------------------------------
// constructor
//
ThreshLookUp::ThreshLookUp(double dThreshLevel) 
    :   LookUp(dThreshLevel, dThreshLevel) {

}

//-----------------------------------------------------------------------------
// getColor
//
void ThreshLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    if (isnan(dValue)) {
        dRed   = 0.5;
        dGreen = 0.5;
        dBlue  = 0.5;
        dAlpha = 0.0;
    } else if (isinf(dValue)) {
        dRed   = 0.5;
        dGreen = 0.5;
        dBlue  = 0.5;
        dAlpha = 0.0;
    } else if (dValue < m_dMinLevel) {
        
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 1.0;
        /*
        dRed   = 0.8;
        dGreen = 0.8;
        dBlue  = 0.8;
        dAlpha = 1.0;
        */
    } else {        
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 1.0;
    }
}

//-----------------------------------------------------------------------------
// getColor
//
void ThreshLookUp::getColor(double dValue, 
                            unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {

   if (isnan(dValue)) {
        uRed   = 127;
        uGreen = 127;
        uBlue  = 127;
        uAlpha = 0;
    } else if (isinf(dValue)) {
        uRed   = 127;
        uGreen = 127;
        uBlue  = 127;
        uAlpha = 0;
    } else if (dValue < m_dMinLevel) {
        uRed   = 0;
        uGreen = 0;
        uBlue  = 0;
        uAlpha = 255;
    } else {        
        uRed   = 255;
        uGreen = 255;
        uBlue  = 255;
        uAlpha = 255;
    }
}
