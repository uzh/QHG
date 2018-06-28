#include <stdio.h>
#include <math.h>
#include <algorithm>

#include "types.h"
#include "smallprimes.h"
#include "icoutil.h"
#include "Region.h"
#include "LonLatRegion.h"
#include "FullRegion.h"
#include "RegionSplitter.h"
#include "LonLatSplitter.h"



//-----------------------------------------------------------------------------
// 
//
LonLatSplitter::LonLatSplitter(int iNX, int iNY, double dCapSize)
    : m_iNX(iNX),
      m_iNY(iNY),
      m_dCapSize(dCapSize*M_PI/180),
      m_bFree(false) {
    m_pBox = NULL;
    printf("LonLatSplitter %dx%d C:%f\n", iNX, iNY, dCapSize);
    m_iNumRegions = iNX*iNY;

}

//-----------------------------------------------------------------------------
// 
//
LonLatSplitter::LonLatSplitter(int iNumTiles, double dCapSize, bool bFree)
    : m_iNX(0),
      m_iNY(0),
      m_dCapSize(dCapSize*M_PI/180),
      m_bFree(bFree) {
    m_pBox = NULL;
    printf("LonLatSplitter %d C:%f\n", iNumTiles, dCapSize);
    m_iNumRegions = iNumTiles;
}


// forward declarations
void recursiveSplit(VECUS vPrimes, VECUS vPowers, 
                    ushort iStart,  ushort iF, 
                    VECUS &vFactors);
void squareSplit(int iN, int *piF1, int *piF2, int iLim);


//-----------------------------------------------------------------------------
// createRegions
//
Region **LonLatSplitter::createRegions(int *piNumTiles) {
    Region **pRegions = NULL;
    if (m_iNumRegions == 1) {
        pRegions = new Region*[1];
        pRegions[0] = new FullRegion(0);
        *piNumTiles = m_iNumRegions;
        printf("Trivial\n");
    } else {
        *piNumTiles = 0;
        if (m_pBox != NULL) {
            printf("have a box\n");
            //            if (m_bCaps) {
            if (m_dCapSize != 0) {
                printf("Caps\n");
                pRegions = createTilesBalancedCaps();
            } else {
                printf("normal\n");
                pRegions = createTilesBalanced();
            }
            *piNumTiles = m_iNumRegions;
        } else {
            printf("[LonLatSplitter::createRegions]**** No box provided\n");
        }
    }
    return pRegions;
}




//-----------------------------------------------------------------------------
// createTilesBalancedCaps
//  Since the area of a zone between latitudes phi1 and phi2 is 
//     2*PI*R*(sin(phi2)-sin(phi1))
//  we get "equal" area zones if the sinuses are equally spaced
//
Region **LonLatSplitter::createTilesBalancedCaps() {
    double PIP = M_PI+0.01;
    Region **apTiles = new Region*[m_iNumRegions];
    double dLat1 = 0;
    if (m_dCapSize < 0) {
        dLat1 = asin(1.0-2.0/m_iNumRegions);
    } else {
        dLat1 = m_dCapSize;
    }
   
    apTiles[0] = new LonLatRegion(0, -PIP, PIP, dLat1, PIP/2);
    apTiles[1] = new LonLatRegion(1, -PIP, PIP, -PIP/2, -dLat1);
    int iNextTile = 2;

    if ((m_iNX == 0) || (m_iNY == 0)) {
        squareSplit(m_iNumRegions-2, &m_iNX, &m_iNY, m_bFree?0:3);
    }
    printf("Using 2+%dx%d\n", m_iNX, m_iNY);

    double ddLon=2*M_PI/m_iNX;

    

    // double ddLon = (m_pBox->dLonMax - m_pBox->dLonMin)/m_iNX;

    double a0 = sin(-dLat1);
    double da = (sin(dLat1)-a0)/m_iNY;
    double dLaPrev = -dLat1; 
    for (int i = 1; i <= m_iNY; i++) {
        double dLaNew = asin(a0+i*da);
        if (i == m_iNY) {
            dLaNew = dLat1;
        } 
        //      printf("  %24.21f - %24.21f\n", dLaPrev, dLaNew); 
        for (int j = 0; j < m_iNX; j++) {
            double dLo = m_pBox->dLonMin + j*ddLon;
            if (j < m_iNX-1) {
                apTiles[iNextTile] = new LonLatRegion(iNextTile, dLo, dLo+ddLon, dLaPrev, dLaNew);
            } else {
                apTiles[iNextTile] = new LonLatRegion(iNextTile, dLo, m_pBox->dLonMax, dLaPrev, dLaNew);
            }
            iNextTile++;
        }
        dLaPrev = dLaNew;
    }
    for (int i = 0; i < m_iNumRegions; i++) {
        apTiles[i]->display();
    }

    return apTiles;
}

