#ifndef __UCHARLOOKUP_H__
#define __UCHARLOOKUP_H__

#include "LookUp.h"

class UCharLookUp : public LookUp {

public:
    UCharLookUp(int iNumCols);
    virtual ~UCharLookUp();
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    int m_iNumCols;
    double **m_aadCols;
};



#endif
