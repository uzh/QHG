#include <string.h>
#include <omp.h>
#include <math.h>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "PopFinder.h"
#include "QDFUtils.h"
#include "LotkaVolterra.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LotkaVolterra<T>::LotkaVolterra(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, PopFinder *pPopFinder)
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_pPopFinder(pPopFinder),
      m_pOtherPop(NULL),
      m_dSelfRate(0),
      m_dMixRate(0),
      m_adB(NULL),
      m_adD(NULL) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    m_adD = new double[this->m_pCG->m_iNumCells];
    *m_sOtherPopname = '\0';
    
}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LotkaVolterra<T>::~LotkaVolterra() {
    
   if (m_adB != NULL) {
       delete[] m_adB;
   }
   if (m_adD != NULL) {
       delete[] m_adD;
   }

}

//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int LotkaVolterra<T>::preLoop() {
    int iResult = -1;
    m_pOtherPop = m_pPopFinder->getPopByName(m_sOtherPopname);
    if (m_pOtherPop != NULL) {
        iResult = 0;
    }
    printf("[LotkaVolterra<T>::preLoop()] got pointer for [%s]: %p\n", m_sOtherPopname, m_pOtherPop);

    return iResult;
}
    


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int LotkaVolterra<T>::initialize(float fT) {
    printf("[LotkaVolterra<T>::initialize]\n");
    memset(m_adB,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    
    
#ifdef OMP_A
    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
#pragma omp parallel for schedule(static,iChunk)
#endif     
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        double d1 = m_dSelfRate;
        double d2 = m_dMixRate * ((double)(m_pOtherPop->getNumAgents(iC))) / m_dK;
       
        if (d1 > 0) {
           
            m_adB[iC] += d1;
            
        } else {
            m_adD[iC] -= d1;
        }

        if (d2 > 0) {
          
            m_adB[iC] += d2;
            
        } else {
            m_adD[iC] -= d2;
        }
        if (iC == 4) {
            printf("B: %f, D: %f (K:%f, NumAG %lu)\n", m_adB[iC], m_adD[iC], m_dK, m_pOtherPop->getNumAgents(iC));
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int LotkaVolterra<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
        int iCellIndex = pa->m_iCellIndex;
       

        int iThread = omp_get_thread_num();
        double dRB = this->m_apWELL[iThread]->wrandd();
          
        if (dRB < m_adB[iCellIndex]) {
            this->m_pPop->registerBirth(iCellIndex, iAgentIndex, -1);
            iResult = 1;
        } 

        double dRD = this->m_apWELL[iThread]->wrandd();
            
        if (dRD < m_adD[iCellIndex]) { // convert negative birth prob to death prob
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
    	}
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_LOTKAVOLTERRA_SELFRATE_NAME
//    ATTR_LOTKAVOLTERRA_MIXRATE_NAME
//    ATTR_LOTKAVOLTERRA_OTHERPOP_NAME
//    ATTR_LOTKAVOLTERRA_K_NAME
//
template<typename T>
int LotkaVolterra<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_SELFRATE_NAME, 1, &m_dSelfRate);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_SELFRATE_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_MIXRATE_NAME, 1, &m_dMixRate);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_MIXRATE_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_LOTKAVOLTERRA_OTHERPOP_NAME, NAME_LEN, m_sOtherPopname);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_OTHERPOP_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_K_NAME, 1, &m_dK);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_K_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_LOTKAVOLTERRA_SELFRATE_NAME
//    ATTR_LOTKAVOLTERRA_MIXRATE_NAME
//    ATTR_LOTKAVOLTERRA_OTHERPOP_NAME
//    ATTR_LOTKAVOLTERRA_K_NAME
//
template<typename T>
int LotkaVolterra<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_SELFRATE_NAME, 1, &m_dSelfRate);
    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_MIXRATE_NAME, 1, &m_dMixRate);
    iResult += qdf_insertSAttribute(hSpeciesGroup, ATTR_LOTKAVOLTERRA_OTHERPOP_NAME, m_sOtherPopname);
    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_K_NAME, 1, &m_dK);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int LotkaVolterra<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, ATTR_LOTKAVOLTERRA_SELFRATE_NAME, &m_dSelfRate);
   iResult += this->readPopKeyVal(pLine, ATTR_LOTKAVOLTERRA_MIXRATE_NAME, &m_dMixRate);
   char *pVal = readKeyString(pLine, ATTR_LOTKAVOLTERRA_OTHERPOP_NAME, "=");
   if (pVal != NULL) {
       strncpy(m_sOtherPopname, pVal, NAME_LEN);
       iResult += 1;
   } else {
       iResult += 0;
   }
   iResult += this->readPopKeyVal(pLine, ATTR_LOTKAVOLTERRA_K_NAME, &m_dK);
      
   return iResult;
 }


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void LotkaVolterra<T>::showAttributes() {
    printf("  %s\n", ATTR_LOTKAVOLTERRA_SELFRATE_NAME);
    printf("  %s\n", ATTR_LOTKAVOLTERRA_MIXRATE_NAME);
    printf("  %s\n", ATTR_LOTKAVOLTERRA_OTHERPOP_NAME);
    printf("  %s\n", ATTR_LOTKAVOLTERRA_K_NAME);
    
}