#define EPS (1e-9)
//-----------------------------------------------------------------------------
// createTilesBalanced
//  Since the area of a zone between latitudes phi1 and phi2 is 
//     2*PI*R*(sin(phi2)-sin(phi1))
//  we get "equal" area zones if the sinuses are equally spaced
//
Region **LonLatSplitter::createTilesBalanced() {

    Region **apTiles = new Region*[m_iNumRegions];
    int iC = 0;

    if ((m_iNX == 0) || (m_iNY == 0)) {
        squareSplit(m_iNumRegions, &m_iNX, &m_iNY, m_bFree?0:3);
    }
    printf("Using %dx%d\n", m_iNX, m_iNY);


    double ddLon = (m_pBox->dLonMax - m_pBox->dLonMin)/m_iNX;

    double a0 = sin(m_pBox->dLatMin-EPS);
    double da = (sin(m_pBox->dLatMax)-a0)/m_iNY;
    double dLaPrev = m_pBox->dLatMin-EPS; 
    for (int i = 1; i <= m_iNY; i++) {
        double dLaNew = asin(a0+i*da)-EPS;
        if (i == m_iNY) {
            dLaNew = m_pBox->dLatMax;
        } 
        printf("  %24.21f - %24.21f\n", dLaPrev, dLaNew); 
        for (int j = 0; j < m_iNX; j++) {
            double dLo = m_pBox->dLonMin-1e-9 + j*ddLon;
            if (j < m_iNX-1) {
                apTiles[iC] = new LonLatRegion(iC, dLo, dLo+ddLon, dLaPrev, dLaNew);
            } else {
                apTiles[iC] = new LonLatRegion(iC, dLo, m_pBox->dLonMax, dLaPrev, dLaNew);
            }
            iC++;
        }
        dLaPrev = dLaNew;
    }
    return apTiles;
}

//-----------------------------------------------------------------------------
// recursiveSplit
//   find all factors creatable from the given primes and their powers
//
void recursiveSplit(VECUS vPrimes, 
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

//-----------------------------------------------------------------------------
// squareSplit
//   try to find factors of N which are as close as possible to  sqrt(N)
//
void squareSplit(int iN, int *piF1, int *piF2, int iLim) {
    VECUS vPrimes;
    VECUS vPowers;
    primiFactor((ushort)iN, vPrimes, vPowers);
    VECUS vFactors;
    recursiveSplit(vPrimes, vPowers, 0, 1, vFactors);
    
    std::sort(vFactors.begin(), vFactors.end());
    printf("factors:\n");
    for (uint i = 0; i < vFactors.size(); ++i) {
        printf(" %d \n", vFactors[i]);
    }
    printf("\n");
    
   
    int iFactor0 = 1;
    int iFactorRes = 1;
    double dAspect=0;
    uint i = 1;
    double dLim = (iLim==0)?(int)(sqrt(iN)+0.5):iLim; // if the stripes get too small there may be no land in them
    while ((i < vFactors.size()) && (iFactor0 <= dLim)) {
        
        int iFactor1 = iN/iFactor0;
        double dA = (1.0*iFactor0)/(1.0*iFactor1);
        if (dA > dAspect) {
            dAspect = dA;
            iFactorRes = iFactor0;
        }
        iFactor0 = vFactors[i++];
    }
    *piF1 = iFactorRes;
    *piF2 = iN/iFactorRes;
}
