#ifndef __ASSORTATIVEPAIRHAM_H__
#define __ASSORTATIVEPAIRHAM_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "Genetics.h"

#define ATTR_ASSPAIRHAM_NAME     "AssortativePairHamming"
#define ATTR_ASSPAIRHAM_CUTOFF   "AssortativePairHamming_cutoff"
#define ATTR_ASSPAIRHAM_PERMUTE  "AssortativePairHamming_permute"

class WELL512;

template<typename T, class U>
class AssortativePairHam : public Action<T> {
    
 public:
    AssortativePairHam(SPopulation<T> *pPop, SCellGrid *pCG, Genetics<T, U> *pGenetics,  WELL512 **apWELL);
    virtual ~AssortativePairHam();
    virtual int initialize(float fT);
    virtual int finalize(float fT);
    virtual int preLoop(); 

    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    virtual int writeParamsQDF(hid_t hSpeciesGroup);
    virtual int tryReadParamLine(char *pLine);

    void showAttributes();
 protected:
    int findMates();
    Genetics<T, U> *m_pGenetics;
    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

    float m_fCutOff;
    bool  m_bPermute;
    int m_iNumCells;
    int m_iGenomeSize;
    int m_iNumBlocks;

#ifdef OMP_A
    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;
#endif

};


#endif
