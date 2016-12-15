#include <omp.h>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "RandomMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandomMove<T>::RandomMove(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dMoveProb(0),
      m_bAbsorbing(false) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RandomMove<T>::~RandomMove() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int RandomMove<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iCellIndex = pa->m_iCellIndex;

    int iThread = omp_get_thread_num();

    // random number to compare with move probability
    double dR = this->m_apWELL[iThread]->wrandd();

    // do we move ?
    if (dR < m_dMoveProb) {

        SCell &sc = this->m_pCG->m_aCells[iCellIndex];

        if (!m_bAbsorbing) {
            int iNewIndex0 = (int)(this->m_apWELL[iThread]->wrandr(0, sc.m_iNumNeighbors));
            int i=-1;
            int j=-1;
            while (i < iNewIndex0) {
                j++;
                if (sc.m_aNeighbors[j] >= 0) {
                    i++;
                }
            }
            int iNewIndex = sc.m_aNeighbors[j];
            this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewIndex);
        } else {
            int iNewIndex0 = (int)(this->m_apWELL[iThread]->wrandr(0, sc.m_iNumNeighbors));
            int iNewIndex = sc.m_aNeighbors[iNewIndex0];
            if(iNewIndex < 0) {
                this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            } else {
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
int RandomMove<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_extractAttribute(hSpeciesGroup, RANDOMMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int RandomMove<T>::writeParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, RANDOMMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int RandomMove<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, RANDOMMOVE_PROB_NAME, &m_dMoveProb);

   return iResult;
}

