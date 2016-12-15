#ifndef __ZEBRALOOKUP_H__
#define __ZEBRALOOKUP_H__

#include "LookUp.h"

class ZebraLookUp : public LookUp {

public:
    ZebraLookUp(int iWidth);
    virtual ~ZebraLookUp();
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    int m_iWidth;
    //    double **m_aadCols;
};



#endif
