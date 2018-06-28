#ifndef __BIOMELOOKUP_H__
#define __BIOMELOOKUP_H__

#include "LookUp.h"

class BiomeLookUp : public LookUp {

public:
    BiomeLookUp();
    virtual ~BiomeLookUp();
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    double **m_aadCols;
};



#endif
