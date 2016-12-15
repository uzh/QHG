#ifndef __VEGLOOKUP_H__
#define __VEGLOOKUP_H__

#include "LookUp.h"

class VegLookUp : public LookUp {

public:
    VegLookUp(double dMinLevel, double dMaxLevel, int iType=0);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    int    m_iType; 
};



#endif
#ifndef __VEGLOOKUP_H__
#define __VEGLOOKUP_H__

#include "LookUp.h"

class VegLookUp : public LookUp {

public:
    VegLookUp(double dMinLevel, double dMaxLevel, int iType=0);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    int    m_iType; 
};



#endif
