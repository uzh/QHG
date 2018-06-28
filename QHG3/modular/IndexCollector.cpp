#include <string.h>
#include <omp.h>
#include <math.h>

#include <algorithm>

#include "clsutils.cpp"
#include "ArrayShare.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "IndexCollector.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
IndexCollector<T>::IndexCollector(SPopulation<T> *pPop, SCellGrid *pCG, const char *pShareName)
    : Action<T>(pPop,pCG),
      m_sShareName("") {
    
    if (pShareName != NULL) {
        m_sShareName = pShareName;
    }

    int iNumCells = this->m_pCG->m_iNumCells;

    m_vLocalIndexes = new std::vector<int>[iNumCells];


#ifdef OMP_A
    m_aIndexLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aIndexLocks[i]);
    }
#endif


}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
IndexCollector<T>::~IndexCollector() {
    
    if (m_vLocalIndexes != NULL) {
        delete[] m_vLocalIndexes;
    }
#ifdef OMP_A
    if (m_aIndexLocks != NULL) {
        delete[] m_aIndexLocks;
    }
#endif
}


//-----------------------------------------------------------------------------
// setShareName
//
template<typename T>
void IndexCollector<T>::setShareName(const char *pShareName) {
    if (pShareName != NULL) {
        m_sShareName = pShareName;
    }
}


//-----------------------------------------------------------------------------
// preLoop
//  prepare mass-interfaces and share preyratio array
//
template<typename T>
int IndexCollector<T>::preLoop() {
    int iResult = 0;

    //share the array
    ArrayShare::getInstance()->shareArray(m_sShareName.c_str(), this->m_pCG->m_iNumCells, m_vLocalIndexes);
    printf("[IndexCollector<T>::preLoop] shared index array as [%s]\n", m_sShareName.c_str());

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int IndexCollector<T>::initialize(float fT) {
    printf("[IndexCollector<T>::initialize] collecting indexes for [%s] at T %f\n", this->m_pPop->getSpeciesName(), fT);

    int iResult = 0;

#ifdef OMP_A
#pragma omp parallel for 
#endif
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_vLocalIndexes[iC].clear();
    }

    // collect predators
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();
    
    // loop through agents and assign to appropriate vector
    if (iFirstAgent >= 0) {
#ifdef OMP_A
#pragma omp parallel for 
#endif
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
            
            T* pA = &(this->m_pPop->m_aAgents[iA]);
    
            if (pA->m_iLifeState > LIFE_STATE_DEAD) {
                
                int iC = pA->m_iCellIndex;
                
#ifdef OMP_A
                omp_set_lock(&m_aIndexLocks[iC]);
#endif
                m_vLocalIndexes[iC].push_back(iA);
#ifdef OMP_A
                omp_unset_lock(&m_aIndexLocks[iC]);
#endif
                
            }
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void IndexCollector<T>::showAttributes() {
    printf("  (none)\n");
}
