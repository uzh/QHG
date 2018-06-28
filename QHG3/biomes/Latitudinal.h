#ifndef __LATITUDINAL_H__
#define __LATITUDINAL_H__

#include "TrinaryFunc.h"

class Latitudinal : public TrinaryFunc {
public:
    double calc(double dLon, double dLat);
};

#endif
