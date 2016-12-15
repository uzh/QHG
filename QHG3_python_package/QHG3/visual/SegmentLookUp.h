#ifndef __SEGMENTLOOKUP_H__
#define __SEGMENTLOOKUP_H__

#include "LookUp.h"

class SegmentLookUp : public LookUp {

public:
    SegmentLookUp(double dMin, double dMax, double dWidth, double iSubDiv=0);
    virtual ~SegmentLookUp();
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    double m_dWidth;
    int    m_iSubDiv;
    double m_dSubDiv;
    int m_iNumCols;
    double **m_aadCols;
};



#endif
