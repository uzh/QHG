#include <stdio.h>
#include <string.h>

#include <omp.h>

#include "GeneUtils.h"
#include "CrossDistMat.h"



//----------------------------------------------------------------------------
// createCrossDistMat
// 
CrossDistMat *CrossDistMat::createCrossDistMat(int iGenomeSize, 
                                               const char **apGenome0, int iN0, 
                                               const char **apGenome1, int iN1) {
    // create long arrays from strings
    ulong **ulGenome0 = new ulong*[iN0];
    for (int i = 0; i < iN0; i++) {
        ulGenome0[i] = GeneUtils::translateGenome(iGenomeSize, apGenome0[i]);
    }
    ulong **ulGenome1 = new ulong*[iN1];
    for (int i = 0; i < iN1; i++) {
        ulGenome1[i] = GeneUtils::translateGenome(iGenomeSize, apGenome1[i]);
    }

    CrossDistMat *pDM = new CrossDistMat();
    int iResult = pDM->init(iGenomeSize, ulGenome0, iN0, ulGenome1, iN1, true);
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}

//----------------------------------------------------------------------------
// createCrossDistMat
// 
CrossDistMat *CrossDistMat::createCrossDistMat(int iGenomeSize, 
                                               ulong **ulGenome0, int iN0,
                                               ulong **ulGenome1, int iN1) {
    CrossDistMat *pDM = new CrossDistMat();
    int iResult = pDM->init(iGenomeSize, ulGenome0, iN0, ulGenome1, iN1, false);
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}

//----------------------------------------------------------------------------
// destructor
// 
CrossDistMat::~CrossDistMat() {
    cleanMat();
    if (m_bDeleteArrays) {
        for (int i = 0; i < m_iN0; i++) {
            delete[] m_ulGenome0[i];
        }
        delete[] m_ulGenome0;

        for (int i = 0; i < m_iN1; i++) {
            delete[] m_ulGenome1[i];
        }
        delete[] m_ulGenome1;
    }
}


//----------------------------------------------------------------------------
// constructor
// 
CrossDistMat::CrossDistMat()
    : m_iGenomeSize(0),
      m_ulGenome0(NULL),
      m_iN0(0),
      m_ulGenome1(NULL),
      m_iN1(0),
      m_bDeleteArrays(false),
      m_aaCounts(NULL),
      m_iMinDist(10000000),
      m_iMinIdx0(-1),
      m_iMinIdx1(-1),
      m_iMaxDist(-10000000),
      m_iMaxIdx0(-1),
      m_iMaxIdx1(-1) {
}

//----------------------------------------------------------------------------
// init
// 
int CrossDistMat::init(int iGenomeSize, 
                       ulong **ulGenome0, int iN0, 
                       ulong **ulGenome1, int iN1, bool bDeleteArrays) {

    int iResult = 0;
    m_iGenomeSize = iGenomeSize;
    m_ulGenome0   = ulGenome0;
    m_iN0         = iN0;
    m_ulGenome1   = ulGenome1;
    m_iN1         = iN1;
    m_bDeleteArrays = bDeleteArrays;

    return iResult;
}

//----------------------------------------------------------------------------
// cleanMat
// 
void CrossDistMat::cleanMat() {
    if (m_aaCounts != NULL) {
        for (int i = 0; i < m_iN0; i++) {
            if (m_aaCounts[i] != NULL) {
                delete[] m_aaCounts[i];
            }
        }
        delete[] m_aaCounts;
    }
}



//----------------------------------------------------------------------------
// createMatrix
// 
int **CrossDistMat::createMatrix() {
    if (m_aaCounts != NULL) {
        cleanMat();
    }
    
    m_aaCounts = new int*[m_iN0];
    // parallelize this loop
#pragma omp parallel for
    for (int i = 0; i < m_iN0; i++) {
        m_aaCounts[i] = new int[m_iN1];
        memset(m_aaCounts[i], 0, m_iN1*sizeof(int));
    }
    m_dAvg = 0;
    int iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
    // parallelize this loop
#pragma omp parallel for
    for (int i = 0; i < m_iN0; i++) {
        for (int j = 0; j < m_iN1; j++) {
            int iC = iNumBlocks*GeneUtils::BITSINBLOCK;
            iC = GeneUtils::calcDist(m_ulGenome0[i], m_ulGenome1[j], m_iGenomeSize);
            m_aaCounts[i][j] = iC;
            if (iC > m_iMaxDist) {
                m_iMaxDist = iC;
                m_iMaxIdx0 = i;
                m_iMaxIdx1 = j;
            }
            if (iC < m_iMinDist) {
                m_iMinDist = iC;
                m_iMinIdx0 = i;
                m_iMinIdx1 = j;
            }
            m_dAvg += iC;
        }
    }
    m_dAvg /= (m_iN0*m_iN1);

    return m_aaCounts;
}


//----------------------------------------------------------------------------
// showStats
// 
void CrossDistMat::showStats() {
    printf("cdm Num Distances: %d x %d = %d\n", m_iN0, m_iN1,  m_iN0*m_iN1);
    printf("cdm Avg Distance:  %f\n", m_dAvg);
    printf("cdm Min Distance:  %d at [%d,%d]\n", m_iMinDist, m_iMinIdx0, m_iMinIdx1);
    printf("cdm Max Distance:  %d at [%d,%d]\n", m_iMaxDist, m_iMaxIdx0, m_iMaxIdx1);
}
