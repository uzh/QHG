#include <stdio.h>

#include "types.h"
#include "smallprimes.h"

#include "RegionSplitter.h"
#include "Region.h"
#include "FullRegion.h"
#include "RectRegion.h"
#include "RectSplitter.h"

float fdiv(int i1, int i2) {
    return (float)((float)i1/(float)i2);
}

//-------------------------------------------------------------------------------
// constructor
//
RectSplitter::RectSplitter(GridProjection *pGP, double dH, int iNX, int iNY, bool bStrict)
    : m_pGP(pGP),
      m_dH(dH),
      m_iNX(iNX),
      m_iNY(iNY),
      m_iW(0),
      m_iH(0),
      m_bGrid(true),
      m_bStrict(bStrict) {

    m_iNumRegions = m_iNX * m_iNY;
}

//-------------------------------------------------------------------------------
// constructor
//
RectSplitter::RectSplitter(GridProjection *pGP, double dH, int iNumTiles, bool bGrid, bool bStrict)
    : m_pGP(pGP),
      m_dH(dH),
      m_iNX(0),
      m_iNY(0),
      m_iW(0),
      m_iH(0),
      m_bGrid(bGrid),
      m_bStrict(bStrict) {

    m_iNumRegions = iNumTiles;
}

//-------------------------------------------------------------------------------
// creeateRegions
//
void RectSplitter::prepareFactors(int iNum) {

    VECUS vPrimes;
    VECUS vPowers;

    primiFactor((ushort)m_iNumRegions, vPrimes, vPowers);
    recursiveSplit(vPrimes, vPowers, 0,1, m_vFactors);
    
printf("factors:\n");
    for (uint i = 0; i < m_vFactors.size(); ++i) {
        printf(" %d \n", m_vFactors[i]);
    }
    printf("\n");

}

//-------------------------------------------------------------------------------
// recursiveSplit
//   Calculates all possible products which can be formed with the primes in
//   vPrimes and their corresponding powers in vPowers and stores them in vFactors.
//   
//   Recursion: for every possible power of the first prime, calculate all
//   possible factors with the rest of the primes.
//
void RectSplitter::recursiveSplit(VECUS vPrimes, 
                                  VECUS vPowers, 
                                  ushort iStart, 
                                  ushort iF, 
                                  VECUS &vFactors) {
                    
    if (iStart < (int)vPowers.size()) {
        ushort iF0 = iF; 

        for (int i = 0; i <= vPowers[iStart]; ++i) {
            recursiveSplit(vPrimes, vPowers, (ushort)(iStart+1), iF0, vFactors);
            iF0 = (ushort)(iF0*vPrimes[iStart]);
        }
    } else {
        vFactors.push_back(iF);
    }
} 

//-------------------------------------------------------------------------------
// creeateRegions
//
Region **RectSplitter::createRegions(int *piNumTiles) {
    Region **pRegions = NULL;
    if (m_iNumRegions == 1) {
        m_apRegions = new Region*[1];
        m_apRegions[0] = new FullRegion(0);
        *piNumTiles = m_iNumRegions;
        printf("Trivial\n");
        pRegions = m_apRegions;
    } else {
        m_apRegions = new Region*[m_iNumRegions];

        int iResult = 0;
        *piNumTiles = 0;
        if (m_pBox != NULL) {
            m_iW = (int)m_pBox->dLonMax - (int)m_pBox->dLonMin;
            m_iH = (int)m_pBox->dLatMax - (int)m_pBox->dLatMin;
            if (m_bGrid) {
                prepareFactors(m_iNumRegions);
                if ((m_iNX == 0) && (m_iNY == 0)) {
                    iResult = findBestPartition((ushort)m_iNumRegions);
                } else {
                    if (m_bStrict) {
                        iResult = findRegularPartition();
                    } else {
                        iResult = findIrregularPartition();
                    }
                }
            } else {
                // free-style: get from Balancer
                // for now: error
                iResult = -2;
            }
            if (iResult == 0) {
                *piNumTiles = m_iNumRegions;
                pRegions = m_apRegions;
            }
        } else {
            printf("No box\n");
        }
    }
   
    return pRegions;
}

//-------------------------------------------------------------------------------
// findBestPartition
//   find partition of iN which yields  the most squarelike tile sizes
//
int RectSplitter::findBestPartition(ushort iNumRegions) {
    int iResult = -1;
    m_iNumRegions = iNumRegions;

    VECUS vPrimes;
    VECUS vPowers;
   
    if (m_iNumRegions > 0) {
        
        iResult = findRegularPartition();
        if (iResult < 0) {
            printf("[TileSplitter::findBestPartition] Couldn't find regular partition\n");
            printf("[TileSplitter::findBestPartition] Trying irregular partition\n");
            iResult = findIrregularPartition();
            if (iResult < 0) {
                printf("[TileSplitter::findBestPartition] Couldn't even find irregular partition\n");

            }
        }


    } else {
        printf("[TileSplitter::findBestPartition] Can't create a partition of 0 tiiles\n");
    }
    return iResult;
}

