#ifndef __RANDPAIR_H__
#define __RANDPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"

#define RANDPAIR_NAME "RandomPair"

class WELL512;

template<typename T>
class RandomPair : public Action<T> {
    
 public:
    RandomPair(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);
    ~RandomPair();
    int initialize(float fT);
    int finalize(float fT);


 protected:
    int findMates();
    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

#ifdef OMP_A
    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;
#endif

};

#endif
