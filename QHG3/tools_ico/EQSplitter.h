#ifndef __EQSPLITTER_H__
#define __EQSPLITTER_H__

#include "types.h"
#include "BasicTile.h"
#include "BasicSplitter.h"
#include "EQsahedron.h"
#include "EQNodeClassificator.h"


class EQSplitter : public BasicSplitter  {
public:
    //    EQSplitter(EQsahedron *pEQ, int iSubDivTiles, int iVerbosity);
    EQSplitter(EQsahedron *pEQ, EQNodeClassificator *pENC, int iVerbosity);
    virtual ~EQSplitter();
    virtual BasicTile **createTiles(int *piNumTiles);
    //    EQNodeClassificator *getENC() { return m_pENC;};
    //   int getNumTiles() {return m_iNumTiles;};
protected:
    //   int     m_iNumTiles;
    BasicTile **m_asTileNodes;
    EQNodeClassificator *m_pENC;
    EQsahedron          *m_pEQ;
    int    m_iVerbosity;
};



#endif
