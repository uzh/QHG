#include <omp.h>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "RandomMove1D.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandomMove1D<T>::RandomMove1D(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dMoveProb(0),
      m_bAbsorbing(false) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RandomMove1D<T>::~RandomMove1D() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int RandomMove1D<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iCellIndex = pa->m_iCellIndex;

    int iThread = omp_get_thread_num();

    // random number to compare with move probability
    double dR = this->m_apWELL[iThread]->wrandd();

    // do we move ?
    if (dR < m_dMoveProb) {

        SCell &sc = this->m_pCG->m_aCells[iCellIndex];

        double dR1 = this->m_apWELL[iThread]->wrandd();

        if (dR1 < 0.5) {
            int iNewIndex = sc.m_aNeighbors[0];
            if (iNewIndex >= 0) {
                this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewIndex);
            }
        } else {
            int iNewIndex = sc.m_aNeighbors[2];
            if (iNewIndex >= 0) {
                this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewIndex);
            }
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int RandomMove1D<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_extractAttribute(hSpeciesGroup, RANDOMMOVE1D_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int RandomMove1D<T>::writeParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, RANDOMMOVE1D_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int RandomMove1D<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, RANDOMMOVE1D_PROB_NAME, &m_dMoveProb);

   return iResult;
}

