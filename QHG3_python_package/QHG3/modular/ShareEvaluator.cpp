#include <omp.h>

#include "/usr/include/valgrind/memcheck.h"

#include <string.h>
#include "clsutils.cpp"
#include "ArrayShare.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "PolyLine.h"
#include "ShareEvaluator.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
ShareEvaluator<T>::ShareEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, const char *sID, bool bCumulate, bool* pbUpdateNeeded) 
    : Action<T>(pPop,pCG),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(NULL),
      m_bCumulate(bCumulate), 
      m_pbUpdateNeeded(pbUpdateNeeded),
      m_pID(NULL) {


    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read
    
    m_pID = new char[strlen(sID)+1];
    strcpy(m_pID, sID);

    *m_sArrayName = '\0';
    *m_sPolyName  = '\0';
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
ShareEvaluator<T>::~ShareEvaluator() {

    if (m_pPL != NULL) {
        delete m_pPL;
    }

    if (m_pID != NULL) {
        delete m_pID;
    }
}

//-----------------------------------------------------------------------------
// preLoop
//   load the shared input data
//
template<typename T>
int ShareEvaluator<T>::preLoop() {

    int iResult = 0;

    // get array if it does not exist
    m_adInputData = (double *) ArrayShare::getInstance()->getArray(m_sArrayName);
    if (m_adInputData != NULL) {
        printf("[ShareEvaluator<T>::initialize]Loaded shared array [%s]: %p\n", m_sArrayName, m_adInputData);
    } else {
        printf("No array with name [%s] found in ArrayExchange\n", m_sArrayName);
        iResult = -1;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// initialize
// here the weights are calculated if needed
//
template<typename T>
int ShareEvaluator<T>::initialize(float fT) {
    int iResult = 0;

    if ((m_pbUpdateNeeded != NULL && *m_pbUpdateNeeded) || fT == 0) {   // need for sure at first step
        
        printf("ShareEvaluator::initialize is updating weights for %s\n", m_sPolyName); 
        
        calcValues(); // get cell values from PolyLine
        
        exchangeAndCumulate(); // compute actual weights
        
    }
         
    return iResult;
}


//-----------------------------------------------------------------------------
// calcValues
//
template<typename T>
void ShareEvaluator<T>::calcValues() {

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
                if (VALGRIND_CHECK_VALUE_IS_DEFINED( m_adInputData[iCellIndex])) {
                    printf("[ShareEvaluator<T>::calcValues]m_adInputData[%d] is undefined: %f\n", iCellIndex, m_adInputData[iCellIndex]);
                }

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
void ShareEvaluator<T>::exchangeAndCumulate() {

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
template<typename T>
int ShareEvaluator<T>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (this->m_pPL != NULL) {
        delete (this->m_pPL);
    }

    char sKeyArrName[SHARE_NAME_LEN];
    sprintf(sKeyArrName, SHAREEVAL_ARRAYNAME, m_pID);
    char sKeyPolyName[SHARE_NAME_LEN];
    sprintf(sKeyPolyName, SHAREEVAL_POLYNAME, m_pID);

    iResult += qdf_extractSAttribute(hSpeciesGroup, sKeyArrName, SHARE_NAME_LEN, m_sArrayName);
    iResult += qdf_extractSAttribute(hSpeciesGroup, sKeyPolyName,  SHARE_NAME_LEN, m_sPolyName);

    if (*m_sPolyName != '\0') {
        printf("ShareEvaluator::extractParamsQDF will work on %s\n", m_sPolyName);
        m_pPL = qdf_createPolyLine(hSpeciesGroup, m_sPolyName);
        if (m_pPL != NULL) {
            if (m_pPL->m_iNumSegments == 0) {
                delete m_pPL;
                m_pPL = NULL;
            }
            iResult = 0;
        } else {
            iResult = -1;
        }
    } else {
        printf("ShareEvaluator::extractParamsQDF: empty poly name\n");
        iResult = -1;
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int ShareEvaluator<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    char sKeyArrName[SHARE_NAME_LEN];
    sprintf(sKeyArrName, SHAREEVAL_ARRAYNAME, m_pID);
    char sKeyPolyName[SHARE_NAME_LEN];
    sprintf(sKeyPolyName, SHAREEVAL_POLYNAME, m_pID);

    printf("writing att [%s]:%s\n", sKeyArrName, m_sArrayName);
    iResult += qdf_insertSAttribute(hSpeciesGroup, sKeyArrName, m_sArrayName);
    printf("writing att [%s]:%s\n", sKeyPolyName, m_sPolyName);
    iResult += qdf_insertSAttribute(hSpeciesGroup, sKeyPolyName, m_sPolyName);
    printf("writing poly [%s]:%s\n", sKeyPolyName, m_sPolyName);
    iResult += qdf_writePolyLine(hSpeciesGroup, m_pPL, m_sPolyName);

    return iResult;

}



//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int ShareEvaluator<T>::tryReadParamLine(char *pLine) {
 
    int iResult = 0;

    char sKeyArrName[SHARE_NAME_LEN];
    sprintf(sKeyArrName, SHAREEVAL_ARRAYNAME, m_pID);
    char sKeyPolyName[SHARE_NAME_LEN];
    sprintf(sKeyPolyName, SHAREEVAL_POLYNAME, m_pID);

 
    char *pVal = readKeyString(pLine,sKeyArrName, "=");
    if (pVal != NULL) {
        strncpy(m_sArrayName, pVal, SHARE_NAME_LEN);
        iResult += 1;
    } else {
        iResult += 0;
    }

    
    pVal = readKeyString(pLine,sKeyPolyName, "=");
    if (pVal != NULL) {
        strncpy(m_sPolyName, pVal, SHARE_NAME_LEN);
        iResult += 1;
    } else {
        iResult += 0;
    }
    
  
    if ((*m_sPolyName != '\0') && (strstr(pLine,m_sPolyName) == pLine)) {

        char *p1 = strstr(pLine, "=");
        if (p1 != NULL) {
            p1++;
        
            m_pPL = PolyLine::readFromString(trim(p1));

            if (m_pPL != NULL) {
                iResult = 0;
            }
        }
    }

    return iResult;
}

