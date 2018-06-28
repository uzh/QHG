#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <omp.h>

#include "/usr/include/valgrind/memcheck.h"

#include "types.h"
#include "utils.h"
#include "colors.h"
#include "geomutils.h"
#include "LineReader.h"
#include "QDFUtils.h" 
#include "QDFArray.h" 
#include "AnalysisUtils.h"
#include "ArrivalChecker.h"

#define RADIUS 6371.3


//----------------------------------------------------------------------------
// constructor
//
ArrivalChecker::ArrivalChecker() 
    : m_iNumCells(0),
      m_pCellIDs(NULL),
      m_pTravelTimes(NULL),
      m_pTravelDists(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
ArrivalChecker::~ArrivalChecker() {

    deleteArrays();

}


//----------------------------------------------------------------------------
// createInstance
//
ArrivalChecker *ArrivalChecker::createInstance(const char *pQDFGrid, const char *pQDFStats, const char *pLocFile, double dDistance) {
    ArrivalChecker *pAC = new ArrivalChecker();
    int iResult = pAC->init(pQDFGrid, pQDFStats, pLocFile, dDistance);
    if (iResult != 0) {
        delete pAC;
        pAC = NULL;
    }
    return pAC;
}


//----------------------------------------------------------------------------
// deleteArrays
//
void ArrivalChecker::deleteArrays() {

    if (m_pCellIDs != NULL) {
        delete[]  m_pCellIDs;
    }
    m_pCellIDs = NULL;

    if (m_pTravelTimes != NULL) {
        delete[]  m_pTravelTimes;
    }
    m_pTravelTimes = NULL;

    if (m_pTravelDists != NULL) {
        delete[]  m_pTravelDists;
    }
    m_pTravelDists = NULL;

    if (m_pmCandidates != NULL) {
        delete[] m_pmCandidates;
    }
    m_pmCandidates = NULL;
}


//----------------------------------------------------------------------------
// init
//
int ArrivalChecker::init(const char *pQDFGrid, const char *pQDFStats, const char *pLocFile, double dDistance) {
    int iResult = -1;

    iResult = fillCoordMap(pQDFGrid);
    if (iResult == 0) {
        iResult = readStats(pQDFStats);
        if (iResult == 0) {
            if (iResult == 0) {
                locspec locSpec(pLocFile, dDistance, 0);
                m_vNames.clear();
 
                iResult = fillLocData(&locSpec, m_mLocData, &m_vNames);
                if (iResult == 0) { 
                    m_pmCandidates = new loccelldist[omp_get_max_threads()];

                }
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findClosestCandidates
//
int ArrivalChecker::findClosestCandidates(bool bSpherical) {
    int iResult = 0;

    if (bSpherical) {
        iResult = calcSphericalDistances();
    } else {
        iResult = calcCartesianDistances();
    }

    // accumulate
    for (int iT = 1; iT < omp_get_max_threads(); ++iT) {
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            m_pmCandidates[0][it->first].insert(m_pmCandidates[0][it->first].end(),
                                                m_pmCandidates[iT][it->first].begin(),
                                                m_pmCandidates[iT][it->first].end());
        }
    }

    // find closest: replace vector of candidates with a one-element vector of closest
    loc_data::const_iterator it;
    for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
        double dMin = 1e9;
        int iCellMin = -1;
        std::vector<celldist> &v = m_pmCandidates[0][it->first];
        for (uint i = 0; i < v.size(); ++i) {
            if (m_pTravelTimes[v[i].first] >= 0) {
                if (v[i].second < dMin) {
                    dMin = v[i].second;
                    iCellMin = v[i].first;
                }
            }
        }
        v.clear();
        v.push_back(celldist(iCellMin, (dMin<1e9)?dMin:-1));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// show
//
void ArrivalChecker::show(bool bNice, bool bSort) {
    

    if (bSort) {
        std::sort(m_vNames.begin(), m_vNames.end());
    }


    if (bNice) {

        uint iMaxL = 0;
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            uint iL = strlen(it->first.c_str());
            if (iL > iMaxL) {
                iMaxL = iL;
            }
        }
        
        stringvec::const_iterator itn;
        for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
            locitem &lItem = m_mLocData[*itn];
            int    iCellMin = m_pmCandidates[0][*itn][0].first;
            double dMin     = m_pmCandidates[0][*itn][0].second;
            printf("%-*s  (%+7.2f,%+6.2f;%6.1f):\tT %7.1f D %7.1f d%6.1f\n", 
                   iMaxL,
                   (*itn).c_str(), 
                   lItem.dLon, 
                   lItem.dLat, 
                   lItem.dDist,
                   (dMin < 0)?-1:m_pTravelTimes[iCellMin], 
                   (dMin < 0)?-1:m_pTravelDists[iCellMin], 
                   ((iCellMin >= 0) && (m_pTravelTimes[iCellMin] >= 0))?dMin:-1.0);
          
        }

    } else {

        printf("#Location\tLon\tLat\tSampDist\tTravelTime\tTravelDist\tmindist\n");
        stringvec::const_iterator itn;
        for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
            locitem &lItem = m_mLocData[*itn];
            int    iCellMin = m_pmCandidates[0][*itn][0].first;
            double dMin     = m_pmCandidates[0][*itn][0].second;
            printf("%s\t%6.2f\t%5.2f\t%6.1f\t%7.1f\t%7.1f\t%6.1f\n", 
                   (*itn).c_str(), 
                   lItem.dLon, 
                   lItem.dLat, 
                   lItem.dDist,
                   (dMin < 0)?-1:m_pTravelTimes[iCellMin], 
                   (dMin < 0)?-1:m_pTravelDists[iCellMin], 
                   ((iCellMin >= 0) && (m_pTravelTimes[iCellMin] >= 0))?dMin:-1.0);
        }
    }
}


//----------------------------------------------------------------------------
// readStats
//
int ArrivalChecker::readStats(const char *pQDFStats) {
    int iResult = -1;

                     
   QDFArray *pQA2 = QDFArray::create(pQDFStats);
    if (pQA2 != NULL) {
        iResult = pQA2->openArray(MSTATGROUP_NAME, MSTAT_DS_TIME);
        if (iResult == 0) {
            m_iNumCells = pQA2->getSize();
            m_pTravelTimes = new double[m_iNumCells];
            uint iCount = pQA2->getFirstSlab(m_pTravelTimes, m_iNumCells);
            if (iCount == m_iNumCells) {
                //                printf("Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                fprintf(stderr, "%sRead bad number of grid IDs from [%s:%s/%s]: %d (instead of %d)%s\n", 
                        RED, pQDFStats, MSTATGROUP_NAME, MSTAT_DS_DIST, iCount, m_iNumCells, OFF);
                iResult = -1;
            }
            pQA2->closeArray();
        } else {
            iResult = -1;
            fprintf(stderr, "%sCouldn't open QDF array for [%s:%s/%s]%s\n", 
                    RED, pQDFStats, MSTATGROUP_NAME, MSTAT_DS_TIME, OFF);
        }
        if (iResult == 0) {
            iResult = pQA2->openArray(MSTATGROUP_NAME, MSTAT_DS_DIST);
            if (iResult == 0) {
                //iNumCells = pQA2->getSize();
                m_pTravelDists = new double[m_iNumCells];
                uint iCount = pQA2->getFirstSlab(m_pTravelDists, m_iNumCells);
                if (iCount == m_iNumCells) {
                    //                printf("Read %d CellIDs\n", iCount);
                    iResult = 0;
                } else {
                    fprintf(stderr, "%sRead bad number of grid IDs from [%s:%s/%s]: %d (instead of %d)%s\n", 
                            RED, pQDFStats, MSTATGROUP_NAME, MSTAT_DS_DIST, iCount, m_iNumCells, OFF);
                    iResult = -1;
                }
                pQA2->closeArray();
            } else {
                iResult = -1;
                fprintf(stderr, "%sCouldn't open QDF array for [%s:%s/%s]%s\n", 
                        RED, pQDFStats, MSTATGROUP_NAME, MSTAT_DS_DIST, OFF);
            }
            
        }
        delete pQA2;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//
int ArrivalChecker::fillCoordMap(const char *pQDFGeoGrid) {
    int iResult = -1;
  
    double *pdLon = NULL;
    double *pdLat = NULL;
    char sPath[512];
    
    QDFArray *pQA = QDFArray::create(pQDFGeoGrid);
    if (pQA != NULL) {
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            m_iNumCells = pQA->getSize();
            m_pCellIDs = new int[m_iNumCells];
            uint iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumCells, GRID_DS_CELL_ID);
            if (iCount == m_iNumCells) {
                //                printf("Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                fprintf(stderr, "%sRead bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)%s\n", 
                        RED, pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME, GRID_DS_CELL_ID, iCount, m_iNumCells, OFF);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            fprintf(stderr, "%sCouldn't open QDF array for [%s:%s/%s]%s\n", 
                    RED, pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME, OFF);
        }

        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == m_iNumCells) {
                    pdLon = new double[m_iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, m_iNumCells);
                    if (iCount == m_iNumCells) {
                        //                    printf("Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        fprintf(stderr, "%sRead bad number of read longitudes from [%s:%s/%s]: %d instead of %d%s\n", 
                                RED, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, m_iNumCells, OFF);
                    }
                } else {
                    iResult = -1;
                    fprintf(stderr, "%sNumber of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", 
                            RED, iNumCellsL, m_iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, OFF);
                }
                pQA->closeArray();
            }
        }

        if (iResult == 0) {
            sprintf(sPath, "%s/%s", GEOGROUP_NAME, GEO_DS_LATITUDE);
            iResult = pQA->openArray(sPath);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == m_iNumCells) {
                    pdLat = new double[m_iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, m_iNumCells);
                    if (iCount == m_iNumCells) {
                        //                        printf("Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        fprintf(stderr, "%sCouldn't read latitudes from [%s:%s/%s]: %d instead of %d%s\n", 
                                RED, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iNumCellsL,m_iNumCells, OFF);
                    }
                } else {
                    iResult = -1;
                    fprintf(stderr, "%sNumber of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", 
                            RED, iNumCellsL, m_iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, OFF);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        fprintf(stderr, "%sCouldn't create QDFArray%s\n", RED, OFF);
    }

 
     
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < m_iNumCells; i++) {
            m_mCoords[m_pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
 
        fprintf(stderr, "  read cell coordinates: %zd items\n", m_mCoords.size());
    }

    if (pdLon != NULL) {
        delete[] pdLon;
    }
    if (pdLat != NULL) {
        delete[] pdLat;
    }

    return iResult;
}



