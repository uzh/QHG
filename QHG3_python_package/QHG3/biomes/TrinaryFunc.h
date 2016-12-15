#ifndef __TRINARYFUNC_H__
#define __TRINARYFUNC_H__

class TrinaryFunc {
public:
    virtual double calc(double dLon, double dLat)=0;
    virtual ~TrinaryFunc(){};
    double operator()(double dLon, double dLat) { 
        return calc(dLon, dLat); 
    };
};


#endif
