#include <omp.h>

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
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, bool* pbUpdateNeeded) 
    : Action<T>(pPop,pCG),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(adInputData),
      m_bCumulate(bCumulate), 
      m_pbUpdateNeeded(pbUpdateNeeded),
      m_pInputArrayName(NULL) {
    
    m_sPLParName = new char[256];
    strcpy(m_sPLParName, sPLParName);
    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read

}

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate, bool* pbUpdateNeeded) 
    : Action<T>(pPop,pCG),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(NULL),
      m_bCumulate(bCumulate), 
      m_pbUpdateNeeded(pbUpdateNeeded),
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
    

        if ((m_pbUpdateNeeded != NULL && *m_pbUpdateNeeded) || fT == 0) {   // need for sure at first step
            
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
template<typename T>
int SingleEvaluator<T>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = -1;

    // TO DO:
    // THE FOLLOWING CAN BE ABSORBED INTO THE POLYLINE CLASS
    // E.G.:  m_pPL = PolyLine::createPolyLineFromQDF(hSpeciesGroup, m_sPLParName->c_str());

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
        iResult = -1;
    }
    /**************
    hid_t hAttr = H5Aopen(hSpeciesGroup, m_sPLParName, H5P_DEFAULT);

    hid_t hSpace = H5Aget_space(hAttr);
    hsize_t dims = 3;
    hvl_t       aPLData[3];
    int ndims = H5Sget_simple_extent_dims(hSpace, &dims, NULL);
    if (ndims == 1) {
        hid_t hMemType = H5Tvlen_create (H5T_NATIVE_DOUBLE);

        herr_t status = H5Aread(hAttr, hMemType, aPLData);
            
        if (status >= 0) {
            if (this->m_pPL != NULL) {
                delete (this->m_pPL);
            }
            this->m_pPL = new PolyLine((uint)aPLData[0].len-1);
            memcpy(this->m_pPL->m_afX, aPLData[0].p, (this->m_pPL->m_iNumSegments+1)*sizeof(double));
            memcpy(this->m_pPL->m_afV, aPLData[1].p, (this->m_pPL->m_iNumSegments+1)*sizeof(double));
            memcpy(this->m_pPL->m_afA, aPLData[2].p, this->m_pPL->m_iNumSegments*sizeof(double));
            iResult = 0;

        } else {
            printf("Couldn't read %s attrtibute\n",m_sPLParName);
        }
        status = H5Dvlen_reclaim (hMemType, hSpace, H5P_DEFAULT, aPLData);
        iResult = (status >= 0)?0:-1;
        qdf_closeAttribute(hAttr);
        qdf_closeDataType(hMemType);
        qdf_closeDataSpace(hSpace);
    } else {
        printf("Bad number of dimensions: %d\n", ndims);
    }
    *********/
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int SingleEvaluator<T>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    iResult = qdf_writePolyLine(hSpeciesGroup, m_pPL, m_sPLParName);

    /***********
    //    printf("SingleEvaluator::writeParamsQDF is inserting %s\n",m_sPLParName->c_str());

    // TO DO:
    // THE FOLLOWING CAN BE ABSORBED INTO THE POLYLINE CLASS
    // E.G.:  m_pPL = PolyLine::createPolyLineFromQDF(hSpeciesGroup, m_sPLParName->c_str());

    hvl_t       aPLData[3];
    int iNumPoints = this->m_pPL->m_iNumSegments+1;
    aPLData[0].len = iNumPoints;
    aPLData[0].p   = (void *) malloc(iNumPoints*sizeof(double));
    memcpy(aPLData[0].p, this->m_pPL->m_afX, iNumPoints*sizeof(double));
    aPLData[1].len = iNumPoints;
    aPLData[1].p   = (void *) malloc(iNumPoints*sizeof(double));
    memcpy(aPLData[1].p, this->m_pPL->m_afV, iNumPoints*sizeof(double));
    aPLData[2].len = this->m_pPL->m_iNumSegments;
    aPLData[2].p   = (void *) malloc(this->m_pPL->m_iNumSegments*sizeof(double));
    memcpy(aPLData[2].p, this->m_pPL->m_afA, this->m_pPL->m_iNumSegments*sizeof(double));

    hid_t hMemType = H5Tvlen_create (H5T_NATIVE_DOUBLE);
    hsize_t dims = 3;
    hid_t hSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttr = H5Acreate (hSpeciesGroup, m_sPLParName, hMemType, hSpace, H5P_DEFAULT,
                             H5P_DEFAULT);
    herr_t status = H5Awrite (hAttr, hMemType, aPLData);


    status = H5Dvlen_reclaim (hMemType, hSpace, H5P_DEFAULT, aPLData);
    qdf_closeAttribute (hAttr);
    iResult = (status >= 0)?0:-1;

    //    printf("SingleEvaluator inserted %s with result %d\n",m_sPLParName,iResult);
    *****/
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

