#ifndef __LONLATTILING_H__
#define __LONLATTILING_H__

#include "dbgprint.h"
#include "BasicTiling.h"

class IcoGridNodes;
class EQsahedron;


class LonLatTiling  : public BasicTiling {
public:
    static LonLatTiling *createInstance(int iSubDivNodes, int iLon, int iNLat, double dMaxLat, int iVerbosity=LL_NONE);
    virtual ~LonLatTiling();

    //   static void extendedLonLatDisplay(EQNodeClassificator *pENC, EQsahedron *pEQNodes);

protected:
    LonLatTiling(int iSubDivNodes, int iLon, int iNLat, double dMaxLat, int iVerbosity);
    virtual int init();
    virtual int split();
    int m_iNLon;
    int m_iNLat;
    double m_dMaxLat;

};

#endif
