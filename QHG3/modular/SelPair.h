#ifndef __SELPAIR_H__
#define __SELPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "PolyLine.h"
#include "LayerArrBuf.h"
#include "WELL512.h"

#define ATTR_SELPAIR_NAME      "SelPair"
#define ATTR_SELPAIR_PROB_NAME "SelPairProb"

class WELL512;
class PolyLine;

template<typename T>
class SelPair : public Action<T> {
    
 public:
    SelPair(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);
    virtual ~SelPair();
    int initialize(float fT);
    int finalize(float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    void setGenome(LayerArrBuf<ulong> *pGenome, int iNumBlocks) { m_pGenome = pGenome; m_iNumBlocks = iNumBlocks;};

    void showAttributes();
 protected:
    int findMates();
    int findCompatiblePartner(int iCur, std::vector<int> &vAvailable);
    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

    PolyLine *m_pPLDistValues;           // how to go from genetic distances to weights
    LayerArrBuf<ulong> *m_pGenome;
    int m_iNumBlocks;

#ifdef OMP_A
    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;
#endif

};

#endif
