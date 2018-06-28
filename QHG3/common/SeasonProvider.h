#ifndef __SEASONPROVIDER_H__
#define __SEASONPROVIDER_H__

class SeasonProvider {
public:
    virtual ~SeasonProvider(){};
    virtual float getValue(double dLon, double dLat, int iMonth)=0;
    virtual float getValueBiLin(double dLon, double dLat, int iMonth)=0;
};
#endif
