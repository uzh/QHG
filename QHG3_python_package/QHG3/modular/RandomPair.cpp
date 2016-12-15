#include <omp.h>
#include <math.h>
#include <algorithm>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "RandomPair.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandomPair<T>::RandomPair(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL) 
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
RandomPair<T>::~RandomPair() {
    
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
int RandomPair<T>::initialize(float fT) {
    
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
int RandomPair<T>::finalize(float fT) {
    
    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T>
int RandomPair<T>::findMates() {

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();


#ifdef OMP_A
#pragma omp parallel 
#endif
    {
#ifdef OMP_A
//	int iChunk = (int)ceil((iLastAgent-iFirstAgent+1)/(double)omp_get_num_threads());
//#pragma omp for schedule(static,iChunk)
#pragma omp for
#endif
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

            T* pA = &(this->m_pPop->m_aAgents[iA]);
            
            if (pA->m_iLifeState > 0) {

                int iC = pA->m_iCellIndex;

                if (pA->m_iGender == 0) { // FEMALE
#ifdef OMP_A
                    omp_set_lock(&m_aFLocks[iC]);
#endif
                    m_vLocFemalesID[iC].push_back(iA);
#ifdef OMP_A
                    omp_unset_lock(&m_aFLocks[iC]);
#endif

                } else if (pA->m_iGender == 1) { // MALE

#ifdef OMP_A
                    omp_set_lock(&m_aMLocks[iC]);
#endif
                    m_vLocMalesID[iC].push_back(iA);
#ifdef OMP_A
                    omp_unset_lock(&m_aMLocks[iC]);
#endif
                }
            }
        }


#ifdef OMP_A
#pragma omp for schedule(static,1)
#endif
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        
            if (this->m_pPop->getNumAgents(iC) > 1) {
                int iFNum = (int)m_vLocFemalesID[iC].size();
                int iMNum = (int)m_vLocMalesID[iC].size();
    
                if (iFNum > 0 && iMNum > 0) {
    
                    // sort to ensure reproducibility of simulation with same random seeds
                    // this should be faster than using STL sets 
                    // because insertions at the end of an STL vector are fast
                    std::sort(m_vLocFemalesID[iC].begin(),  m_vLocFemalesID[iC].end());
                    std::sort(m_vLocMalesID[iC].begin(), m_vLocMalesID[iC].end()); 
                    
                    // let's pair them up
    
                    if (iFNum <= iMNum) {

                        // here we'll indicate if a male is "taken"
                        bool* bM = new bool[iMNum];
                        memset(bM,0,iMNum*sizeof(bool));
                        
                        int iF = 0;

                        while (iF < iFNum && iMNum > 0) {
                            int iChoose = (int)(this->m_apWELL[omp_get_thread_num()]->wrandr(0,iMNum)); // which uncoupled male do we want?
                            int iMSearch = -1;  // count how many uncoupled males we encouter in the vector
                            int iM = -1;  // go through local males vector
                            
                            while (iMSearch < iChoose) {
                                iM++;
                                if ( ! bM[iM] ) {
                                    iMSearch++;
                                }
                            }
                            
                            bM[iM] = true; // mark male as already coupled 
                            
                            int iIDF = m_vLocFemalesID[iC][iF];
                            int iIDM = m_vLocMalesID[iC][iM];
                            
                            this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                            this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                            
                            iMNum--;
                            iF++;
                        }
                        delete[] bM;
                        
                    } else {
                        
                        // here we'll indicate if a female is "taken"
                        bool* bF = new bool[iFNum];
                        memset(bF,0,iFNum*sizeof(bool));
                
                        int iM = 0;

                        while (iM < iMNum && iFNum > 0) {
                            int iChoose = (int)(this->m_apWELL[omp_get_thread_num()]->wrandr(0,iFNum)); // which uncoupled female do we want?
                            int iFSearch = -1;  // count how many uncoupled females we encouter in the vector
                            int iF = -1;  // go through local females vector
                            
                            while (iFSearch < iChoose) {
                                iF++;
                                if ( ! bF[iF] ) {
                                    iFSearch++;
                                }
                            }
                            
                            bF[iF] = true; // mark female as already coupled 
                            
                            int iIDF = m_vLocFemalesID[iC][iF];
                            int iIDM = m_vLocMalesID[iC][iM];
                            
                            this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                            this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                            
                            iFNum--;
                            iM++;
                        }
                        delete[] bF;                   

                    } 
                }
            }
        }
    }        

    return 0;
}


