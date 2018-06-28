#include <omp.h>
#include <math.h>
#include <algorithm>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "WELL512.h"
#include "Genetics.h"
#include "MessLogger.h"
#include "RandDistrPair.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
RandDistrPair<T,U>::RandDistrPair(SPopulation<T> *pPop, SCellGrid *pCG, Genetics<T, U> *pGenetics, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_pGenetics(pGenetics),
      m_ppPermutators(NULL),
      m_iNumCells(0),
      m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_fDCrit(1),
      m_fVCrit(0),
      m_fA(-1) {
 
    m_iNumCells     = this->m_pCG->m_iNumCells;

    m_vLocFemalesID = new std::vector<int>[m_iNumCells];
    m_vLocMalesID = new std::vector<int>[m_iNumCells];
 
    m_ppPermutators = new Permutator *[m_iNumCells];
#ifdef OMP_A
    m_aFLocks = new omp_lock_t[m_iNumCells];
    m_aMLocks = new omp_lock_t[m_iNumCells];
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
        m_ppPermutators[i] = Permutator::createInstance(INIT_PERM_SIZE);
    }
#endif

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T, class U>
RandDistrPair<T,U>::~RandDistrPair() {
    
    if (m_vLocFemalesID != NULL) {
        delete[] m_vLocFemalesID;
    }

    if (m_vLocMalesID != NULL) {
        delete[] m_vLocMalesID;
    }

    if (m_ppPermutators != NULL) {
        for (int i = 0; i < m_iNumCells; i++) {
            if (m_ppPermutators[i] != NULL) {
                delete m_ppPermutators[i];
            }
        }
        delete[] m_ppPermutators;
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
// preLoop
// get relevant stuff from Genetics
//
template<typename T, class U>
int RandDistrPair<T,U>::preLoop() {
    m_iNumCells   = this->m_pCG->m_iNumCells;
    m_iGenomeSize  = m_pGenetics->getGenomeSize();
    m_iNumBlocks  = U::numNucs2Blocks(m_iGenomeSize);
    printf("[RandDistrPair<T,U>::preLoop] numcells %d, genomesize %d, numblocks %d\n", m_iNumCells, m_iGenomeSize, m_iNumBlocks);

    m_fA = (1-m_fVCrit)/m_fDCrit;

    return 0;
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T, class U>
int RandDistrPair<T,U>::initialize(float fT) {
    
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
template<typename T, class U>
int RandDistrPair<T,U>::finalize(float fT) {
    
    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T, class U>
int RandDistrPair<T,U>::findMates() {

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

                if (pA->m_iLifeState == LIFE_STATE_FERTILE) {
                    if (pA->m_iGender == 0)  { // FEMALE
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
        }
    }

#ifdef OMP_A
#pragma omp for schedule(dynamic)
#endif
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            
        if (this->m_pPop->getNumAgents(iC) > 1) {
            int iFNum = (int)m_vLocFemalesID[iC].size();
            int iMNum = (int)m_vLocMalesID[iC].size();
       
            if (iFNum > 0 && iMNum > 0) {
                int iT = omp_get_thread_num();
                std::vector<std::pair<uint, uint> > vPairs;
                if (iFNum < iMNum) {
                    uint *pPerm = m_ppPermutators[iC]->permute(iMNum, iFNum, m_apWELL[iT]);
                    for (int iF = 0; iF < iFNum; iF++) {
                        vPairs.push_back(std::pair<uint,uint>(m_vLocFemalesID[iC][iF], m_vLocMalesID[iC][pPerm[iF]]));
                    }
                } else {
                    uint *pPerm = m_ppPermutators[iC]->permute(iFNum, iMNum, m_apWELL[iT]);
                    for (int iM = 0; iM < iMNum; iM++) {
                        vPairs.push_back(std::pair<uint,uint>(m_vLocFemalesID[iC][pPerm[iM]], m_vLocMalesID[iC][iM]));
                    }
                }

                // now decide on the mating success
                for (uint i = 0; i < vPairs.size(); ++i) {
                    ulong *pGenomeF = m_pGenetics->getGenome(vPairs[i].first);
                    ulong *pGenomeM = m_pGenetics->getGenome(vPairs[i].second);
                    double dDist = (1.0* U::calcDist(pGenomeF, pGenomeM, m_iNumBlocks))/m_iGenomeSize;
                    double dP = (dDist > m_fDCrit)?m_fVCrit:1 - m_fA *dDist; // need other function here?
                    double dR = m_apWELL[iT]->wrandd();
                    if (dR < dP) {
                        this->m_pPop->m_aAgents[vPairs[i].first].m_iMateIndex  = vPairs[i].second;
                        this->m_pPop->m_aAgents[vPairs[i].second].m_iMateIndex = vPairs[i].first;
                    }
                }
                
            }
        }
    }
            

    return 0;
}

//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_RANDDISTRPAIR_DCRIT 
//    ATTR_RANDDISTRPAIR_VCRIT
//
template<typename T, class U>
int RandDistrPair<T,U>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_RANDDISTRPAIR_DCRIT,  1,  &m_fDCrit);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_RANDDISTRPAIR_VCRIT,  1,  &m_fVCrit);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_RANDDISTRPAIR_DCRIT 
//    ATTR_RANDDISTRPAIR_VCRIT
//
template<typename T, class U>
int RandDistrPair<T,U>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_RANDDISTRPAIR_DCRIT,  1,  &m_fDCrit);
        if (iResult != 0) {
            LOG_ERROR("[RandDistrPair] couldn't read attribute [%s]", ATTR_RANDDISTRPAIR_DCRIT);
        }
    }

    if (iResult == 0) {

        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_RANDDISTRPAIR_VCRIT,  1, &m_fVCrit);
        if (iResult != 0) {
            LOG_ERROR("[RandDistrPair] couldn't read attribute [%s]", ATTR_RANDDISTRPAIR_VCRIT);
        }
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T, class U>
int RandDistrPair<T,U>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;
    
    iResult += this->readPopKeyVal(pLine, ATTR_RANDDISTRPAIR_DCRIT,        &m_fDCrit);

    iResult += this->readPopKeyVal(pLine, ATTR_RANDDISTRPAIR_VCRIT,        &m_fVCrit);
 
    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T, class U>
void RandDistrPair<T,U>::showAttributes() {
    printf("  %s\n", ATTR_RANDDISTRPAIR_DCRIT);
    printf("  %s\n", ATTR_RANDDISTRPAIR_VCRIT);
}

