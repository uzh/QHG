#include <omp.h>

#include "MessLogger.h"

#include "clsutils.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "Verhulst.h"
#include "LinearBirth.cpp"
#include "LinearDeath.cpp"


// this number must changed if the parameters change
template<typename T>
int Verhulst<T>::NUM_VERHULST_PARAMS = 4;

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Verhulst<T>::Verhulst(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, int iMateOffset) 
    : Action<T>(pPop,pCG),
      m_iMateOffset(iMateOffset),
      m_apWELL(apWELL),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dB0 = -1024;
    m_dD0 = -1024;
    m_dTheta = -1024;
    m_dK = -1024;

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Verhulst<T>::~Verhulst() {

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
int Verhulst<T>::initialize(float fT) {
    
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
int Verhulst<T>::operator()(int iAgentIndex, float fT) {

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
int Verhulst<T>::finalize(float fT) {

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
//  tries to read the attributes
//    ATTR_VERHULST_B0_NAME
//    ATTR_VERHULST_D0_NAME
//    ATTR_VERHULST_TURNOVER_NAME
//    ATTR_VERHULST_CAPACITY_NAME
//  and creates a LinearBirth and LinearDeath object
//
template<typename T>
int Verhulst<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_B0_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_D0_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_TURNOVER_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_TURNOVER_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_CAPACITY_NAME, 1, &m_dK);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_CAPACITY_NAME);
        }
    }


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
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dB0, m_dTheta, m_dK, m_iMateOffset);
        m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dD0, m_dTheta, m_dK);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//  tries to write the attributes
//    ATTR_VERHULST_B0_NAME
//    ATTR_VERHULST_D0_NAME
//    ATTR_VERHULST_TURNOVER_NAME
//    ATTR_VERHULST_CAPACITY_NAME
//
template<typename T>
int Verhulst<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_CAPACITY_NAME, 1, &m_dK);

    return iResult;
}



//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int Verhulst<T>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;

    iResult += this->readPopKeyVal(pLine, ATTR_VERHULST_B0_NAME, &m_dB0);
    iResult += this->readPopKeyVal(pLine, ATTR_VERHULST_D0_NAME, &m_dD0);
    iResult += this->readPopKeyVal(pLine, ATTR_VERHULST_TURNOVER_NAME, &m_dTheta);
    iResult += this->readPopKeyVal(pLine, ATTR_VERHULST_CAPACITY_NAME, &m_dK);

    m_iNumSetParams += iResult;
  
    // since this fuction is called once for every line
    // we need to make sure that it was called successfully for all parameters
    // and only then we can create linear birth and death

    if (m_iNumSetParams == NUM_VERHULST_PARAMS) {

        // just in case we're reading in new parameters,
        // delete old birth and death actions

        if (m_pLB != NULL) {
            delete m_pLB;
        }
        if (m_pLD != NULL) {
            delete m_pLD;
        }
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dB0, m_dTheta, m_dK, m_iMateOffset);
        m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, m_apWELL, m_dD0, m_dTheta, m_dK);
    }
    

    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void Verhulst<T>::showAttributes() {
    printf("  %s\n", ATTR_VERHULST_B0_NAME);
    printf("  %s\n", ATTR_VERHULST_D0_NAME);
    printf("  %s\n", ATTR_VERHULST_TURNOVER_NAME);
    printf("  %s\n", ATTR_VERHULST_CAPACITY_NAME);
}



