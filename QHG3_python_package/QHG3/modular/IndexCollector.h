#ifndef __INDEXCOLLECTOR_H__
#define __INDEXCOLLECTOR_H__

#include "Action.h"

#define INDEXCOLLECTOR_NAME     "IndexCollector"


template<typename T>
class IndexCollector : public Action<T> {
public:
    IndexCollector(SPopulation<T> *pPop, SCellGrid *pCG, const char *pShareName);
    virtual ~IndexCollector();

    int preLoop();
    int initialize(float fTime);
    
    void setShareName(const char *pShareName);


protected:
    std::string       m_sShareName;
    std::vector<int> *m_vLocalIndexes;


#ifdef OMP_A
    omp_lock_t* m_aIndexLocks;
#endif

};

#endif

