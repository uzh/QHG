#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <omp.h>

#include <map>
#include <vector>

#include "types.h"
#include "WELL512.h"
#include "RankTable.h"

//---------------------------------------------------------------------------
//  createInstance
//
RankTable *RankTable::createInstance(uint iNumF, uint iNumM, WELL512 **apWELL, float **ppRanks) {
    RankTable *pRT = new RankTable(iNumF, iNumM, apWELL);
    int iResult = pRT->init(ppRanks);
    if (iResult != 0) {
        delete pRT;
        pRT = NULL;
    }
    return pRT;
}


//---------------------------------------------------------------------------
//  destructor
//
RankTable::~RankTable() {
    if (m_aaRanks != NULL) {
        for (uint i = 0; i < m_iNumF; ++i) {
            delete[] m_aaRanks[i];
        }
        delete[] m_aaRanks;
    }
    if (m_aFPermutation != NULL) {
        delete[] m_aFPermutation;
    }
    if (m_aFIndexes != NULL) {
        delete[] m_aFIndexes;
    }
    if (m_aMIndexes != NULL) {
        delete[] m_aMIndexes;
    }
}


//---------------------------------------------------------------------------
//  setRank
//
int RankTable::setRank(uint iF, uint iM, float fRank) {
    int iResult = -1;
    if ((iF < m_iNumF) && (iM < m_iNumM)) {
        m_aaRanks[iF][iM] = fRank;
        iResult = 0;
    } else {
        printf("bad index values : (%u,%u)\n", iF, iM);
    }
    return iResult;
}


//---------------------------------------------------------------------------
//  constructor
//
RankTable::RankTable(uint iNumF, uint iNumM, WELL512 **apWELL) 
    :  m_fPairTime(0),
       m_fRankTime(0),
       m_iNumF(iNumF),
       m_iNumM(iNumM),
       m_apWELL(apWELL),
       m_aaRanks(NULL),
       m_fCurMax(0),
       m_aFPermutation(NULL),
       m_aFIndexes(NULL),
       m_aMIndexes(NULL),
       m_iNumIndexes(0),
       m_bVerbose(false) {
    /*
    s_fPairTime += 0.0;
    s_fRankTime += 0.0;
    s_fRank1Time += 0.0;
    s_fRank2Time += 0.0;
    */
}
 

//---------------------------------------------------------------------------
//  init
//
int RankTable::init(float **ppRanks) {
    int iResult = 0;
    m_aaRanks = new float*[m_iNumF];
    if (ppRanks != NULL) {
        for (uint i = 0; i < m_iNumF; ++i) {
            m_aaRanks[i] = new float[m_iNumM];
            memcpy(m_aaRanks[i], ppRanks[i], m_iNumM*sizeof(float));
        }
    } else {
        for (uint i = 0; i < m_iNumF; ++i) {
            m_aaRanks[i] = new float[m_iNumM];
            memset(m_aaRanks[i], 0, m_iNumM*sizeof(float));
        }
    }

    m_aFPermutation = new uint[m_iNumF];
    m_aFIndexes     = new uint[m_iNumF*m_iNumM];
    m_aMIndexes     = new uint[m_iNumF*m_iNumM];

    return iResult;
}


//---------------------------------------------------------------------------
//  makeAllPairings
//
int RankTable::makeAllPairings(float fCutOff, bool bPermute) {
    int iResult = 0;
    m_vPaired.clear();

    bool bGoOn = true;

    while ((iResult == 0) && bGoOn) {
        // display();

        couples vc;
        float f1 = omp_get_wtime();
        iResult = pairHighestRankers(fCutOff, bPermute);
        m_fRankTime += (omp_get_wtime() - f1);
        
        if (iResult == 0) {
       
            //printf("---\n");
        } else {
            bGoOn = false;
            iResult = 0;
        }
        
    }



    return iResult;
}




