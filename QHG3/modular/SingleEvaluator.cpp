#include <omp.h>

#include "MessLogger.h"
#include "EventConsts.h"

#include <string.h>
#include "clsutils.cpp"
#include "ArrayShare.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "PolyLine.h"
#include "SingleEvaluator.h"


//-----------------------------------------------------------------------------
// constructor
//   ATTENTION: using bCumulate=true for SingleEvaluators in a MultiEvaluator
//              can cause strange effects (icosahdral artefacts,  swimming)
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, intset &sTriggerIDs) 
    : Action<T>(pPop,pCG),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(adInputData),
      m_bCumulate(bCumulate), 
      m_sTriggerIDs(sTriggerIDs),
      m_bAlwaysUpdate(false),
      m_pInputArrayName(NULL),
      m_bFirst(true) {
    
    m_sPLParName = new char[256];
    strcpy(m_sPLParName, sPLParName);
    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read

}




//-----------------------------------------------------------------------------
// constructor
//   ATTENTION: using bCumulate=true for SingleEvaluators in a MultiEvaluator
//              can cause strange effects (icosahdral artefacts,  swimming)
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, int iTriggerID)
    : Action<T>(pPop,pCG),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(adInputData),
      m_bCumulate(bCumulate),
      m_bAlwaysUpdate(false),
      m_pInputArrayName(NULL),
      m_bFirst(true) {
    
    if (iTriggerID == EVENT_ID_NONE) {
        m_bAlwaysUpdate = true;
    } else {
        m_sTriggerIDs.insert(iTriggerID);
    }

    m_sPLParName = new char[256];
    strcpy(m_sPLParName, sPLParName);
    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read
}

//-----------------------------------------------------------------------------
// constructor
//   ATTENTION: using bCumulate=true for SingleEvaluators in a MultiEvaluator
//              can cause strange effects (icosahdral artefacts,  swimming)
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate, intset &sTriggerIDs) 
    : Action<T>(pPop,pCG),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(NULL),
      m_bCumulate(bCumulate), 
      m_sTriggerIDs(sTriggerIDs),
      m_pInputArrayName(NULL) {
    
    m_sPLParName = new char[256];
    strcpy(m_sPLParName, sPLParName);
    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read
    
    if (pInputArrayName != NULL) {
        m_pInputArrayName = new char[strlen(pInputArrayName)+1];
        strcpy(m_pInputArrayName, pInputArrayName);
    }
}



//-----------------------------------------------------------------------------
// destructorsimple
//
template<typename T>
SingleEvaluator<T>::~SingleEvaluator() {

    if (m_sPLParName  != NULL) {
        delete[] m_sPLParName;
    }
    if (m_pPL != NULL) {
        delete m_pPL;
    }
    if (m_pInputArrayName != NULL) {
        delete[] m_pInputArrayName;
    }
}



//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void SingleEvaluator<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if (!m_bAlwaysUpdate) {
        if (iEvent == EVENT_ID_FLUSH) {   
            // no need to act - recalculation will happen at next step's initialize()
        } else {
            intset::const_iterator its;

            for (its = m_sTriggerIDs.begin(); !m_bNeedUpdate && (its != m_sTriggerIDs.end()); ++its) {
                if (iEvent == *its) {
                    m_bNeedUpdate = true;
                }
            }
        }
    }
}


