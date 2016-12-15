#include <omp.h>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "WeightedMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
WeightedMove<T>::WeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adEnvWeights) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_adEnvWeights(adEnvWeights),
      m_dMoveProb(0) {

    // nothing to be done here
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
WeightedMove<T>::~WeightedMove() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int WeightedMove<T>::operator()(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iNewIndex = -1; 
    
    if (pa->m_iLifeState > 0) {
        
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();

        if (dR < m_dMoveProb) {
    
            int iCellIndex = pa->m_iCellIndex;
        
            int iMaxNeighbors = this->m_pCG->m_iConnectivity;
        
            int iOffset = iCellIndex*(iMaxNeighbors+1);

            // count how many neighbors are not eligible for movement
            int iInaccessible = 0;
            for (int i = iOffset+1; i < iOffset+iMaxNeighbors+1; i++) {
                if (m_adEnvWeights[i] == m_adEnvWeights[i - 1]) {
                    iInaccessible++;
                }
            }
            double dR1 = m_apWELL[iThread]->wrandd();

            if (dR1 > iInaccessible / (double)iMaxNeighbors) {

                // get a random number between current cell's and max
                double dR2 =  m_adEnvWeights[iOffset] + 
                    m_apWELL[iThread]->wrandd() * (m_adEnvWeights[iOffset + iMaxNeighbors] - m_adEnvWeights[iOffset]);
                
                int iI = 0;
                while (iI < iMaxNeighbors + 1) {
                    if (dR2 < m_adEnvWeights[iOffset + iI]) {
                        iNewIndex = iI;
                        iI = iMaxNeighbors + 1;
                    } else {
                        iI++;
                    }
                }
                
                if (iNewIndex > 0) {
                    int iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewIndex-1];
                    if ((this->m_pCG->m_pGeography == NULL) || (!this->m_pCG->m_pGeography->m_abIce[iCellIndex])) {
                        this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                    }
                }
            }
        }
        
    }
    
    return (iNewIndex > -1) ? 0 : -1;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int WeightedMove<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_extractAttribute(hSpeciesGroup, WEIGHTEDMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int WeightedMove<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, WEIGHTEDMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int WeightedMove<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, WEIGHTEDMOVE_PROB_NAME, &m_dMoveProb);

   return iResult;
}

