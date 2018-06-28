#include <omp.h>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "WeightedMoveRand.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
WeightedMoveRand<T>::WeightedMoveRand(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adEnvWeights) 
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
WeightedMoveRand<T>::~WeightedMoveRand() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int WeightedMoveRand<T>::operator()(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iNewIndex = -1; 
    
    if (pa->m_iLifeState > 0) {
        
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();

        if (dR < m_dMoveProb) {
    
            int iCellIndex = pa->m_iCellIndex;
        
            int iMaxNeighbors = this->m_pCG->m_iConnectivity;
        
            int iOffset = iCellIndex*(iMaxNeighbors+1);
       
            if ( m_adEnvWeights[iOffset + iMaxNeighbors] > 0) {
                // get a random number between 0 and max 
                double dR2 =  m_apWELL[iThread]->wrandd() * m_adEnvWeights[iOffset + iMaxNeighbors];
                
                int iI = 0;
                while (iI < iMaxNeighbors + 1) {
                    if (dR2 < m_adEnvWeights[iOffset + iI]) {
                        iNewIndex = iI;
                        iI = iMaxNeighbors + 1;
                    } else {
                        iI++;
                    }
                }
       
            } else {
                int iNumActualNeigh = this->m_pCG->m_aCells[iCellIndex].m_iNumNeighbors;
                // do a random move if everything's zero
                iNewIndex = (int) m_apWELL[iThread]->wrandr(0, iNumActualNeigh+1);
            }

            if (iNewIndex > 0) {
                int iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewIndex-1];
                if ((this->m_pCG->m_pGeography == NULL) || (!this->m_pCG->m_pGeography->m_abIce[iCellIndex])) {
                    // on flat grid, some neighbo cells have "ID" -1
                    if (iNewCellIndex >= 0) {
                        this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                        /*
                        printf("Moving %p from (%f,%f) to (%f,%f)\n", pa,  
                               this->m_pCG->m_pGeography->m_adLongitude[iCellIndex],
                               this->m_pCG->m_pGeography->m_adLatitude[iCellIndex],
                               this->m_pCG->m_pGeography->m_adLongitude[iNewCellIndex],
                               this->m_pCG->m_pGeography->m_adLatitude[iNewCellIndex]);
                        */
                    }

                    /*
                    if (iNewCellIndex < 0) {
                        printf("vvvvv[WeightedMoveRand<T>::operator()]Negative target from cell %d\n", iCellIndex);
                        printf("vvvvv[WeightedMoveRand<T>::operator()]m_adEnvweights (offset %d)\n", iOffset);
                        for (int z = 0; z < iMaxNeighbors + 1; z++) {
                            printf("vvvvv  %.4f\n",  m_adEnvWeights[iOffset + z]);
                        }
                    }
                    */
                }
            }
        }
        
    }
    
    return (iNewIndex > -1) ? 0 : -1;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//  tries to read the atttribute
//    ATTR_WEIGHTEDMOVERAND_PROB_NAME
//
template<typename T>
int WeightedMoveRand<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_WEIGHTEDMOVERAND_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[WeightedMoveRand] couldn't read attribute [%s]", ATTR_WEIGHTEDMOVERAND_PROB_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//  tries to write the atttribute
//    ATTR_WEIGHTEDMOVERAND_PROB_NAME
//
template<typename T>
int WeightedMoveRand<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_WEIGHTEDMOVERAND_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int WeightedMoveRand<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, ATTR_WEIGHTEDMOVERAND_PROB_NAME, &m_dMoveProb);

   return iResult;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void WeightedMoveRand<T>::showAttributes() {
    printf("  %s\n", ATTR_WEIGHTEDMOVERAND_PROB_NAME);
}
