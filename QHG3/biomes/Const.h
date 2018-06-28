#ifndef __CONST_H__
#define __CONST_H__

#include "TrinaryFunc.h"

class Const : public TrinaryFunc {
public:
    Const(double dC); 

    double calc(double dLon, double dLat) {return m_dC;};
protected:
    double  m_dC;
};

#endif
