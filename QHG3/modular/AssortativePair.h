#ifndef __ASSORTATIVEPAIR_H__
#define __ASSORTATIVEPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "Genetics.h"

#define ATTR_ASSPAIR_NAME     "AssortativePair"
#define ATTR_ASSPAIR_CUTOFF   "AssortativePair_cutoff"
#define ATTR_ASSPAIR_PERMUTE  "AssortativePair_permute"

class WELL512;

template<typename T, class U>
class AssortativePair : public Action<T> {
    
 public:
    AssortativePair(SPopulation<T> *pPop, SCellGrid *pCG, Genetics<T, U> *pGenetics,  WELL512 **apWELL);
    virtual ~AssortativePair();
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
    bool m_bPermute;
    int m_iNumCells;
    int m_iGenomeSize;

#ifdef OMP_A
    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;
#endif

};


#endif