//----------------------------------------------------------------------------
// SphericalDistance
//
int ArrivalChecker::calcSphericalDistances() {
    int iResult = 0;
#pragma omp parallel for
    for (uint iCell = 0; iCell < m_iNumCells; ++iCell) {
        std::pair<double,double> &coords = m_mCoords[iCell];
        double dLon0 = coords.first*M_PI/180;
        double dLat0 = coords.second*M_PI/180;
        double dX0 = cos(dLon0)*cos(dLat0);    
        double dY0 = sin(dLon0)*cos(dLat0);    
        double dZ0 = sin(dLat0);    
 
        int iT = omp_get_thread_num();
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            double dLon1 = it->second.dLon*M_PI/180;
            double dLat1 = it->second.dLat*M_PI/180;
            double dX1   = cos(dLon1)*cos(dLat1);    
            double dY1   = sin(dLon1)*cos(dLat1);    
            double dZ1   = sin(dLat1);    
            double dProd = dX0*dX1+dY0*dY1+dZ0*dZ1;
            if (dProd > 1) {
                dProd = 1;
            } else if (dProd < -1) {
                dProd = -1;
            }
            double dDist = RADIUS * acos(dProd);
            
            if (dDist < it->second.dDist) {
                m_pmCandidates[iT][it->first].push_back(celldist(iCell, dDist));
            }
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// CartesianDistance
//
int ArrivalChecker::calcCartesianDistances() {
    int iResult = 0;
#pragma omp parallel for
    for (uint iCell = 0; iCell < m_iNumCells; ++iCell) {
        std::pair<double,double> &coords = m_mCoords[iCell];
        double dX0 = coords.first;
        double dY0 = coords.second;
 
        int iT = omp_get_thread_num();
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            double dX1 = it->second.dLon;
            double dY1 = it->second.dLat;
            double dDist = sqrt((dX0-dX1)*(dX0-dX1) + (dY0-dY1)*(dY0-dY1));
            
            if (dDist < it->second.dDist) {
                m_pmCandidates[iT][it->first].push_back(celldist(iCell, dDist));
            }
        }
    }
    return iResult;
}

