#include <omp.h>
#include <math.h>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "LinearBirth.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearBirth<T>::LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, int iMateOffset) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dB0(0),
      m_dTheta(0),
      m_adK(NULL),
      m_iStride(1),
      m_iMateOffset(iMateOffset) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];

}

//-----------------------------------------------------------------------------
// constructor for use with Verhulst action
//
template<typename T>
LinearBirth<T>::LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double dB0, double dTheta, double dK, int iMateOffset) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dB0(dB0),
      m_dTheta(dTheta),
      m_dK(dK), 
      m_adK(NULL),
      m_iStride(1),
      m_iMateOffset(iMateOffset) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    
}

//-----------------------------------------------------------------------------
// constructor for use with VerhulstVarK action
//
template<typename T>
LinearBirth<T>::LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double dB0, double dTheta, double* adK, int iStride, int iMateOffset) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dB0(dB0),
      m_dTheta(dTheta),
      m_dK(-1024),
      m_adK(adK), 
      m_iStride(iStride),
      m_iMateOffset(iMateOffset) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearBirth<T>::~LinearBirth() {
    
    if (m_adB != NULL) {
        delete[] m_adB;
    }
    
}


//-----------------------------------------------------------------------------
 // initialize
//
template<typename T>
int LinearBirth<T>::initialize(float fT) {
    
    memset(m_adB,0,sizeof(double) * (this->m_pCG->m_iNumCells));
   

#ifdef OMP_A
	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
#pragma omp parallel for schedule(static,iChunk)
#endif  
  for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {

        if (m_adK == NULL) {
            // case with constant K
            m_adB[iC] = m_dB0 + (m_dTheta - m_dB0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_dK);
        } else {
            if ( m_adK[iC * m_iStride] <= 0) {
                m_adB[iC] = 0;
            } else {
                // case with space-dependent K
                m_adB[iC] = m_dB0 + (m_dTheta - m_dB0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_adK[iC * m_iStride]);            
            }
        }
        
    }
    
    return 0;
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int LinearBirth<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
	
    	int iCellIndex = pa->m_iCellIndex;
        
    	// if reproduction is in couples, use mate index, 
    	// otherwise use agent index also as father
	
    	// note: the offset is needed because if the agent structure 
    	// does not have a m_iMateIndex member, the compiler 
    	// would not know what to do
	
    	int iMate = (m_iMateOffset >= 0) ? 
            *(int*)((char*)pa + m_iMateOffset) : 
            -1;

    	if (m_adB[iCellIndex] > 0) { // positive birth prob
            if (m_iMateOffset < 0 ||
            	(pa->m_iGender == 0 && iMate >= 0)) {
            	
                int iThread = omp_get_thread_num();
                
                double dR = this->m_apWELL[iThread]->wrandd();
                
                if (dR < m_adB[iCellIndex]) {
                    this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iMate);
                    iResult = 1;
            	} 
            }
    	} else if (m_adB[iCellIndex] < 0) {
            
            int iThread = omp_get_thread_num();
            
            double dR = this->m_apWELL[iThread]->wrandd();

            if (dR < -m_adB[iCellIndex]) { // convert negative birth prob to death prob
                this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            }
    	}

    }

    return iResult;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_LINBIRTH_B0_NAME
//    ATTR_LINBIRTH_TURNOVER_NAME
//    ATTR_LINBIRTH_CAPACITY_NAME
//
template<typename T>
int LinearBirth<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTH_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirth] couldn't read attribute [%s]", ATTR_LINBIRTH_B0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTH_TURNOVER_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirth] couldn't read attribute [%s]", ATTR_LINBIRTH_TURNOVER_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTH_CAPACITY_NAME, 1, &m_dK);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirth] couldn't read attribute [%s]", ATTR_LINBIRTH_CAPACITY_NAME);
        }
    }
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_LINBIRTH_B0_NAME
//    ATTR_LINBIRTH_TURNOVER_NAME
//    ATTR_LINBIRTH_CAPACITY_NAME
//
template<typename T>
int LinearBirth<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTH_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTH_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTH_CAPACITY_NAME, 1, &m_dK);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int LinearBirth<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, ATTR_LINBIRTH_B0_NAME, &m_dB0);
   iResult += this->readPopKeyVal(pLine, ATTR_LINBIRTH_TURNOVER_NAME, &m_dTheta);
   iResult += this->readPopKeyVal(pLine, ATTR_LINBIRTH_CAPACITY_NAME, &m_dK);

   return iResult;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void LinearBirth<T>::showAttributes() {
    printf("  %s\n", ATTR_LINBIRTH_B0_NAME);
    printf("  %s\n", ATTR_LINBIRTH_TURNOVER_NAME);
    printf("  %s\n", ATTR_LINBIRTH_CAPACITY_NAME);
}
