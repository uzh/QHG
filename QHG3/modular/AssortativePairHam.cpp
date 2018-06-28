#include <string.h>
#include <omp.h>
#include <math.h>
#include <algorithm>

//#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "Genetics.h"
#include "AssortativePairHam.h"

#include "MessLogger.h"
#include "RankTable.h"



//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
AssortativePairHam<T,U>::AssortativePairHam(SPopulation<T> *pPop, SCellGrid *pCG, Genetics<T,U> *pGenetics, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_pGenetics(pGenetics),
      m_apWELL(apWELL),
      m_fCutOff(0.0),
      m_bPermute(0) {
   
    // Genetic's genome size is probably not set yet - get it later
    m_iNumCells     = this->m_pCG->m_iNumCells;
    m_vLocFemalesID = new std::vector<int>[m_iNumCells];
    m_vLocMalesID   = new std::vector<int>[m_iNumCells];

#ifdef OMP_A
 m_aFLocks = new omp_lock_t[m_iNumCells];
    m_aMLocks = new omp_lock_t[m_iNumCells];
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
    }
#endif

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T, class U>
AssortativePairHam<T,U>::~AssortativePairHam() {
    
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
template<typename T, class U>
int AssortativePairHam<T,U>::preLoop() {
    m_iNumCells   = this->m_pCG->m_iNumCells;
    m_iGenomeSize  = m_pGenetics->getGenomeSize();
    m_iNumBlocks  = U::numNucs2Blocks(m_iGenomeSize);
    printf("[AssortativePairHam<T,U>::preLoop] numcells %d, genomesize %d, numblocks %d\n", m_iNumCells, m_iGenomeSize, m_iNumBlocks);
    return 0;
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T, class U>
int AssortativePairHam<T,U>::initialize(float fT) {
  
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
int AssortativePairHam<T,U>::finalize(float fT) {
    
    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T, class U>
int AssortativePairHam<T,U>::findMates() {
    int iResult = 0;
    
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();
    float fBuildMatch[omp_get_max_threads()];
    memset(fBuildMatch, 0, omp_get_max_threads()*sizeof(float));
    float fRanking[omp_get_max_threads()];
    memset(fRanking, 0, omp_get_max_threads()*sizeof(float));
    float fCreation[omp_get_max_threads()];
    memset(fCreation, 0, omp_get_max_threads()*sizeof(float));
    float fCompare[omp_get_max_threads()];
    memset(fCompare, 0, omp_get_max_threads()*sizeof(float));
    float fTranslate[omp_get_max_threads()];
    memset(fTranslate, 0, omp_get_max_threads()*sizeof(float));

#ifdef OMP_A
#pragma omp parallel 
#endif
    {
#ifdef OMP_A
//	int iChunk = (int)ceil((iLastAgent-iFirstAgent+1)/(double)omp_get_num_threads());
//#pragma omp for schedule(static,iChunk)
#pragma omp for schedule(dynamic)
#endif
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

            T* pA = &(this->m_pPop->m_aAgents[iA]);
            
            if (pA->m_iLifeState > 0) {

                int iC = pA->m_iCellIndex;
                if  (pA->m_iLifeState == LIFE_STATE_FERTILE) {

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
        }
    }
    // now we have male and female vectors of agent indexes for each cell
#pragma omp parallel for schedule(dynamic)
    for (int iC = 0; iC < m_iNumCells; iC++) {

        if ((m_vLocFemalesID[iC].size() > 0) && (m_vLocMalesID[iC].size() > 0)) {
            //            printf("%d:Cell %d: %zd females, %zd males\n", iT,iC, m_vLocFemalesID[iC].size(), m_vLocMalesID[iC].size());
            // allocat match array
            float **ppMatch = new float*[m_vLocFemalesID[iC].size()];
            for (uint iF = 0; (iResult == 0) && (iF < m_vLocFemalesID[iC].size()); ++iF) {
                ppMatch[iF] = new float[m_vLocMalesID[iC].size()];
                memset(ppMatch[iF], 0, m_vLocMalesID[iC].size()*sizeof(float));
            }

            // fill the match array using ProteinBuilder and ProteomeComparator
            for (uint iF = 0; (iResult == 0) && (iF < m_vLocFemalesID[iC].size()); ++iF) {
                ulong *pGenomeF = m_pGenetics->getGenome(m_vLocFemalesID[iC][iF]);
                for (uint iM = 0; (iResult == 0) && (iM < m_vLocMalesID[iC].size()); ++iM) {
                    ulong *pGenomeM = m_pGenetics->getGenome(m_vLocMalesID[iC][iM]);
                    // the matching part is everything without the different parts
                    // match entry must be between 0 (no match) and 1 (perfect match)
                    ppMatch[iF][iM] = 1.0 - (1.0* U::calcDist(pGenomeF, pGenomeM, m_iNumBlocks))/m_iGenomeSize;
                }
            }


            // use RankTable to find pairings
            
            RankTable *pRT = RankTable::createInstance( m_vLocFemalesID[iC].size(), m_vLocMalesID[iC].size(), m_apWELL, ppMatch);
            if (pRT != NULL) {
                pRT->setVerbosity(false);
                
                
                //            pRT->display();
                iResult = pRT->makeAllPairings(m_fCutOff, m_bPermute);
                
                const couples &vc = pRT->getPairs();
                for (uint i = 0; i < vc.size(); ++i) {
                    int iIDF = m_vLocFemalesID[iC][vc[i].first];
                    int iIDM = m_vLocMalesID[iC][vc[i].second];
                    
                    this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                    this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                    
                }
                delete pRT;    
            } else {
                printf("Xouldn't create RankTable\n");
            }
            
            // clean up ppMatch
            for (uint i = 0; i < m_vLocFemalesID[iC].size(); ++i) {
                delete[] ppMatch[i];
            }
            delete[] ppMatch;
        }

    }

    return iResult;
}





//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_ASSPAIRHAM_CUTOFF 
//    ATTR_ASSPAIRHAM_PERMUTE
//
template<typename T, class U>
int AssortativePairHam<T,U>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ASSPAIRHAM_CUTOFF,  1,  &m_fCutOff);
    }
    if (iResult == 0) {
        int iPermute = m_bPermute;
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ASSPAIRHAM_PERMUTE,  1, &iPermute);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_ASSPAIRHAM_CUTOFF, 
//    ATTR_ASSPAIRHAM_PERMUTE
//
template<typename T, class U>
int AssortativePairHam<T,U>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ASSPAIRHAM_CUTOFF,  1,  &m_fCutOff);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ASSPAIRHAM_CUTOFF);
        }
    }

    if (iResult == 0) {

        int iPermute = 0;
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ASSPAIRHAM_PERMUTE,  1, &iPermute);
        m_bPermute = iPermute;
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ASSPAIRHAM_PERMUTE);
        }
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T, class U>
int AssortativePairHam<T,U>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;
    
    iResult += this->readPopKeyVal(pLine, ATTR_ASSPAIRHAM_CUTOFF,         &m_fCutOff);

    int iPermute = 0;
    iResult += this->readPopKeyVal(pLine, ATTR_ASSPAIRHAM_PERMUTE,        &iPermute);
    if (iPermute != 0) {
        m_bPermute = true;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T, class U>
void AssortativePairHam<T,U>::showAttributes() {
    printf("  %s\n", ATTR_ASSPAIRHAM_CUTOFF);
    printf("  %s\n", ATTR_ASSPAIRHAM_PERMUTE);
}
