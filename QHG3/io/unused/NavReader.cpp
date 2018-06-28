#include <stdio.h>
#include <hdf5.h>

#include "Navigation.h"
#include "QDFUtils.h"
#include "NavReader.h"


//----------------------------------------------------------------------------
// constructor
//
NavReader::NavReader()
    : m_hFile(H5P_DEFAULT),
      m_hNavGroup(H5P_DEFAULT),
      m_iNumPorts(0),
      m_iNumDests(0),
      m_iNumDists(0) {
}


//----------------------------------------------------------------------------
// destructor
//
NavReader::~NavReader() {
}


//----------------------------------------------------------------------------
// createNavReader
//
NavReader *NavReader::createNavReader(const char *pFileName) {
    NavReader *pNR = new NavReader();
    int iResult = pNR->init(pFileName);
    if (iResult != 0) {
        delete pNR;
        pNR = NULL;
    }
    return pNR;
}


//----------------------------------------------------------------------------
// createNavReader
//
NavReader *NavReader::createNavReader(hid_t hFile) {
    NavReader *pNR = new NavReader();
    int iResult = pNR->init(hFile);
    if (iResult != 0) {
        delete pNR;
        pNR = NULL;
    }
    return pNR;
}


//----------------------------------------------------------------------------
// init
//
int NavReader::init(const char *pFileName) {
    int iResult = -1;
    
    hid_t hFile = qdf_openFile(pFileName);
    if (hFile > 0) {
        iResult = init(hFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// init
//
int NavReader::init(hid_t hFile) {

    int iResult = -1;
    m_hNavGroup = qdf_openGroup(hFile, NAVGROUP_NAME);
    if (m_hNavGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}


//----------------------------------------------------------------------------
// readAttributes
//
int NavReader::readAttributes(uint *piNumPorts, uint *piNumDests, uint *piNumDists, double *pdSampleDist) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hNavGroup, NAV_ATTR_NUM_PORTS,   1, piNumPorts); 
        m_iNumPorts = *piNumPorts;
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hNavGroup, NAV_ATTR_NUM_DESTS,   1, piNumDests); 
        m_iNumDests = *piNumDests;
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hNavGroup, NAV_ATTR_NUM_DISTS,   1, piNumDists); 
        m_iNumDists = *piNumDists;
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hNavGroup, NAV_ATTR_SAMPLE_DIST, 1, pdSampleDist); 
        m_dSampleDist = *pdSampleDist;
    }
    //    printf("[NavReader] attributes: numportd %u, numdests %u, numdists %u\n", m_iNumPorts, m_iNumDests, m_iNumDists);
    return iResult;
}

//----------------------------------------------------------------------------
// readData
//
int NavReader::readData(Navigation *pNav) {
    int iResult = 0;
    
    //create temporary arrays
    ushort *pMultiplicities  = new ushort[m_iNumPorts];
    int    *pDestinationIDs  = new int[m_iNumDests];
    double *pDistances       = new double[m_iNumDists];
    

    if (iResult == 0) {
        iResult = qdf_readArray(m_hNavGroup, NAV_DS_MULTIPLICITIES, m_iNumPorts, pMultiplicities);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hNavGroup, NAV_DS_DEST_IDS,       m_iNumDests, pDestinationIDs);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hNavGroup, NAV_DS_DISTANCES,      m_iNumDists, pDistances);
    }

    // consistency check: sum of (pDestinationCounts+1) = sze of pDestinationCounts
    if (iResult == 0) {
        uint iSum = 0;
        for (uint i = 0; i < m_iNumPorts; ++i) {
            iSum += pMultiplicities[i]+1;
        }
        if (iSum == m_iNumDests) {
            if ((iSum - m_iNumPorts) == m_iNumDists) {
                iResult = 0;
            } 
        } else {
            iResult = -1;
        }
        if (iResult != 0) {
            printf("Consistency check failed:\n");
            printf("Num Ports %d\n", m_iNumPorts);
            printf("Num Dests %d, actual %d\n", m_iNumDests, iSum);
            printf("Num Dists %d, actual %d\n", m_iNumDists, iSum-m_iNumPorts);
        }
    }
    
    if (iResult == 0) {
        distancemap mDestinations;
        int    *pIDs   = pDestinationIDs;
        double *pDists = pDistances;
        for (uint i = 0; i < m_iNumPorts; ++i) {
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
        
        pNav->setData(mDestinations, m_dSampleDist);
        iResult = pNav->checkSizes(m_iNumPorts, m_iNumDests, m_iNumDists);
        if (iResult == 0) {
        } else {
            printf("[NavReader] size mismatch!\n");
        }
    }


    delete[] pDistances;
    delete[] pDestinationIDs;
    delete[] pMultiplicities;
    return iResult;
}
    
