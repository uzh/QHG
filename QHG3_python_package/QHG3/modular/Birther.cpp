#include <string.h>
#include <omp.h>
#include <math.h>

#include "clsutils.cpp"
#include "WELL512.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "MassInterface.h"
#include "Birther.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Birther<T>::Birther(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL)
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dAdultMass(0),
      m_dBirthMass(0),
      m_dUncertainty(0),
      m_pMI(NULL) {
}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int Birther<T>::preLoop() {
    int iResult = -1;
    m_pMI = dynamic_cast<MassInterface *>(this->m_pPop);
    if (m_pMI != NULL) {
        iResult = 0;
    } else {
        printf("[Birther<T>::initialize] Population must implement MassInterface\n");
    }
    return iResult;   
}


//-----------------------------------------------------------------------------
// operator()
//
template<typename T>
int Birther<T>::operator()(int iAgentIndex, float fT) {
    int iResult = 0;

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {

        double dR =  this->m_apWELL[omp_get_thread_num()]->wrandr(1 - m_dUncertainty, 1 + m_dUncertainty);
        double dMBaby = dR*m_dBirthMass;
        double dMAgent = m_pMI->getMass(iAgentIndex);

        if (dMAgent > dMBaby) {
            m_pMI->setMass(iAgentIndex, dMAgent - dMBaby);
            m_pMI->setSecondaryMass(iAgentIndex, dMBaby);
            int iCellIndex = pa->m_iCellIndex;
            this->m_pPop->registerBirth(iCellIndex, iAgentIndex, -1);
            
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int Birther<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_extractAttribute(hSpeciesGroup, BIRTHER_ADULTMASS_NAME,   1, &m_dAdultMass);
    iResult += qdf_extractAttribute(hSpeciesGroup, BIRTHER_BIRTHMASS_NAME,   1, &m_dBirthMass);
    iResult += qdf_extractAttribute(hSpeciesGroup, BIRTHER_UNCERTAINTY_NAME, 1, &m_dUncertainty);   

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int Birther<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, BIRTHER_ADULTMASS_NAME,   1, &m_dAdultMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, BIRTHER_BIRTHMASS_NAME,   1, &m_dBirthMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, BIRTHER_UNCERTAINTY_NAME, 1, &m_dUncertainty);   

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int Birther<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, BIRTHER_ADULTMASS_NAME,   &m_dAdultMass);
   iResult += this->readPopKeyVal(pLine, BIRTHER_BIRTHMASS_NAME,   &m_dBirthMass);
   iResult += this->readPopKeyVal(pLine, BIRTHER_UNCERTAINTY_NAME, &m_dUncertainty);
      
   return iResult;
 }
