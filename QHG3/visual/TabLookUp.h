#ifndef __SEGLOOKUP_H__
#define __SEGLOOKUP_H__

#include "LookUp.h"
#include "SegCenters.h"

class TabLookUp : public LookUp {

public:
    TabLookUp(char *pTabFile);
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    bool readTable(char *pTabFile);

    MAP_DOUBLE_INT m_mapLookUp;
};



#endif
