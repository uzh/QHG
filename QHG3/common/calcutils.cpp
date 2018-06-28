#include <math.h>
#include "PolyLine.h"
#include "calcutils.h"


static PolyLine *pPL_log = NULL;
static PolyLine *pPL_exp = NULL;

void initCalc() {
    // polyline for log
    pPL_log = new PolyLine(10);
    pPL_log->addPoint(0,      1, log(1));    
    pPL_log->addPoint(1,      3, log(3));
    pPL_log->addPoint(2,      7, log(7));
    pPL_log->addPoint(3,     20, log(20));
    pPL_log->addPoint(4,     55, log(55));
    pPL_log->addPoint(5,    150, log(150));
    pPL_log->addPoint(6,    400, log(400));
    pPL_log->addPoint(7,   1100, log(1100));
    pPL_log->addPoint(8,   3000, log(3000));
    pPL_log->addPoint(9,   8100, log(8100));
    pPL_log->addPoint(10, 22000, log(22000));

    // polyline for exp
    pPL_exp = new PolyLine(10);
    pPL_exp->addPoint(0,   0, exp(0));    
    pPL_exp->addPoint(1,   1, exp(1));    
    pPL_exp->addPoint(2,   2, exp(2));
    pPL_exp->addPoint(3,   3, exp(3));
    pPL_exp->addPoint(4,   4, exp(4));
    pPL_exp->addPoint(5,   5, exp(5));
    pPL_exp->addPoint(6,   6, exp(6));
    pPL_exp->addPoint(7,   7, exp(7));
    pPL_exp->addPoint(8,   8, exp(8));
    pPL_exp->addPoint(9,   9, exp(9));
    pPL_exp->addPoint(10, 10, exp(10));
    
}

//-----------------------------------------------------------------------------
// linlog
//    piecewise linear approximation to log
//
double linlog(double fArg) {
    return pPL_log->getVal(fArg);
}

//-----------------------------------------------------------------------------
// linlog
//    piecewise linear approximation to exp
//
double linexp(double fArg) {
    return pPL_exp->getVal(fArg);
}

//-----------------------------------------------------------------------------
// termCalc
//    clean up
//
void termCalc() {
    if (pPL_log != NULL) {
        delete pPL_log;
    }
    if (pPL_exp != NULL) {
        delete pPL_exp;
    }
}
