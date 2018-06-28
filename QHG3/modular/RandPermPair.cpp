#include <omp.h>
#include <math.h>
#include <algorithm>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "RandPermPair.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandPermPair<T>::RandPermPair(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL) {
 
    int iNumCells = this->m_pCG->m_iNumCells;

    m_vLocFemalesID = new std::vector<int>[iNumCells];
    m_vLocMalesID = new std::vector<int>[iNumCells];
 
#ifdef OMP_A
    m_aFLocks = new omp_lock_t[iNumCells];
    m_aMLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
    }
#endif

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RandPermPair<T>::~RandPermPair() {
    
    if (m_vLocFemalesID != NULL) {
        delete[] m_vLocFemalesID;
    }

    if (m_vLocMalesID != NULL) {
        delete[] m_vLocMalesID;
    }

#ifdef OMP_A
    if (m_aFLocks != NULL) {
        delete[] m_aFLocks;
    }
    if (m_aMLocks != NULL) {
        delete[] m_aMLocks;
    }
#endif

}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int RandPermPair<T>::initialize(float fT) {
    
    int iResult = 0;

#ifdef OMP_A
#pragma omp parallel for 
#endif
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_vLocFemalesID[iC].clear();
        m_vLocMalesID[iC].clear();
    }

   int iFirstAgent = this->m_pPop->getFirstAgentIndex();
   int iLastAgent  = this->m_pPop->getLastAgentIndex();

#ifdef OMP_A
#pragma omp parallel for 
#endif
   for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        this->m_pPop->m_aAgents[iA].m_iMateIndex = -3;
   }

    iResult = findMates();

    return iResult;
}

//-----------------------------------------------------------------------------
// finalize
// reset 
//
template<typename T>
int RandPermPair<T>::finalize(float fT) {
    
    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T>
int RandPermPair<T>::findMates() {

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();


    // fill local male and female index vectors
#pragma omp parallel for
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

        T* pA = &(this->m_pPop->m_aAgents[iA]);
        
        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            
            if (pA->m_iLifeState == LIFE_STATE_FERTILE) { 
                if (pA->m_iGender == 0)  { // FEMALE
                    
                    omp_set_lock(&m_aFLocks[iC]);
                    m_vLocFemalesID[iC].push_back(iA);
                    omp_unset_lock(&m_aFLocks[iC]);
                } else if (pA->m_iGender == 1) { // MALE

                    omp_set_lock(&m_aMLocks[iC]);
                    m_vLocMalesID[iC].push_back(iA);
                    omp_unset_lock(&m_aMLocks[iC]);
                }
            }
            
        }
    }


    // maximum pairing
#pragma omp for schedule(dynamic)
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            
        if (this->m_pPop->getNumAgents(iC) > 1) {
            int iFNum = (int)m_vLocFemalesID[iC].size();
            int iMNum = (int)m_vLocMalesID[iC].size();
            
            if (iFNum > 0 && iMNum > 0) {
                int iT = omp_get_thread_num();
                // sort to ensure reproducibility of simulation with same random seeds
                // this should be faster than using STL sets 
                // because insertions at the end of an STL vector are fast
                std::sort(m_vLocFemalesID[iC].begin(),  m_vLocFemalesID[iC].end());
                std::sort(m_vLocMalesID[iC].begin(), m_vLocMalesID[iC].end()); 
                
                // let's pair them up
                int iNumPaired = 0;
                if (iFNum <= iMNum) {
                    
                    iNumPaired = iFNum;
                    permute(m_vLocMalesID[iC], iFNum, iT);
                    
                } else {
                    
                    iNumPaired = iMNum;
                    permute(m_vLocFemalesID[iC], iMNum, iT);
                    
                }
                
                for (int k = 0; k < iNumPaired; k++) {
                    int iIDF = m_vLocFemalesID[iC][k];
                    int iIDM = m_vLocMalesID[iC][k];
                    
                    this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                    this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                    
                } 
            }
        }
    }        

    return 0;
}


//-----------------------------------------------------------------------------
// permute
//   fills the first iNumSel plaxces of vData with a random selection
//   picked from [0, iNumTot-1]
//   If iNumTot == iNumSel, vData is filled with a random permutation of
//   the integers from [0, iNumTot-1]
// (note: this works with vectors of every type)
//
template<typename T>
void RandPermPair<T>::permute(std::vector<int> &vData, uint iNumSel, int iT){

    // now exchange the first iNumSel elements with other random elements
    for (uint i = 0; i < iNumSel; ++i) {
        uint k = m_apWELL[iT]->wrandi(i, vData.size());
        int s = vData[k];
        vData[k] = vData[i];
        vData[i] = s;
    }
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void RandPermPair<T>::showAttributes() {
    printf("  (none)\n");
}
