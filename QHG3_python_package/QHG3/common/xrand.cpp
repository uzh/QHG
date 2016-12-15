
#include <stdlib.h>
#include <math.h>

#include "xrand.h"
#include "xrandtab.h"


//-----------------------------------------------------------------------------
// gauss_rand
//
double gauss_rand() {
    double x1, x2, w, y1;
    static double y2; 
    static bool bNeedNew = true;

    if (bNeedNew) {
        do {
            x1 = (2.0 * tabrand())/RAND_MAX - 1.0;
            x2 = (2.0 * tabrand())/RAND_MAX - 1.0;
            w = x1 * x1 + x2 * x2;
        } while ( w >= 1.0 );
        
        w = (w==0)?0:sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;
        bNeedNew = false;
    } else {
        y1 = y2;
        bNeedNew = true;
    }
    return y1;
}


static unsigned int s_xIndex = 0;
void tabrandinit(unsigned int iSeed) {
    s_xIndex = iSeed%RTAB_SIZE;
}

unsigned int get_seed() {
    return s_xIndex;
}

unsigned int tabrand() {
    return ((s_xIndex < RTAB_SIZE) ? aiRands[s_xIndex++]:aiRands[s_xIndex=0]);
}
