#include <omp.h>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "OldAgeDeath.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
OldAgeDeath<T>::OldAgeDeath(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dMaxAge(0),
      m_dUncertainty(0) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
OldAgeDeath<T>::~OldAgeDeath() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int OldAgeDeath<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        pa->m_fAge = fT - pa->m_fBirthTime;
        // death possible starting at 0.8 times m_dMaxAge
        double dR =  this->m_apWELL[omp_get_thread_num()]->wrandr(1 - m_dUncertainty, 1 + m_dUncertainty);
//        printf("OldAgeDeath a%d: age %f, rand %f, max %f\n", iAgentIndex,  pa->m_fAge, dR, m_dMaxAge);
        if (pa->m_fAge*dR > m_dMaxAge) {
            this->m_pPop->registerDeath(pa->m_iCellIndex, iAgentIndex);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_OLDAGEDEATH_MAXAGE_NAME
//    ATTR_OLDAGEDEATH_UNCERTAINTY_NAME
//
template<typename T>
int OldAgeDeath<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_MAXAGE_NAME, 1, &m_dMaxAge);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_OLDAGEDEATH_MAXAGE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_UNCERTAINTY_NAME, 1, &m_dUncertainty);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_OLDAGEDEATH_UNCERTAINTY_NAME);
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_OLDAGEDEATH_MAXAGE_NAME
//    ATTR_OLDAGEDEATH_UNCERTAINTY_NAME
//
template<typename T>
int OldAgeDeath<T>:: writeParamsQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_MAXAGE_NAME, 1, &m_dMaxAge);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_UNCERTAINTY_NAME, 1, &m_dUncertainty);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int OldAgeDeath<T>::tryReadParamLine(char *pLine) {
   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, ATTR_OLDAGEDEATH_MAXAGE_NAME, &m_dMaxAge);
   iResult += this->readPopKeyVal(pLine, ATTR_OLDAGEDEATH_UNCERTAINTY_NAME, &m_dUncertainty);

   return iResult;

}
    
//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void OldAgeDeath<T>::showAttributes() {
    printf("  %s\n", ATTR_OLDAGEDEATH_MAXAGE_NAME);
    printf("  %s\n", ATTR_OLDAGEDEATH_UNCERTAINTY_NAME);
}

