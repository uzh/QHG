#include <vector>
#include "nr.h"
#include "types.h"
#include "BinomialDist.h"

//----------------------------------------------------------------------------
// create
//
BinomialDist *BinomialDist::create(double dProb, int n, double dEpsilon) {
    BinomialDist *pCB = new BinomialDist();
    int iResult = pCB->init(dProb, n ,dEpsilon);
    if (iResult != 0) {
        delete pCB;
        pCB = NULL;
    }
    return pCB;
    
}

//----------------------------------------------------------------------------
// constructor
//
BinomialDist::BinomialDist()
    : m_N(0),
      m_dProb(0),
      m_iNumBins(0),
      m_adLookUp(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
BinomialDist::~BinomialDist() {
    if (m_adLookUp != NULL) {
        delete[] m_adLookUp;
    }
}

//----------------------------------------------------------------------------
// init
//
int BinomialDist::init(double dProb, int n, double dEpsilon) {
    int iResult = -1;

    // sanity check fo 
    if ((dProb >= 0) && (dProb < 1) && (n > 0)) {
        m_dProb = dProb;
        m_N = n;
        iResult = buildTable(dEpsilon);
    }

    return iResult;

}

//----------------------------------------------------------------------------
// buildTable
//
int BinomialDist::buildTable(double dEpsilon) {
    int iResult = -1;

    std::vector<double> vList;
    int k = 1;
    double d2 = betai(k, m_N-k+1, m_dProb);
    //    printf("value %d: %e\n", k, 1-d2);
    while ((d2 > dEpsilon) && (k < m_N)) {
        vList.push_back(d2);
        k++;
        d2 = betai(k, m_N-k+1, m_dProb);
        //        printf("value %d: %e\n", k, 1-d2);
    }
    vList.push_back(d2);

    if (vList.size() > 0) {
        m_adLookUp = new double[vList.size()];
        m_iNumBins = (uint) vList.size();
        m_adLookUp[0] = 1-vList[0];
        for (uint i = 1; i < m_iNumBins; i++) {
            m_adLookUp[i] = 1-vList[i];
        }
        iResult = 0;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// getN
//
int BinomialDist::getN(double dP) {
    int iResult = -1;
    
    if (dP >=  0) {
        uint i = 0;
        while ((i < m_iNumBins) && (dP > m_adLookUp[i])) {
            i++;
        }
        iResult = i;
    }

    return iResult;
}



//----------------------------------------------------------------------------
// showTable
//
void BinomialDist::showTable() {
    printf("%d entries:\n", m_iNumBins);
    for (uint i = 0; i < m_iNumBins; i++) {
        printf("  %e", m_adLookUp[i]);
    }
    printf("\n"); 
}
