#ifndef __EQTILING_H__
#define __EQTILING_H__

#include "dbgprint.h"
#include "BasicTiling.h"

class IcoGridNodes;
class EQsahedron;
class EQNodeClassificator;

class EQTiling : public BasicTiling {
public:
    static EQTiling *createInstance(int iSubDivNodes, int iSubDivTiles, int iVerbosity=LL_NONE);
    virtual ~EQTiling();

    //    int getSubDivTiles() { return m_iSubDivTiles;};
    static void extendedEQDisplay(EQNodeClassificator *pENC, EQsahedron *pEQNodes);

protected:
    EQTiling(int iSubDivNodes, int iSubDivTiles, int iVerbosity);
    virtual int init();
    virtual int split();
    int m_iSubDivTiles;

};

#endif
