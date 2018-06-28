#include <omp.h>

#include "MessLogger.h"

#include "clsutils.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "Fertility.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Fertility<T>::Fertility(SPopulation<T> *pPop, SCellGrid *pCG) 
    : Action<T>(pPop,pCG),
      m_fFertilityMinAge(0),
      m_fFertilityMaxAge(0),
      m_fInterbirth(0) {

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Fertility<T>::~Fertility() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//  must be called after GetOld
//
template<typename T>
int Fertility<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        if (pa->m_iGender == 0) {
            if ((pa->m_fAge > m_fFertilityMinAge) && 
                (pa->m_fAge < m_fFertilityMaxAge) && 
                ((fT - pa->m_fLastBirth) > m_fInterbirth)) {
                pa->m_iLifeState = LIFE_STATE_FERTILE;
            } else {
                pa->m_iLifeState = LIFE_STATE_ALIVE;
            }
        } else {            
            if (pa->m_fAge > m_fFertilityMinAge) {
                pa->m_iLifeState = LIFE_STATE_FERTILE;
            } else {
                pa->m_iLifeState = LIFE_STATE_ALIVE;
            }
        }
    }

    return iResult;
}




//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_FERTILITY_MIN_AGE_NAME
//    ATTR_FERTILITY_MAX_AGE_NAME
//    ATTR_FERTILITY_INTERBIRTH_NAME
//
template<typename T>
int Fertility<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FERTILITY_MIN_AGE_NAME, 1, &m_fFertilityMinAge);
        if (iResult != 0) {
            LOG_ERROR("[Fertility] couldn't read attribute [%s]", ATTR_FERTILITY_MIN_AGE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FERTILITY_MAX_AGE_NAME, 1, &m_fFertilityMaxAge);
        if (iResult != 0) {
            LOG_ERROR("[Fertility] couldn't read attribute [%s]", ATTR_FERTILITY_MAX_AGE_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FERTILITY_INTERBIRTH_NAME, 1, &m_fInterbirth);
        if (iResult != 0) {
            LOG_ERROR("[Fertility] couldn't read attribute [%s]", ATTR_FERTILITY_INTERBIRTH_NAME);
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_FERTILITY_MIN_AGE_NAME
//    ATTR_FERTILITY_MAX_AGE_NAME
//    ATTR_FERTILITY_INTERBIRTH_NAME
//
template<typename T>
int Fertility<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FERTILITY_MIN_AGE_NAME, 1, &m_fFertilityMinAge);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FERTILITY_MAX_AGE_NAME, 1, &m_fFertilityMaxAge);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FERTILITY_INTERBIRTH_NAME, 1, &m_fInterbirth);

    return iResult;
}



//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int Fertility<T>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;

    iResult += this->readPopKeyVal(pLine, ATTR_FERTILITY_MIN_AGE_NAME, &m_fFertilityMinAge);
    iResult += this->readPopKeyVal(pLine, ATTR_FERTILITY_MAX_AGE_NAME, &m_fFertilityMaxAge);
    iResult += this->readPopKeyVal(pLine, ATTR_FERTILITY_INTERBIRTH_NAME, &m_fInterbirth);

    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void Fertility<T>::showAttributes() {
    printf("  %s\n", ATTR_FERTILITY_MIN_AGE_NAME);
    printf("  %s\n", ATTR_FERTILITY_MAX_AGE_NAME);
    printf("  %s\n", ATTR_FERTILITY_INTERBIRTH_NAME);
}

