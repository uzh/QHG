#include <omp.h>
#include <math.h>

#include "clsutils.cpp"
#include "geomutils.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "ConfinedMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
ConfinedMove<T>::ConfinedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::vector<int>** vMoveList) 
    : Action<T>(pPop,pCG),
      m_vMoveList(vMoveList),
      m_bAllowed(NULL),
      m_bAbsorbing(false),
      m_bInitialized(false) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
ConfinedMove<T>::~ConfinedMove() {

    if (m_bAllowed != NULL) {
        delete[] m_bAllowed;
    }

}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int ConfinedMove<T>::initialize(float fT) {
    
    if (!m_bInitialized) {

        m_bAllowed = new bool[this->m_pCG->m_iNumCells];

        memset(m_bAllowed, 0, this->m_pCG->m_iNumCells * sizeof(bool));

        double* dLon = this->m_pCG->m_pGeography->m_adLongitude;
        double* dLat = this->m_pCG->m_pGeography->m_adLatitude;

        if (this->m_pCG->m_iType == GRID_TYPE_FLAT4 ||
            this->m_pCG->m_iType == GRID_TYPE_FLAT6) {

            double dRC2 = m_dR * m_dR;

#ifdef OMP_A
#pragma omp parallel for
#endif
            for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
                double dR2 = (m_dX - dLon[i]) * (m_dX - dLon[i]) + (m_dY - dLat[i]) * (m_dY - dLat[i]);
                if (dR2 < dRC2) {
                    m_bAllowed[i] = true;
                } 
            }
        } else {  // icosahedral grid

            double dConv = 3.14159 / 180.0;

#ifdef OMP_A
#pragma omp parallel for
#endif
            for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
                double dR = spherdist(dLon[i]*dConv, dLat[i]*dConv, m_dX*dConv, m_dY*dConv, RADIUS_EARTH_KM);
                if (dR < m_dR) {
                    m_bAllowed[i] = true;
                }
            }
        }
    }
    
    return 0;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int ConfinedMove<T>::finalize(float fT) {

    // it's VERY important that finalize is called before lists
    // are updated in SPopulation::finalizeStep()
    
#ifdef OMP_A
#pragma omp parallel 
#endif
    {
        int iT = omp_get_thread_num();
        for (unsigned int iIndex = 0; iIndex < m_vMoveList[iT]->size(); iIndex += 3) {
            int iCellTo = (*m_vMoveList[iT])[iIndex+2];
            if ( ! m_bAllowed[iCellTo] ) {
                (*m_vMoveList[iT])[iIndex+2] = (*m_vMoveList[iT])[iIndex];
            }
        }
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int ConfinedMove<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
     int iResult = 0;

    iResult += qdf_extractAttribute(hSpeciesGroup, CONFINEDMOVE_X_NAME, 1, &m_dX);
    iResult += qdf_extractAttribute(hSpeciesGroup, CONFINEDMOVE_Y_NAME, 1, &m_dY);
    iResult += qdf_extractAttribute(hSpeciesGroup, CONFINEDMOVE_R_NAME, 1, &m_dR);

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int ConfinedMove<T>::writeParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    iResult += qdf_insertAttribute(hSpeciesGroup, CONFINEDMOVE_X_NAME, 1, &m_dX);
    iResult += qdf_insertAttribute(hSpeciesGroup, CONFINEDMOVE_Y_NAME, 1, &m_dY);
    iResult += qdf_insertAttribute(hSpeciesGroup, CONFINEDMOVE_R_NAME, 1, &m_dR);
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int ConfinedMove<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, CONFINEDMOVE_X_NAME, &m_dX);
   iResult += this->readPopKeyVal(pLine, CONFINEDMOVE_Y_NAME, &m_dY);
   iResult += this->readPopKeyVal(pLine, CONFINEDMOVE_R_NAME, &m_dR);
   
   return iResult;
}
