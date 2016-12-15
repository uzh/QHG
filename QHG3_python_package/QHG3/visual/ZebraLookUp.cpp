#include <stdio.h>
#include <math.h>
#include "ZebraLookUp.h"
#include "types.h"

/*
const int BASE_COLS = 12;
static double adCBasic[BASE_COLS][4] = {
    {1.0, 0.0, 0.0, 1.0}, 
    {0.0, 1.0, 0.0, 1.0}, 
    {0.0, 0.0, 1.0, 1.0}, 
    {0.0, 1.0, 1.0, 1.0}, 
    {1.0, 0.0, 1.0, 1.0}, 
    {1.0, 1.0, 0.0, 1.0}, 

    {1.0, 0.7, 0.0, 1.0}, 
    {1.0, 0.7, 0.7, 1.0},
    {0.7, 1.0, 0.0, 1.0},
    {0.5, 0.0, 1.0, 1.0},
    {0.7, 0.7, 0.7, 1.0},
    {0.5, 1.0, 0.7, 1.0},
};
*/
//-----------------------------------------------------------------------------
// constructor
//
ZebraLookUp::ZebraLookUp(int iWidth) 
    :   LookUp(0,1.0*iWidth),
        m_iWidth(iWidth) {
    

}


//-----------------------------------------------------------------------------
// destructor
//
ZebraLookUp::~ZebraLookUp() {
 
}

//-----------------------------------------------------------------------------
// getColor
//
void ZebraLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {


    if (isnan(dValue)) {
        dRed   = 0.3;
        dGreen = 0.3;
        dBlue  = 0.3;
        dAlpha = 0.0;
    } else if (isinf(dValue)) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 0.0;
    } else {
        double dI = fmod(dValue, 2*m_iWidth);
        if (dI < 0) {
            dI += 2*m_iWidth;
        }
        
        if (dI < m_iWidth) {
            dRed   = 1;
            dGreen = 1;
            dBlue  = 1;
            dAlpha = 1;
        } else {
            dRed   = 0;
            dGreen = 0;
            dBlue  = 0;
            dAlpha = 1;
        }
    }
    // printf("Val %d, col %f %f %f %f\n", iVal, dRed, dGreen, dBlue, dAlpha);
}
