#ifndef __DIRTESTPOP_H__
#define __DIRTESTPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "DirMove.h"
#include "Verhulst.h"
#include "GetOld.h"

struct DirTestAgent : Agent {
    float  m_fAge;
    int    m_iMateIndex;
    float  m_fDirection;
    float  m_fError;
    float  m_fOldError;
};


class DirTestPop : public SPopulation<DirTestAgent> {

 public:
    DirTestPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~DirTestPop();

    int preLoop();

    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);


 protected:
    DirMove<DirTestAgent>    *m_pDM;
    Verhulst<DirTestAgent>   *m_pVer;
    RandomPair<DirTestAgent> *m_pPair;
    GetOld<DirTestAgent>     *m_pGO;

};


#endif
