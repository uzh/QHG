#ifndef __DISCRETELOOKUP_H__
#define __DISCRETELOOKUP_H__

#include "LookUp.h"

class DiscreteLookUp : public LookUp {

public:
    DiscreteLookUp(int iNumCols);
    virtual ~DiscreteLookUp();
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    int m_iNumCols;
    double **m_aadCols;
};



#endif