//-----------------------------------------------------------------------------
// finalize
//   reset m_bNeeedUpdate to false
//
template<typename T>
int SingleEvaluator<T>::finalize(float fT) {
    if ((!m_bAlwaysUpdate) && (m_bNeedUpdate)) {
        m_bNeedUpdate = false;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// initialize
// here the weights are calculated if needed
//
template<typename T>
int SingleEvaluator<T>::initialize(float fT) {

    int iResult = 0;

    // get array if it does not exist
    if (m_adInputData == NULL) {
        m_adInputData = (double *) ArrayShare::getInstance()->getArray(m_pInputArrayName);
    }


    if (m_adInputData != NULL) {
    

        if (m_bNeedUpdate || (m_bFirst)) {   // need for sure at first step
            m_bFirst = false;
            printf("SingleEvaluator::initialize is updating weights for %s\n", m_sPLParName); 
            
            calcValues(); // get cell values from PolyLine
            
            exchangeAndCumulate(); // compute actual weights
            
        }
    } else {
        printf("No array with name [%s] found in ArrayExchange\n", m_pInputArrayName);
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// calcValues
//
template<typename T>
void SingleEvaluator<T>::calcValues() {
 
    if (m_pPL != NULL) {
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (uint iCellIndex = 0; iCellIndex < this->m_pCG->m_iNumCells; iCellIndex++) {
            
            // please do not hard-code here things that can be set as parameters :-)
            if ((this->m_pCG->m_pGeography == NULL) || ( ! this->m_pCG->m_pGeography->m_abIce[iCellIndex] )) {
                
                double dV = this->m_pPL->getVal((float)m_adInputData[iCellIndex]);
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = (dV > 0) ? dV : 0;
                
            } else {
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = 0;
            }
        }
    } else {
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (uint iCellIndex = 0; iCellIndex < this->m_pCG->m_iNumCells; iCellIndex++) {
            
            // please do not hard-code here things that can be set as parameters :-)
            if ((this->m_pCG->m_pGeography == NULL) || ( ! this->m_pCG->m_pGeography->m_abIce[iCellIndex] )) {
                double dV = m_adInputData[iCellIndex];
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = (dV > 0) ? dV : 0;
            } else {
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = 0;
            }
        }
    }
}


//----------------------------------------------------------------------------
// exchangeAndNormalize
// here the actual normalized weights are computed 
// for all cells and their neighbors
//
template<typename T>
void SingleEvaluator<T>::exchangeAndCumulate() {

    // fill neighbor buffer for all cells
    

#ifdef OMP_A
#pragma omp parallel for
#endif
    for (uint iCellIndex = 0; iCellIndex < this->m_pCG->m_iNumCells; iCellIndex++) {
        
        SCell &sc = this->m_pCG->m_aCells[iCellIndex];
        double dW = m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)];  
        
        for (int i = 0; i < m_iMaxNeighbors; i++) {

            double dCW = 0;
            int iCurIndex = sc.m_aNeighbors[i];

            if (iCurIndex >= 0) {
                dCW = m_adOutputWeights[iCurIndex*(m_iMaxNeighbors+1)];  // Current Weight
            }

            dCW = (dCW > 0) ? dCW : 0;  // put negative weights to 0
            dW = (m_bCumulate) ? dW + dCW : dCW;  // cumulate if necessary
            
            m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1) + i + 1] = dW;
        }
    }
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attribute specified by m_sPLParName
//  and creates a PolyLine from it
//
template<typename T>
int SingleEvaluator<T>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = -1;

    printf("SingleEvaluator::extractParamsQDF will work on %s\n", m_sPLParName);
    if (this->m_pPL != NULL) {
        delete (this->m_pPL);
    }
    m_pPL = qdf_createPolyLine(hSpeciesGroup, m_sPLParName);
    if (m_pPL != NULL) {
        if (m_pPL->m_iNumSegments == 0) {
            delete m_pPL;
            m_pPL = NULL;
        }
        iResult = 0;
    } else {
        LOG_ERROR("[SingleEvaluator] couldn't read attribute [%s]", m_sPLParName);
        
        iResult = -1;
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attribute m_sPLParName
//
template<typename T>
int SingleEvaluator<T>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    iResult = qdf_writePolyLine(hSpeciesGroup, m_pPL, m_sPLParName);

     return iResult;

}



//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int SingleEvaluator<T>::tryReadParamLine(char *pLine) {

    int iResult = 0;

    if (strstr(pLine,m_sPLParName) == pLine) {

        char *p1 = strstr(pLine, "=");
        if (p1 != NULL) {
            p1++;
        
            m_pPL = PolyLine::readFromString(trim(p1));

            if (m_pPL != NULL) {
                iResult = 1;
            }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void SingleEvaluator<T>::showAttributes() {
    printf("  %s\n", m_sPLParName);
}