//-------------------------------------------------------------------------------
// findRegularPartition
//   find partition of iN which yields the most squarelike tile sizes
//
int RectSplitter::findRegularPartition() {
    int iResult = -1;
 
    VECUS vFactors2;
    int iBest = compatiblePairs(m_vFactors, vFactors2);
    /*@@
    for (unsigned int i = 0; i < vFactors2.size(); ++i) {
        int iF2 = iN/vFactors2[i];
        printf("%d x %d -> %dx%d\n", vFactors2[i], iF2, iW/vFactors2[i], iH/iF2);
    }
    */
    if (iBest >= 0) {

        int iTX = vFactors2[iBest];
        int iTY = m_iNumRegions/vFactors2[iBest];
        printf(" --> %d x %d\n", iTX, iTY);
        int iWX = m_iW/iTX;
        int iHY = m_iH/iTY;
        printf("W=%d, H=%d, WX=%d, HY=%d\n", m_iW, m_iH, iWX, iHY);
        int iSY = 0;
        int iC = 0;
        // now fill it
        for (int i = 0; i < iTY; i++) {
            int iSX = 0;
            for (int j = 0; j < iTX; j++) {
                printf("strting point (%d,%d) %dx%d\n", iSX, iSY, iWX, iHY);
                m_apRegions[iC] = new RectRegion(iC, m_pGP, m_dH, iSX, iSX+iWX, iSY, iSY+iHY);
                iC++;
                iSX += iWX;
            }
            iSY += iHY;
        }
        iResult = 0;
     //   printf("(%d) Have tiling %dx%d\n", getpid(), iTX, iTY);
    } else {
        printf("can't do a regular partition\n");
        printf("none of the prime factors of %d divides width (%d) or height (%d)\n",m_iNumRegions, m_iW, m_iH);
        iResult = -1;
    }
    return iResult;
}




//-------------------------------------------------------------------------------
// approximatePairs
//   find partition of iN which yields the most squarelike tile sizes
//
int RectSplitter::approximatePairs(VECUS vFactors, 
                                   float fAsp,
                                   int *piFactor1,
                                   int *piFactor2) {
    int iResult =0;
    float fBest = 1e10;
    
    for (uint i = 0; i < vFactors.size(); i++) {
        int iP1 = vFactors[i];
        int iP2 = m_iNumRegions/iP1;
        printf("Checking %d %d\n", iP1, iP2);
        float fCur = (float)((1.0*iP1)/iP2);
        //      if ((fAsp >= 1) == (fCur >= 1)) {
            float ff = fAsp/fCur;
            if (fabs(ff - 1) < fabs(fBest-1)) {
                fBest = ff;
                *piFactor1 = iP1;
                *piFactor2 = iP2;
            }
            //        }
    }
    return iResult;
}

//-------------------------------------------------------------------------------
// findIrregularPartition
//   find partition of iN which yields the most squarelike tile sizes
//
int RectSplitter::findIrregularPartition() {
    int iResult = -1;

    float fAsp = fdiv(m_iW,m_iH);
    int iFactor1=0;
    int iFactor2=0;
    int iBest = approximatePairs(m_vFactors, fAsp, &iFactor1, &iFactor2);
    if (iBest == 0) {
        
        iResult = doForcedPartition(iFactor1, iFactor2);

    }
    return iResult;
}




//-----------------------------------------------------------------------------
// doForcedPartition
//   split field into tile of as equal size as possible
//
int RectSplitter::doForcedPartition(int iTX, int iTY, int iCutOffX, int iCutOffY) {
    float dW = fdiv(m_iW,iTX);
    float dH = fdiv(m_iH,iTY);
    if (dW < iCutOffX) {
        iCutOffX = 1;
    }
    if (dH < iCutOffY) {
        iCutOffY = 1;
    }
         
    printf("dW %f, dH %f\n", dW, dH);
    int iC = 0;
    int iSY = 0;
    double dSY = 0;
    for (int i = 0; i < iTY; i++) {
        double dSYN = dSY + dH;
        int iSYN = (int)(dSYN);
        
        if (m_iH-dSYN < iCutOffY) {
            //            printf("Y-stop: H:%d SYN:%f, dH:%f\n", m_iH, dSYN, dH);
            iSYN = m_iH;
        }
        
        int iSX = 0;
        double dSX = 0;
        for (int j = 0; j < iTX; j++) {
            double dSXN = dSX + dW;
            int iSXN = (int)(dSXN);
            if (m_iW-dSXN < iCutOffX) {
                //                printf("X-stop: W:%d SXN:%f, dW:%f\n", m_iW, dSXN, dW);
                iSXN = m_iW;
            }
            printf("Double data %f,%f, %f,%f\n", dSX, dSXN, dSY, dSYN);
            printf("Add region %d: xmin:%d, xmax:%d, ymin:%d, ymax:%d\n", iC, iSX, iSXN, iSY, iSYN);
            m_apRegions[iC] = new RectRegion(iC, m_pGP, m_dH, iSX, iSXN, iSY, iSYN);
            iC++;

            iSX = iSXN;
            dSX = dSXN;
        }
        iSY = iSYN;
        dSY = dSYN;
    }
        
 
    return 0;
}


//-------------------------------------------------------------------------------
// compatiblePairs
//   selects all factors which divide iW, and whose cofactors with respect to 
//   iN divide iH
//   Returns index of best pair, i.e. the pair whose aspect ratio is closest to 1
//
int RectSplitter::compatiblePairs(VECUS vFactors, VECUS &vFactors2) {
    int iBest = -1;
    float fBest = 0;
    int iC = 0;
    for (uint i = 0; i < vFactors.size(); i++) {
        int iF1 = vFactors[i];
        if (m_iW%iF1 == 0) {
            ushort iF2 = (ushort) (m_iNumRegions/iF1);
            if ((m_iW%iF1 == 0) && (m_iH%iF2 == 0)) {
                vFactors2.push_back((ushort)iF1);
                // calculate "squareness deviation" (1: square, <1: rectangle)
                float f = fdiv(m_iW*iF2,m_iH*iF1);
                if (f > 1) {
                    f = 1/f;
                }
                if (f >= fBest)  {
                    iBest = iC;
                    fBest = f;
                }
                iC++;
            }
        }

    }
    return iBest;
}
