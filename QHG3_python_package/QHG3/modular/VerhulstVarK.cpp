#include <omp.h>

#include "clsutils.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "VerhulstVarK.h"
#include "LinearBirth.cpp"
#include "LinearDeath.cpp"



//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
VerhulstVarK<T>::VerhulstVarK(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adK, int iMateOffset) 
    : Action<T>(pPop,pCG),
      m_adK(adK),
      m_iMateOffset(iMateOffset),
      m_apWELL(apWELL),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dB0 = -1024;
    m_dD0 = -1024;
    m_dTheta = -1024;

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
VerhulstVarK<T>::~VerhulstVarK() {

    if (m_pLB != NULL) {
        delete m_pLB;
    }
    if (m_pLD != NULL) {
        delete m_pLD;
    }

}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int VerhulstVarK<T>::initialize(float fT) {
    
    int iResult = 0;

    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += m_pLB->initialize(fT);
        iResult += m_pLD->initialize(fT);
        

    } else {
        iResult = -1;
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int VerhulstVarK<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += (*m_pLB)(iAgentIndex,fT);
        iResult += (*m_pLD)(iAgentIndex,fT);

    } else {
        iResult = -1;
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int VerhulstVarK<T>::finalize(float fT) {

    int iResult = 0;
    
    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += m_pLB->finalize(fT);
        iResult += m_pLD->finalize(fT);

    } else {
        iResult = -1;
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int VerhulstVarK<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    iResult += qdf_extractAttribute(hSpeciesGroup, VERHULST_B0_NAME, 1, &m_dB0);
    iResult += qdf_extractAttribute(hSpeciesGroup, VERHULST_D0_NAME, 1, &m_dD0);
    iResult += qdf_extractAttribute(hSpeciesGroup, VERHULST_TURNOVER_NAME, 1, &m_dTheta);

    // now that we have the parameters, we can actuall create life and death objects
    
    if (iResult == 0) {

        // just in case we're reading in new parameters,
        // delete old life and death actions

        if (m_pLB != NULL) {
            delete m_pLB;
        }
        if (m_pLD != NULL) {
            delete m_pLD;
        }
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dB0, m_dTheta, m_adK, m_iMateOffset);
        m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dD0, m_dTheta, m_adK);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int VerhulstVarK<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, VERHULST_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, VERHULST_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, VERHULST_TURNOVER_NAME, 1, &m_dTheta);

    return iResult;
}



//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int VerhulstVarK<T>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;

    iResult += this->readPopKeyVal(pLine, VERHULST_B0_NAME, &m_dB0);

    iResult += this->readPopKeyVal(pLine, VERHULST_D0_NAME, &m_dD0);

    iResult += this->readPopKeyVal(pLine, VERHULST_TURNOVER_NAME, &m_dTheta);

    m_iNumSetParams += iResult;
  
    // since this fuction is called once for every line
    // we need to make sure that it was called successfully for all parameters
    // and only then we can create linear birth and death

    if(m_iNumSetParams == 3) {

        // just in case we're reading in new parameters,
        // delete old birth and death actions

        if (m_pLB != NULL) {
            delete m_pLB;
        }
        if (m_pLD != NULL) {
            delete m_pLD;
        }
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dB0, m_dTheta, m_adK, m_iMateOffset);
        m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dD0, m_dTheta, m_adK);
    }
    

    return iResult;
}



