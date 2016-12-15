#include <stdio.h>
#include <string.h>

#include "GeneUtils.h"
#include "DistMat.h"



//----------------------------------------------------------------------------
// createDistMat
// 
DistMat *DistMat::createDistMat(int iGenomeSize, 
                                       const char **apGenome1, int iN1) {
    // create long arrays from strings
    ulong **ulGenome1 = new ulong*[iN1];
    for (int i = 0; i < iN1; i++) {
        ulGenome1[i] = GeneUtils::translateGenome(iGenomeSize, apGenome1[i]);
    }

    DistMat *pDM = new DistMat();
    int iResult = pDM->init(iGenomeSize, ulGenome1, iN1, true);
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}

//----------------------------------------------------------------------------
// createDistMat
// 
DistMat *DistMat::createDistMat(int iGenomeSize, 
                                ulong **ulGenome1, int iN1) {
    DistMat *pDM = new DistMat();
    int iResult = pDM->init(iGenomeSize, ulGenome1, iN1, false);
    if (iResult != 0) {
        delete pDM;
        pDM = NULL;
    }
    return pDM;
}

//----------------------------------------------------------------------------
// destructor
// 
DistMat::~DistMat() {
    cleanMat();
    if (m_bDeleteArrays) {
        for (int i = 0; i < m_iN1; i++) {
            delete[] m_ulGenome1[i];
        }
        delete[] m_ulGenome1;
    }
    delete[] m_ulG1;
    delete[] m_ulG2;

}


//----------------------------------------------------------------------------
// constructor
// 
DistMat::DistMat()
    : m_iGenomeSize(0),
      m_ulGenome1(NULL),
      m_iN1(0),
      m_bDeleteArrays(false),
      m_aaCounts(NULL) {
}

//----------------------------------------------------------------------------
// init
// 
int DistMat::init(int iGenomeSize, 
                  ulong **ulGenome1, int iN1, bool bDeleteArrays) {

    int iResult = 0;
    m_iGenomeSize = iGenomeSize;
    m_ulGenome1   = ulGenome1;
    m_iN1         = iN1;
    m_bDeleteArrays = bDeleteArrays;

    int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
    m_ulG1 = new ulong[2*iNumBlocks];
    m_ulG2 = new ulong[2*iNumBlocks];

    return iResult;
}

//----------------------------------------------------------------------------
// cleanMat
// 
void DistMat::cleanMat() {
    if (m_aaCounts != NULL) {
        for (int i = 0; i < m_iN1; i++) {
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
int **DistMat::createMatrix() {
    bool bVerbose = false;
    if (m_aaCounts != NULL) {
        cleanMat();
    }
    
    m_aaCounts = new int*[m_iN1];
    for (int i = 0; i < m_iN1; i++) {
        m_aaCounts[i] = new int[m_iN1];
        memset(m_aaCounts[i], 0, m_iN1*sizeof(int));
    }

    int iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
    for (int i = 0; i < m_iN1; i++) {
        for (int j = i; j < m_iN1; j++) {
            int iC = iNumBlocks*GeneUtils::BITSINBLOCK;
            if (bVerbose) GeneUtils::showGenome(m_ulGenome1[i], m_iGenomeSize, SHOW_NUC);
            iC = GeneUtils::calcDist(m_ulGenome1[i], m_ulGenome1[j], m_iGenomeSize);
            m_aaCounts[i][j] = iC;
            m_aaCounts[j][i] = iC;
        }
    }

    return m_aaCounts;
}
