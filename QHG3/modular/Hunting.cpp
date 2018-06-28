#include <string.h>
#include <omp.h>
#include <math.h>

#include <algorithm>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "WELL512.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "MassInterface.h"
#include "PopFinder.h"
#include "Hunting.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Hunting<T>::Hunting(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, PopFinder *pPopFinder)
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dEfficiency(0),
      m_dUsability(0),
      m_pMIPrey(NULL),
      m_pMIPred(NULL),
      m_pPopFinder(pPopFinder),
      m_pPreyPop(NULL) {
    
    *m_sPreyPopName = '\0';
    
   int iNumCells = this->m_pCG->m_iNumCells;

    m_vLocPredIdx = new std::vector<int>[iNumCells];
    m_vLocPreyIdx = new std::vector<int>[iNumCells];

#ifdef OMP_A
    m_aPredLocks = new omp_lock_t[iNumCells];
    m_aPreyLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aPredLocks[i]);
        omp_init_lock(&m_aPreyLocks[i]);
    }
#endif


}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Hunting<T>::~Hunting() {
    
    if (m_vLocPredIdx != NULL) {
        delete[] m_vLocPredIdx;
    }

    if (m_vLocPreyIdx != NULL) {
        delete[] m_vLocPreyIdx;
    }

#ifdef OMP_A
    if (m_aPredLocks != NULL) {
        delete[] m_aPredLocks;
    }
    if (m_aPreyLocks != NULL) {
        delete[] m_aPreyLocks;
    }
#endif

}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int Hunting<T>::preLoop() {
    int iResult = -1;
    m_pPreyPop = m_pPopFinder->getPopByName(m_sPreyPopName);
    if (m_pPreyPop != NULL) {
        printf("[Hunting<T>::preLoop()] got pointer for [%s]: %p\n", m_sPreyPopName, m_pPreyPop);
        m_pMIPrey = dynamic_cast<MassInterface *>(m_pPreyPop);
        if (m_pMIPrey != NULL) {
            m_pMIPred = dynamic_cast<MassInterface *>(this->m_pPop);
            if (m_pMIPred != NULL) {
                iResult = 0;
            }
        }
    } else {
        printf("[Hunting<T>::preLoop()] couldn't find population for species name [%s]\n", m_sPreyPopName);
    }

    return iResult;
}
    

//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int Hunting<T>::initialize(float fT) {
    
    int iResult = 0;

#ifdef OMP_A
#pragma omp parallel for 
#endif
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_vLocPredIdx[iC].clear();
        m_vLocPreyIdx[iC].clear();
    }
    

    // collect predators
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();
    
    if (iFirstAgent >= 0) {
#ifdef OMP_A
#pragma omp parallel for 
#endif
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
            
            T* pA = &(this->m_pPop->m_aAgents[iA]);
            pA->m_iPreyIndex = -3;
            if (pA->m_iLifeState > LIFE_STATE_DEAD) {
                
                int iC = pA->m_iCellIndex;
                
#ifdef OMP_A
                omp_set_lock(&m_aPredLocks[iC]);
#endif
                m_vLocPredIdx[iC].push_back(iA);
#ifdef OMP_A
                omp_unset_lock(&m_aPredLocks[iC]);
#endif
                
            }
        }
    }

    // collect prey
    iFirstAgent = this->m_pPreyPop->getFirstAgentIndex();
    iLastAgent  = this->m_pPreyPop->getLastAgentIndex();
    if (iFirstAgent >= 0) {
#ifdef OMP_A
#pragma omp parallel for 
#endif
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
            
            if (this->m_pPreyPop->getAgentLifeState(iA) > LIFE_STATE_DEAD) {
                
                int iC = this->m_pPreyPop->getAgentCellIndex(iA);
                
#ifdef OMP_A
                omp_set_lock(&m_aPreyLocks[iC]);
#endif
                m_vLocPreyIdx[iC].push_back(iA);
#ifdef OMP_A
                omp_unset_lock(&m_aPreyLocks[iC]);
#endif
                
            }
        }
    }

    // now we have the prey and predator vectors
    // now allocate prey to predator
    
    
