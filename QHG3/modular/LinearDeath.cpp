#include <omp.h>
#include <math.h>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "LinearDeath.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearDeath<T>::LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL)
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dD0(0),
      m_dTheta(0),
      m_dK(0),
      m_adK(NULL),
      m_iStride(1) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];

}

//-----------------------------------------------------------------------------
// constructor for use with e.g. Verhulst action
//
template<typename T>
LinearDeath<T>::LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double dD0, double dTheta, double dK) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_dK(dK),
      m_adK(NULL),
      m_iStride(1) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    
}

//-----------------------------------------------------------------------------
// constructor for use with e.g. VerhulstVarK action
//
template<typename T>
LinearDeath<T>::LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double dD0, double dTheta, double* adK, int iStride) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_dK(-1024),
      m_adK(adK),
      m_iStride(iStride) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearDeath<T>::~LinearDeath() {
    
    if (m_adD != NULL) {
        delete[] m_adD;
    }
    
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int LinearDeath<T>::initialize(float fT) {
    
    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));

    if (m_adK == NULL) {

        // loop for constant K
#ifdef OMP_A
	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
#pragma omp parallel for schedule(static,iChunk)
#endif     
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            // constant K
            m_adD[iC] = m_dD0 + (m_dTheta - m_dD0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_dK);
        }
    } else {
        // loop for varying K
#ifdef OMP_A
	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
#pragma omp parallel for schedule(static,iChunk)
#endif     
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            if (m_adK[iC * m_iStride] <= 0) {
                m_adD[iC] = 1;
            } else {
                // space-varying K
                m_adD[iC] = m_dD0 + (m_dTheta - m_dD0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_adK[iC * m_iStride]);
            }
        }
    }
    
    return 0;
}



//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int LinearDeath<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
    	int iCellIndex = pa->m_iCellIndex;
    	
    	int iThread = omp_get_thread_num();
    	
    	double dR = this->m_apWELL[iThread]->wrandd();

    	if (dR < m_adD[iCellIndex]) {
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            iResult = 1;
    	}
    }
	
    return iResult;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_LINDEATH_D0_NAME
//    ATTR_LINDEATH_TURNOVER_NAME
//    ATTR_LINDEATH_CAPACITY_NAME
//
template<typename T>
int LinearDeath<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATH_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[LinearDeath] couldn't read attribute [%s]", ATTR_LINDEATH_D0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATH_TURNOVER_NAME, 1, &m_dTheta);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeath] couldn't read attribute [%s]", ATTR_LINDEATH_TURNOVER_NAME);
           }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATH_CAPACITY_NAME, 1, &m_dK);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeath] couldn't read attribute [%s]", ATTR_LINDEATH_CAPACITY_NAME);
           }
    }
        
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_LINDEATH_D0_NAME
//    ATTR_LINDEATH_TURNOVER_NAME
//    ATTR_LINDEATH_CAPACITY_NAME
//
template<typename T>
int LinearDeath<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATH_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATH_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATH_CAPACITY_NAME, 1, &m_dK);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int LinearDeath<T>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;

    iResult += this->readPopKeyVal(pLine, ATTR_LINDEATH_D0_NAME, &m_dD0);
    iResult += this->readPopKeyVal(pLine, ATTR_LINDEATH_TURNOVER_NAME, &m_dTheta);
    iResult += this->readPopKeyVal(pLine, ATTR_LINDEATH_CAPACITY_NAME, &m_dK);
    
    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void LinearDeath<T>::showAttributes() {
    printf("  %s\n", ATTR_LINDEATH_D0_NAME);
    printf("  %s\n", ATTR_LINDEATH_TURNOVER_NAME);
    printf("  %s\n", ATTR_LINDEATH_CAPACITY_NAME);
}
