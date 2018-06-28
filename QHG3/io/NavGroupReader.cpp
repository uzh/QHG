#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Navigation.h"
#include "QDFUtils.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "NavGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
NavGroupReader::NavGroupReader() {
}


//----------------------------------------------------------------------------
// createNavGroupReader
//
NavGroupReader *NavGroupReader::createNavGroupReader(const char *pFileName) {
    NavGroupReader *pNR = new NavGroupReader();
    int iResult = pNR->init(pFileName, NAVGROUP_NAME);
    if (iResult != 0) {
        delete pNR;
        pNR = NULL;
    }
    return pNR;
}

//----------------------------------------------------------------------------
// createNavGroupReader
//
NavGroupReader *NavGroupReader::createNavGroupReader(hid_t hFile) {
    NavGroupReader *pNR = new NavGroupReader();
    int iResult = pNR->init(hFile, NAVGROUP_NAME);
    if (iResult != 0) {
        delete pNR;
        pNR = NULL;
    }
    return pNR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int NavGroupReader::tryReadAttributes(NavAttributes *pAttributes) {
    // we do *not* call the super class method, because navigation has no numcells
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV_ATTR_NUM_PORTS,   1, &(pAttributes->m_iNumPorts)); 
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV_ATTR_NUM_DESTS,   1, &(pAttributes->m_iNumDests)); 
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV_ATTR_NUM_DISTS,   1, &(pAttributes->m_iNumDists)); 
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV_ATTR_SAMPLE_DIST, 1, &(pAttributes->m_dSampleDist)); 
    }


    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int NavGroupReader::readArray(Navigation *pNG, const char *pArrayName) {
    int iResult = -1;

    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//
int NavGroupReader::readData(Navigation *pNG) {
    int iResult = 0;
    // no check for number of cells required 

    uint iNumPorts = m_pAttributes->m_iNumPorts;
    uint iNumDests = m_pAttributes->m_iNumDests;
    uint iNumDists = m_pAttributes->m_iNumDists;

   //create temporary arrays
    ushort *pMultiplicities  = new ushort[iNumPorts];
    int    *pDestinationIDs  = new int[iNumDests];
    double *pDistances       = new double[iNumDists];
    

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV_DS_MULTIPLICITIES, iNumPorts, pMultiplicities);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV_DS_DEST_IDS,       iNumDests, pDestinationIDs);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV_DS_DISTANCES,      iNumDists, pDistances);
    }

    // consistency check: sum of (pDestinationCounts+1) = sze of pDestinationCounts
    if (iResult == 0) {
        uint iSum = 0;
        for (uint i = 0; i < iNumPorts; ++i) {
            iSum += pMultiplicities[i]+1;
        }
        if (iSum == iNumDests) {
            if ((iSum - iNumPorts) == iNumDists) {
                iResult = 0;
            } 
        } else {
            iResult = -1;
        }
        if (iResult != 0) {
            printf("Consistency check failed:\n");
            printf("Num Ports %d\n", iNumPorts);
            printf("Num Dests %d, actual %d\n", iNumDests, iSum);
            printf("Num Dists %d, actual %d\n", iNumDists, iSum-iNumPorts);
        }
    }
    
    if (iResult == 0) {
        distancemap mDestinations;
        int    *pIDs   = pDestinationIDs;
        double *pDists = pDistances;
        for (uint i = 0; i < iNumPorts; ++i) {
            int iOrigin = *pIDs++;

            ushort iNumCurDests = pMultiplicities[i];
            distlist dl;
            for (ushort j = 0; j < iNumCurDests; j++) {
                int iDest = *pIDs++;
                double dDist = *pDists++;
                dl[iDest] = dDist;
            }
            mDestinations[iOrigin] = dl;
        }   
        
        pNG->setData(mDestinations, m_pAttributes->m_dSampleDist);
        iResult = pNG->checkSizes(iNumPorts, iNumDests, iNumDists);
        if (iResult == 0) {
        } else {
            printf("[NavReader] size mismatch!\n");
        }
    }


    delete[] pDistances;
    delete[] pDestinationIDs;
    delete[] pMultiplicities;
    return iResult;

    return iResult;
}
