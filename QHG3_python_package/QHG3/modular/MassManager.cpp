#include <string.h>
#include <omp.h>
#include <math.h>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "MassInterface.h"
#include "MassManager.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
MassManager<T>::MassManager(SPopulation<T> *pPop, SCellGrid *pCG)
    : Action<T>(pPop,pCG),
      m_dMinMass(0),
      m_dMaxMass(0),
      m_dDelta(0),
      m_pMI(NULL) {
}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int MassManager<T>::preLoop() {
    int iResult = -1;
    m_pMI = dynamic_cast<MassInterface *>(this->m_pPop);
    if (m_pMI != NULL) {
        iResult = 0;
    } else {
        printf("[MassManager<T>::preLoop] Population must implement MassInterface\n");
    }
    return iResult;   
}


//-----------------------------------------------------------------------------
// operator()
//
template<typename T>
int MassManager<T>::operator()(int iAgentIndex, float fT) {
    int iResult = 0;

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > LIFE_STATE_DEAD) {
        double dM = m_pMI->getMass(iAgentIndex);
        dM += m_dDelta;
        if (dM < m_dMinMass) {
            int iCellIndex = pa->m_iCellIndex;
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
        } else if (dM > m_dMaxMass) {
            m_pMI->setMass(iAgentIndex, m_dMaxMass);
        } else {
            m_pMI->setMass(iAgentIndex, dM);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int MassManager<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_extractAttribute(hSpeciesGroup, MASSMANAGER_MIN_NAME,   1, &m_dMinMass);
    iResult += qdf_extractAttribute(hSpeciesGroup, MASSMANAGER_MAX_NAME,   1, &m_dMaxMass);
    iResult += qdf_extractAttribute(hSpeciesGroup, MASSMANAGER_DELTA_NAME, 1, &m_dDelta);   

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int MassManager<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, MASSMANAGER_MIN_NAME,   1, &m_dMinMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, MASSMANAGER_MAX_NAME,   1, &m_dMaxMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, MASSMANAGER_DELTA_NAME, 1, &m_dDelta);   

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int MassManager<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, MASSMANAGER_MIN_NAME,   &m_dMinMass);
   iResult += this->readPopKeyVal(pLine, MASSMANAGER_MAX_NAME,   &m_dMaxMass);
   iResult += this->readPopKeyVal(pLine, MASSMANAGER_DELTA_NAME, &m_dDelta);
      
   return iResult;
 }
