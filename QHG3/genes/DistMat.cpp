#include <stdio.h>
#include <string.h>
#include <math.h>

#include <omp.h>

#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "DistMat.h"



/*
//----------------------------------------------------------------------------
// createDistMat
// 
template<typename T>
DistMat<T> *DistMat<T>::createDistMat(int iSequenceSize, 
                                const char **apSequence1, int iN1) {
    // create long arrays from strings
    ulong **ulGenome1 = new ulong*[iN1];
    for (int i = 0; i < iN1; i++) {
        ulGenome1[i] = GeneUtils::translateGenome(iSequenceSize, apSequence1[i]);
    }

    DistMat *pDM = new DistMat();
    int iResult = pDM->init(iSequenceSize, ulGenome1, iN1, ulGenome1, iN1, true, false); // deletGenomes, normal nucs
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}
*/

//----------------------------------------------------------------------------
// createDistMat
// 
template<typename T>
DistMat<T> *DistMat<T>::createDistMat(int iSequenceSize, 
                                      T **ulSequence1, int iN1, calcdist_t fCalcDist) {
    DistMat<T> *pDM = new DistMat<T>(fCalcDist, false);
    int iResult = pDM->init(iSequenceSize, ulSequence1, iN1, ulSequence1, iN1);
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}

/*
//----------------------------------------------------------------------------
// createDistMatRef
// 
template<typename T>
DistMat<T> *DistMat<T>::createDistMatRef(int iGenomeSize, 
                                   const char **apGenome1, int iN1,
                                   const char **apGenome2, int iN2) {
    // create long arrays from strings
    ulong **ulGenome1 = new ulong*[iN1];
    for (int i = 0; i < iN1; i++) {
        ulGenome1[i] = GeneUtils::translateGenome(iGenomeSize, apGenome1[i]);
    }

    ulong **ulGenome2 = new ulong*[iN2];
    for (int i = 0; i < iN2; i++) {
        ulGenome2[i] = GeneUtils::translateGenome(iGenomeSize, apGenome2[i]);
    }

    DistMat<T> *pDM = new DistMat<T>();
    int iResult = pDM->init(iGenomeSize, ulGenome1, iN1, ulGenome2, iN2, true); // deletGenomes, normal nucs
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}
*/

//----------------------------------------------------------------------------
// createDistMatRef
// 
template<typename T>
DistMat<T> *DistMat<T>::createDistMatRef(int iSequenceSize, 
                                   T **ulSequence1, int iN1, 
                                   T **ulSequence2, int iN2,
                                   calcdist_t fCalcDist) {
    DistMat<T> *pDM = new DistMat<T>(fCalcDist, false);
    int iResult = pDM->init(iSequenceSize, ulSequence1, iN1, ulSequence2, iN2);
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}


//----------------------------------------------------------------------------
// destructor
// 
template<typename T>
DistMat<T>::~DistMat() {
    cleanMat();
    if (m_bDeleteArrays) {
        for (int i = 0; i < m_iN1; i++) {
            delete[] m_ulSequence1[i];
        }
        for (int i = 0; i < m_iN2; i++) {
            delete[] m_ulSequence2;
        }

    }

}


//----------------------------------------------------------------------------
// constructor
// 
template<typename T>
DistMat<T>::DistMat(calcdist_t fCalcDist, bool bDeleteArrays)
    : m_iSequenceSize(0),
      m_ulSequence1(NULL),
      m_iN1(0),
      m_ulSequence2(NULL),
      m_iN2(0),
      m_bDeleteArrays(bDeleteArrays),
      m_aaDists(NULL),
      m_bSymmetric(false),
      m_fCalcDist(fCalcDist) {
}



//----------------------------------------------------------------------------
// init
// 
template<typename T>
int DistMat<T>::init(int iSequenceSize, 
                     T **ulSequence1, int iN1, 
                     T **ulSequence2, int iN2) {

    int iResult = 0;
    m_iSequenceSize = iSequenceSize;
    m_ulSequence1   = ulSequence1;
    m_iN1         = iN1;
    m_ulSequence2   = ulSequence2;
    m_iN2         = iN2;

    if ((m_ulSequence1 == m_ulSequence2) && (iN1 == iN2)) {
        m_bSymmetric = true;
    }

    return iResult;

}

//----------------------------------------------------------------------------
// cleanMat
// 
template<typename T>
void DistMat<T>::cleanMat() {
    if (m_aaDists != NULL) {
        for (int i = 0; i < m_iN1; i++) {
            if (m_aaDists[i] != NULL) {
                delete[] m_aaDists[i];
            }
        }
        delete[] m_aaDists;
    }
}



//----------------------------------------------------------------------------
// createMatrix
// 
template<typename T>
float **DistMat<T>::createMatrix() {
    if (m_aaDists != NULL) {
        cleanMat();
    }
    
    m_aaDists = new float*[m_iN1];
    // parallelize this loop
#pragma omp parallel for
    for (int i = 0; i < m_iN1; i++) {
        m_aaDists[i] = new float[m_iN2];
        memset(m_aaDists[i], 0, m_iN2*sizeof(float));
    }

    if (m_bSymmetric) {
        // parallelize this loop
#pragma omp parallel for
        for (int i = 0; i < m_iN1; i++) {
            for (int j = i; j < m_iN2; j++) {
                float iC = m_fCalcDist(m_ulSequence1[i], m_ulSequence2[j], m_iSequenceSize);
                m_aaDists[i][j] = iC;
                m_aaDists[j][i] = iC;
            }
        }
    } else {
#pragma omp parallel for
        for (int i = 0; i < m_iN1; i++) {
            for (int j = 0; j < m_iN2; j++) {
                float iC = m_fCalcDist(m_ulSequence2[i], m_ulSequence2[j], m_iSequenceSize);
                m_aaDists[i][j] = iC;
            }
        }
    }

    return m_aaDists;
}