#ifdef OMP_A
#pragma omp for schedule(static,1)
#endif
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        

        if (this->m_pPop->getNumAgents(iC) > 1) {
            int iPreyNum = (int)m_vLocPreyIdx[iC].size();
            int iPredNum = (int)m_vLocPredIdx[iC].size();
            
            if ((iPreyNum > 0) && (iPredNum > 0)) {
                
                // sort to ensure reproducibility of simulation with same random seeds
                // this should be faster than using STL sets 
                // because insertions at the end of an STL vector are fast
                std::sort(m_vLocPreyIdx[iC].begin(),  m_vLocPreyIdx[iC].end());
                std::sort(m_vLocPredIdx[iC].begin(), m_vLocPredIdx[iC].end()); 
                
                bool* bPreyServed = new bool[iPreyNum];
                memset(bPreyServed,0,iPreyNum*sizeof(bool));
                
                int iPred = 0;
                while ((iPred < iPredNum) && (iPreyNum > 0)) {
                    int iChoose = (int)(this->m_apWELL[omp_get_thread_num()]->wrandr(0,iPreyNum)); // which uncoupled male do we want?
                    int iPreySearch = -1;  // count how many uncoupled preay we encouter in the vector
                    int iPrey = -1;  // go through local prey vector
                    
                    while (iPreySearch < iChoose) {
                        iPrey++;
                        if ( ! bPreyServed[iPrey] ) {
                            iPreySearch++;
                        }
                    }
                    bPreyServed[iPrey] = true; // mark prey as already assigned 
                    
                    int iIdxPrey = m_vLocPreyIdx[iC][iPrey];
                    //                    printf("C%d: Pred #%d (%d) has prey #%d\n", iC, iPred, m_vLocPredIdx[iC][iPred], iIdxPrey);
                    this->m_pPop->m_aAgents[m_vLocPredIdx[iC][iPred]].m_iPreyIndex = iIdxPrey;
                    iPreyNum--;
                    iPred++;
                }
                delete[] bPreyServed;
            }
        }
    }

    /*
    iFirstAgent = this->m_pPop->getFirstAgentIndex();
    iLastAgent  = this->m_pPop->getLastAgentIndex();
    if (iFirstAgent >= 0) {
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
            
            T* pA = &(this->m_pPop->m_aAgents[iA]);
            if (pA->m_iLifeState > 0) {
                int iC = pA->m_iCellIndex;
                printf("Repeat: C%d: Pred #%d has prey #%d\n", iC, iA, pA->m_iPreyIndex);
                
            }
        }
    }
    */
    return iResult;
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int Hunting<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if ((pa->m_iLifeState > 0) && (pa->m_iPreyIndex >= 0) &&  (this->m_pPreyPop->getAgentLifeState(pa->m_iPreyIndex) > LIFE_STATE_DEAD)) {
        
        double dRB = this->m_apWELL[omp_get_thread_num()]->wrandd();
        if (dRB < m_dEfficiency) {
            double dM = m_pMIPrey->getMass(pa->m_iPreyIndex);
            m_pMIPred->addMass(iAgentIndex, dM*m_dUsability);
            m_pPreyPop->registerDeath(pa->m_iCellIndex, pa->m_iPreyIndex);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_HUNTING_EFFICIENCY_NAME,
//    ATTR_HUNTING_USABILITY_NAME, 
//    ATTR_HUNTING_PREYSPECIES_NAME
//
template<typename T>
int Hunting<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_HUNTING_EFFICIENCY_NAME,  1, &m_dEfficiency);
        if (iResult != 0) {
            LOG_ERROR("[Hunting] couldn't read attribute [%s]", ATTR_HUNTING_EFFICIENCY_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_HUNTING_USABILITY_NAME,   1, &m_dUsability);
        if (iResult != 0) {
            LOG_ERROR("[Hunting] couldn't read attribute [%s]", ATTR_HUNTING_USABILITY_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_HUNTING_PREYSPECIES_NAME, NAME_LEN, m_sPreyPopName);
        if (iResult != 0) {
            LOG_ERROR("[Hunting] couldn't read attribute [%s]", ATTR_HUNTING_PREYSPECIES_NAME);
        }
    }
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_HUNTING_EFFICIENCY_NAME,
//    ATTR_HUNTING_USABILITY_NAME, 
//    ATTR_HUNTING_PREYSPECIES_NAME
//
template<typename T>
int Hunting<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_HUNTING_EFFICIENCY_NAME,  1, &m_dEfficiency);
    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_HUNTING_USABILITY_NAME,   1, &m_dUsability);
    iResult += qdf_insertSAttribute(hSpeciesGroup, ATTR_HUNTING_PREYSPECIES_NAME, m_sPreyPopName);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int Hunting<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, ATTR_HUNTING_EFFICIENCY_NAME, &m_dEfficiency);
   iResult += this->readPopKeyVal(pLine, ATTR_HUNTING_USABILITY_NAME,  &m_dUsability);
   char *pVal = readKeyString(pLine, ATTR_HUNTING_PREYSPECIES_NAME, "=");
   if (pVal != NULL) {
       strncpy(m_sPreyPopName, pVal, NAME_LEN);
       iResult += 1;
   } else {
       iResult += 0;
   }
      
   return iResult;
 }


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void Hunting<T>::showAttributes() {
    printf("  %s\n", ATTR_HUNTING_EFFICIENCY_NAME);
    printf("  %s\n", ATTR_HUNTING_USABILITY_NAME);
    printf("  %s\n", ATTR_HUNTING_PREYSPECIES_NAME);
}
