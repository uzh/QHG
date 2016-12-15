#include <math.h>
#include "LookUp.h"

void LookUp::getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha) {
    double dRed;
    double dGreen;
    double dBlue;
    double dAlpha;

    getColor(dValue, dRed, dGreen, dBlue, dAlpha);
    uRed   = (unsigned char)round(dRed*255);    
    uGreen = (unsigned char)round(dGreen*255);    
    uBlue  = (unsigned char)round(dBlue*255);
    uAlpha = (unsigned char)round(dAlpha*255);
}