//---------------------------------------------------------------------------
//  pairHighestRankers
//
int  RankTable::pairHighestRankers(float fCutOff, bool bPermute) {
    int iResult = -1;
    bool bHavePairs = false;

    // find current max rank (above cutoff)
    m_fCurMax = 0;
    float f1 = omp_get_wtime();
    float fMax=0;
    //#pragma omp parallel for reduction(max:fMax)
    for (uint i = 0; i < m_iNumF; i++) {
        for (uint j = 0; j < m_iNumM; j++) {
            if (m_aaRanks[i][j] < fCutOff) {
                m_aaRanks[i][j] = 0;
            } else if (m_aaRanks[i][j] >= fMax) {
                fMax = m_aaRanks[i][j];
            }
        }
    }
    m_fRank1Time += (omp_get_wtime() -f1);
    m_fCurMax = fMax;
  
    float f2 = omp_get_wtime();

    if (m_fCurMax > 0) {
        
        uint iNumPairable = 0;
        
        // collect females for which there are males with maximum rank
        for (uint iF = 0; iF < m_iNumF; iF++) {
            bool bGoOn = true;
            for (uint iM = 0; bGoOn && (iM < m_iNumM); iM++) {
                if (m_aaRanks[iF][iM] >= m_fCurMax) {
                    m_aFPermutation[iNumPairable] = iF;
                    iNumPairable++;
                    bGoOn = false;
                }
            }
        }
        if (m_bVerbose) printf("%d pairable females (total %d)\n", iNumPairable, m_iNumF);

        int iT = omp_get_thread_num();
        // we should permute the order of the females;
        // otherwise the females sitting early in the list will always paired; and later females will not
        if (bPermute) {
            for (uint i = iNumPairable-1; i > 1; i--) {
                uint k = m_apWELL[iT]->wrandi(0, i);
                uint s = m_aFPermutation[k];
                m_aFPermutation[k] = m_aFPermutation[i];
                m_aFPermutation[i] = s;
            }
        }
        
        // oerform the actual pairings
        for (uint i = 0; i < iNumPairable; i++) {
            uint iF = m_aFPermutation[i];
            memset(m_aMIndexes, 0, m_iNumM*sizeof(uint));
            m_iNumIndexes = 0;

            for (uint iM = 0; iM < m_iNumM; iM++) {
                if (m_aaRanks[iF][iM] >= m_fCurMax) {
                    m_aFIndexes[m_iNumIndexes] = iF;
                    m_aMIndexes[m_iNumIndexes] = iM;
                    m_iNumIndexes++;
                    if (m_bVerbose) printf("%f: %d, %d\n", m_fCurMax, iF, iM);
                }
            }

            randomPair(iF);
            if (m_iNumIndexes > 0) {
                bHavePairs = true;
            }
        }
    }
    m_fRank2Time += (omp_get_wtime() -f2);

    if (m_bVerbose) printf("m_iNumIndexes: %u\n", m_iNumIndexes);
    
    if (bHavePairs) {
        iResult = 0;
    }
    return iResult; 
}


//---------------------------------------------------------------------------
//  randomPair
//
int  RankTable::randomPair(uint iF) {
    int iResult = 0;

    if (m_iNumIndexes > 0) {
        int iT = omp_get_thread_num();
        uint iSel = m_apWELL[iT]->wrandi(0, m_iNumIndexes);
        pair(iF, m_aMIndexes[iSel]);
        iResult = 1;
    }
    return iResult;
}


//---------------------------------------------------------------------------
//  pair
//
int  RankTable::pair(uint iF, uint iM) {
    int iResult = 0;
    if (m_bVerbose) printf("Paired (%u,%u)\n", iF, iM);
    m_vPaired.push_back(couple(iF, iM));
    clearCross(iF, iM);
    return iResult;
}


//---------------------------------------------------------------------------
//  clearCross
//
void RankTable::clearCross(uint iF, uint iM) {

    memset(m_aaRanks[iF], 0, m_iNumM*sizeof(float));
    for (uint i = 0; i < m_iNumF; ++i) {
        m_aaRanks[i][iM] = 0.0;
    }
}


//---------------------------------------------------------------------------
//  display
//
void  RankTable::display() {

    for (uint i = 0; i < m_iNumF; ++i) {
        for (uint j = 0; j < m_iNumM; ++j) {
            printf("  %6.3f", m_aaRanks[i][j]);
        }
        printf("\n");
    }
}
