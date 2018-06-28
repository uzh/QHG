#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "GeneUtils.h"
#include "BinomialDist.h"
#include "AncGraphBase.h"
#include "GraphEvolverBase.h"

#define EPS 1e-6

//----------------------------------------------------------------------------
// constructor
//
GraphEvolverBase::GraphEvolverBase(int iNumCrossovers, double dMutationRate) 
    : m_pAG(NULL),
      m_iNumBlocks(0),
      m_iNumBits(0),
      m_iNumCrossOvers(iNumCrossovers),
      m_dMutationRate(dMutationRate),
      m_pTempGenome1(NULL),
      m_pTempGenome2(NULL),
      m_pBDist(NULL),
      m_pAux(NULL) {

    m_iNumThreads = omp_get_num_threads();
    m_pTempGenome1 = new ulong*[m_iNumThreads]; 
    m_pTempGenome2 = new ulong*[m_iNumThreads]; 
    memset(m_pTempGenome1, 0, m_iNumThreads*sizeof(ulong *));
    memset(m_pTempGenome2, 0, m_iNumThreads*sizeof(ulong *));
}

//----------------------------------------------------------------------------
// destructor
//
GraphEvolverBase::~GraphEvolverBase() {
    idgenome::const_iterator it;
    for (it = m_mGenomes.begin(); it != m_mGenomes.end(); ++it) {
        if (it->second != NULL) {
            delete[] it->second;
        }
    }
    if (m_pTempGenome1 != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            delete[] m_pTempGenome1[i];
        }
        delete[] m_pTempGenome1;
    }
    if (m_pTempGenome2 != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            delete[] m_pTempGenome2[i];
        }
        delete[] m_pTempGenome2;
    }
    if (m_pBDist != NULL) {
        delete m_pBDist;
    }

    if (m_pAux != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            delete[] m_pAux[i];
        }
        delete[] m_pAux;
    }

}

//----------------------------------------------------------------------------
// init
//
int GraphEvolverBase::init(int iGenomeSize) {
    int iResult = -1;
 
    if ((m_dMutationRate >= 0) && (m_dMutationRate < 1)) {
        
        if (iGenomeSize > 0) {
            iResult = setGenomeSize(iGenomeSize);
        } else {
            iResult = 0;
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getGenome
//
const ulong *GraphEvolverBase::getGenome(idtype iID) {
    const ulong *pGenome = NULL;
    idgenome::const_iterator it = m_mGenomes.find(iID);
    if (it != m_mGenomes.end()) {
        pGenome = it->second;
    } else {
        pGenome = NULL;
    }
    return pGenome;
}


//----------------------------------------------------------------------------
// setGenomeSize
//
int GraphEvolverBase::setGenomeSize(int iGenomeSize) {
    int iResult = -1;
    if (iGenomeSize > 0) {
        m_iGenomeSize = iGenomeSize;
        m_iNumBlocks  = GeneUtils::numNucs2Blocks(m_iGenomeSize);
        m_iNumBits    = 2*m_iGenomeSize;
        
        if (m_iNumBlocks > 0) {
            for (int i = 0; i < m_iNumThreads; i++) {
                if (m_pTempGenome1[i] != NULL) {
                    delete[] m_pTempGenome1[i];
                }
                m_pTempGenome1[i] = new ulong[2*m_iNumBlocks];
            }
            for (int i = 0; i < m_iNumThreads; i++) {
                if (m_pTempGenome2[i] != NULL) {
                    delete[] m_pTempGenome2[i];
                }
                m_pTempGenome2[i] = new ulong[2*m_iNumBlocks];
            }
        }
        
        if (m_pBDist != NULL) {
            delete m_pBDist;
        }
        m_pBDist = BinomialDist::create(m_dMutationRate, 2*iGenomeSize, EPS);
        if (m_pBDist != NULL) {
            iResult = 0;
        }

        for (int i = 0; i < m_iNumThreads; i++) {
            if (m_pAux[i] != NULL) {
                delete[] m_pAux[i];
            }
            m_pAux[i] = new ulong[2*m_iNumBlocks];
        }

    }
    return iResult;
}


//----------------------------------------------------------------------------
// createMix 
//   performs crossing over on Genome1 and Genome2 and selects
//   one chromosome of each for the baby
//
ulong *GraphEvolverBase::createMix(const ulong *pGenome1, const ulong *pGenome2) {
    int iT = omp_get_thread_num();
    memcpy(m_pAux[iT], pGenome1, 2*m_iNumBlocks*sizeof(ulong));
    return createMix(m_pAux[iT], pGenome2);
}

//----------------------------------------------------------------------------
// createMix 
//   performs crossing over on Genome1 and Genome2 and selects
//   one chromosome of each for the baby
//
ulong *GraphEvolverBase::createMix(ulong *pGenome1, ulong *pGenome2) {
    int iT = omp_get_thread_num();
    ulong *pBabyGenome = new ulong[2*m_iNumBlocks];
    memset(pBabyGenome, 0, 2*m_iNumBlocks*sizeof(ulong));
    
    // copy 1 a strand  of genome1 and genome2 each to BabyGenome
    int i1 = (int)((2.0*rand())/(RAND_MAX+1.0));
    int i2 = (int)((2.0*rand())/(RAND_MAX+1.0));


    if (m_iNumCrossOvers > 0) {
        GeneUtils::crossOver(m_pTempGenome1[iT], pGenome1, m_iGenomeSize, m_iNumCrossOvers, NULL);
        GeneUtils::crossOver(m_pTempGenome2[iT], pGenome2, m_iGenomeSize, m_iNumCrossOvers, NULL);

        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == -1) {
        // full recombination
        GeneUtils::freeReco(m_pTempGenome1[iT], static_cast<ulong*>(pGenome1), m_iNumBlocks, NULL);
        GeneUtils::freeReco(m_pTempGenome2[iT], static_cast<ulong*>(pGenome2), m_iNumBlocks, NULL);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == 0) {
        memcpy(pBabyGenome,              pGenome1+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, pGenome2+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
    }

    // perhaps some mutations...
    if (m_dMutationRate > 0) {
        double p = (1.0*rand())/(RAND_MAX+1.0);
        int iNumMutations = m_pBDist->getN(p);
        // anywhere on the two chromosomes
        GeneUtils::mutateNucs(pBabyGenome, 2*m_iGenomeSize, iNumMutations, NULL);
    }
    return pBabyGenome;
}
