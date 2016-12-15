#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <omp.h>

#include "types.h"
#include "utils.h"
#include "geomutils.h"
#include "LineReader.h"
#include "QDFUtils.h" 
#include "QDFArray.h" 
#include "IDSampler.h"

#define RADIUS 6371.3
#define SPECIAL_CHAR "~"

//----------------------------------------------------------------------------
// constructor
//
IDSampler::IDSampler() 
    : m_iNumAgents(0),
      m_pIDs(NULL), 
      m_pCellIDs(NULL),
      m_pGenders(NULL),
      m_pHits(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
IDSampler::~IDSampler() {

    deleteArrays();

}


//----------------------------------------------------------------------------
// createInstance
//
IDSampler *IDSampler::createInstance(const char *pQDFGrid) {
    IDSampler *pIS = new IDSampler();
    int iResult = pIS->init(pQDFGrid);
    if (iResult != 0) {
        delete pIS;
        pIS = NULL;
    }
    return pIS;
}


//----------------------------------------------------------------------------
// deleteArrays
//
void IDSampler::deleteArrays() {
    if (m_pIDs != NULL) {
        delete[]  m_pIDs;
    }
    m_pIDs = NULL;

    if (m_pCellIDs != NULL) {
        delete[]  m_pCellIDs;
    }
    m_pCellIDs = NULL;

    if (m_pGenders != NULL) {
        delete[]  m_pGenders;
    }
    m_pGenders = NULL;

    if (m_pHits != NULL) {
        delete[]  m_pHits;
    }
    m_pHits = NULL;

}


//----------------------------------------------------------------------------
// init
//
int IDSampler::init(const char *pQDFGrid) {
    int iResult = -1;

    iResult = fillCoordMap(pQDFGrid);

    return iResult;
}


//----------------------------------------------------------------------------
// getSamples
//
int IDSampler::getSamplesP(const char *pQDFTime, const char *pPopName, const char *pLocFile, 
                          locdata  &mLocData, stringvidmap &mvIDs, idagdatamap &mAgentData) {
    int iResult = -1;

    iResult = readArrays(pQDFTime, pPopName);

    stringvec vNames;
    if (iResult == 0) {
        iResult = fillLocData(pLocFile, vNames, mLocData);
    }

    if (iResult == 0) {
        stringvidmap mvTemp; 
        idagdatamap mAgentDataTemp;
        locdata::const_iterator it;
        for (it = mLocData.begin(); (iResult == 0) && it != mLocData.end(); ++it) {
            printf("Getting IDs for %s\n", it->first.c_str()); fflush(stdout);
            double dLo = it->second.dLon;
            double dLa = it->second.dLat;

            if (it->first.find(SPECIAL_CHAR) == 0) {
                // ordinary cartesian grid distance
#pragma omp parallel for
                for (int i = 0; i < m_iNumAgents; i++) {
                    double dLon0 = m_mCoords[m_pCellIDs[i]].first;
                    double dLat0 = m_mCoords[m_pCellIDs[i]].second;
                    
                    m_pHits[i] = (it->second.dDist > sqrt((dLon0-dLo)*(dLon0-dLo) + (dLat0-dLa)*(dLat0-dLa)));
                }
            } else {
                // normal spherical distance
#pragma omp parallel for
                for (int i = 0; i < m_iNumAgents; i++) {
                    double dLon0 = m_mCoords[m_pCellIDs[i]].first*M_PI/180;
                    double dLat0 = m_mCoords[m_pCellIDs[i]].second*M_PI/180;
                    m_pHits[i] = (it->second.dDist > spherdist(dLon0, dLat0, dLo, dLa, RADIUS));
                }
            }

            // save the hits
            for (int i = 0; i < m_iNumAgents; i++) {
                if (m_pHits[i]) {
                    mvTemp[it->first].push_back(m_pIDs[i]);    
                    
                    // we put the required data into an agent data struct
                    // which must be deleted when mAgentDataTemmp is not needed anymore
                    agdata *pAD = new agdata;
                    pAD->iMomID = -1;
                    pAD->iDadID = -1;
                    pAD->iGender = m_pGenders[i];
                    pAD->iCellID = m_pCellIDs[i];
                    pAD->dLon = m_mCoords[m_pCellIDs[i]].first;
                    pAD->dLat = m_mCoords[m_pCellIDs[i]].second;
                    mAgentDataTemp[m_pIDs[i]] = pAD;
                }
            }
        }
            
        if (iResult == 0) {
            // select requested number
            stringvidmap::iterator it2;
            for (it2 = mvTemp.begin(); it2 != mvTemp.end(); ++it2) {
                idset sTemp;
                ulong iRealNum = 0;
                int iListNum =  mLocData[it2->first].iNum;
                bool bSelect = true;
                
                if (iListNum < 0) {
                    bSelect = false;
                } else {
                    iRealNum = iListNum;

                    if (iRealNum > it2->second.size()) {
                        bSelect = false;
                    } else {
                        bSelect = true;
                    }
                }

                int iWidth = 1;
                if (iRealNum > 9) {
                    iWidth = 1+(int)(log((double)iRealNum-1)/log(10));
                
}
                if (bSelect) {
                    while (sTemp.size() < iRealNum) {
                        int iIndex = (int)(((double)it2->second.size()*rand())/RAND_MAX);
                        sTemp.insert(it2->second[iIndex]);
                    }
                } else {
                    sTemp.insert(it2->second.begin(), it2->second.end());
                }
                printf("  found % *zd ids for [%s]\n", iWidth, sTemp.size(), it2->first.c_str());fflush(stdout);
                std::vector<idtype> &v = mvIDs[it2->first];
                v.insert(v.end(), sTemp.begin(), sTemp.end());
                    
            }
        }

        /*
        if (iResult == 0) {
            // select requested number
            stringvidmap::iterator it2;
            for (it2 = mvTemp.begin(); it2 != mvTemp.end(); ++it2) {
                idset sTemp;
                ulong iRealNum = mLocData[it2->first].iNum;
                if (iRealNum > it2->second.size()) {
                    iRealNum = it2->second.size();
                }

                while (sTemp.size() < iRealNum) {
                    int iIndex = (int)(((double)it2->second.size()*rand())/RAND_MAX);
                    sTemp.insert(it2->second[iIndex]);
                    mAgentData[it2->second[iIndex]] =  mAgentDataTemp[it2->second[iIndex]];
                } 
                printf("  found %zd ids for [%s]\n", sTemp.size(), it2->first.c_str());fflush(stdout);
                std::vector<idtype> &v = mvIDs[it2->first];
                v.insert(v.end(), sTemp.begin(), sTemp.end());
            }
        }
        */
    } else {
        printf("Couldn't fill loc data\n");
    }
    //    deleteArrays();
    return iResult;
}


//----------------------------------------------------------------------------
// getSamples
//
int IDSampler::getSamples(const char *pQDFTime, const char *pPopName, const char *pLocFile, 
                          locdata  &mLocData, stringvidmap &mvIDs, idagdatamap &mAgentData) {
    int iResult = -1;

    iResult = readArrays(pQDFTime, pPopName);

    stringvec vNames;
    if (iResult == 0) {
        iResult = fillLocData(pLocFile, vNames, mLocData);
    }

    if (iResult == 0) {
        stringvidmap mvTemp;
        // transform CellIDs to coords
        for (int i = 0; i < m_iNumAgents; i++) {
			double dLon0 = 0;
			double dLat0 = 0;
            bool bSearching = true;
            locdata::const_iterator it;
            for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
                double d = 0;
                if (it->first.find(SPECIAL_CHAR) == 0) {
                    // use flat grid distance
    	        	dLon0 = m_mCoords[m_pCellIDs[i]].first;
        	    	dLat0 = m_mCoords[m_pCellIDs[i]].second;
                    double dLo = it->second.dLon;
                    double dLa = it->second.dLat;
                    d = sqrt((dLon0 - dLo) * (dLon0 - dLo) + (dLat0 - dLa) * (dLat0 - dLa));
                } else {
	        	    // the coordinates are in degrees! make them radians
    	        	dLon0 = m_mCoords[m_pCellIDs[i]].first*M_PI/180;
        	    	dLat0 = m_mCoords[m_pCellIDs[i]].second*M_PI/180;
                    // normal spherical distance
                    d = spherdist(dLon0, dLat0, it->second.dLon, it->second.dLat, RADIUS);
                }
                if (d < it->second.dDist) {
                    mvTemp[it->first].push_back(m_pIDs[i]);

                    agdata *pAD = new agdata;
                    pAD->iMomID = -1;
                    pAD->iDadID = -1;
                    pAD->iGender = m_pGenders[i];
                    pAD->iCellID = m_pCellIDs[i];
                    pAD->dLon = dLon0;
                    pAD->dLat = dLat0;
                    mAgentData[m_pIDs[i]] = pAD;
                    
                    bSearching = false;
                    
                }
            }
        }
        
        if (iResult == 0) {
            // select requested number
            stringvidmap::iterator it2;
            for (it2 = mvTemp.begin(); it2 != mvTemp.end(); ++it2) {
                idset sTemp;
                ulong iRealNum = 0;
                int iListNum =  mLocData[it2->first].iNum;
                bool bSelect = true;
                
                if (iListNum < 0) {
                    bSelect = false;
                } else {
                    iRealNum = iListNum;

                    if (iRealNum > it2->second.size()) {
                        bSelect = false;
                    } else {
                        bSelect = true;
                    }
                }

                int iWidth = 1;
                if (iRealNum > 9) {
                    iWidth = 1+(int)(log((double)iRealNum-1)/log(10));
                
}
                if (bSelect) {
                    while (sTemp.size() < iRealNum) {
                        int iIndex = (int)(((double)it2->second.size()*rand())/RAND_MAX);
                        sTemp.insert(it2->second[iIndex]);
                    }
                } else {
                    sTemp.insert(it2->second.begin(), it2->second.end());
                }
                printf("  found % *zd ids for [%s]\n", iWidth, sTemp.size(), it2->first.c_str());fflush(stdout);
                std::vector<idtype> &v = mvIDs[it2->first];
                v.insert(v.end(), sTemp.begin(), sTemp.end());
                    
            }
        }
    } else {
        printf("Couldn't fill loc data\n");
    }
    //    deleteArrays();
    return iResult;
}

//----------------------------------------------------------------------------
// getSamples
//
int IDSampler::getSamples(stringvec &vQDFPops, const char *pPopName, const char *pLocFile, locdata &mLocData, loctimeids &mmvIDs, idagdatamap &mAgentData, bool bParallel) {
    int iResult = 0;
    printf("Heellllooo?\n");
    for (uint i = 0; (iResult == 0) && (i < vQDFPops.size()); ++i) {
        stringvidmap mvIDs;
        printf("Getting samples from [%s]\n", vQDFPops[i].c_str());fflush(stdout);
        if (bParallel) {
            iResult = getSamplesP(vQDFPops[i].c_str(), pPopName, pLocFile, mLocData, mvIDs, mAgentData);
        } else {
            iResult = getSamples(vQDFPops[i].c_str(), pPopName, pLocFile, mLocData, mvIDs, mAgentData);
        }
        if (iResult == 0) {
            // insert mvIDs to mmvIDs
            stringvidmap::const_iterator it;
            for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                std::vector<idtype> &vCur = mmvIDs[it->first][m_fTimeStep];
                vCur.insert(vCur.end(), it->second.begin(), it->second.end());
            }
        }
            
    }

    return iResult;
}




//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
int IDSampler::getAttributes(const char *pQDFTime, const char *pPopName, const char *pLocFile, 
                             locdata  &mLocData, stringvidmap &mvIDs, idagdatamap &mAgentData, idset &sSelected) {
    int iResult = -1;

    iResult = readArrays(pQDFTime, pPopName);

    stringvec vNames;
    if (iResult == 0) {
        iResult = fillLocData(pLocFile, vNames, mLocData);
    }

    if (iResult == 0) {
        // transform CellIDs to coords
        for (int i = 0; i < m_iNumAgents; i++) {
            idset_cit it0 = sSelected.find(m_pIDs[i]);
            if (it0 != sSelected.end()) {
                // the coordinates are in degrees! make them radians
                double dLon0 = m_mCoords[m_pCellIDs[i]].first*M_PI/180;
                double dLat0 = m_mCoords[m_pCellIDs[i]].second*M_PI/180;
                bool bSearching = true;
                locdata::const_iterator it;
                for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
                    double d = spherdist(dLon0, dLat0, it->second.dLon, it->second.dLat, RADIUS);
                    if (d < it->second.dDist) {
                        mvIDs[it->first].push_back(m_pIDs[i]);
                        
                        agdata *pAD = new agdata;
                        pAD->iMomID  = -1;
                        pAD->iDadID  = -1;
                        pAD->iGender = m_pGenders[i];
                        pAD->iCellID = m_pCellIDs[i];
                        pAD->dLon = dLon0;
                        pAD->dLat = dLat0;
                        mAgentData[m_pIDs[i]] = pAD;
                            
                        bSearching = false;

                    }
                }
            }
        }
        
        if (iResult == 0) {
            // show
            stringvidmap::iterator it2;
            for (it2 = mvIDs.begin(); it2 != mvIDs.end(); ++it2) {
                std::sort(it2->second.begin(), it2->second.end());
                int iWidth = 1;
                if (it2->second.size() > 9) {
                    iWidth = 1+(int)(log((double)it2->second.size())/log(10));
                }
                // make unused rands (to have the same state later)
                for (uint i = 0; i < it2->second.size(); i++) {
                    rand();
                }
                printf("  found attributes for  % *zd ids at [%s]\n", iWidth, it2->second.size(), it2->first.c_str());
            }
        }
    } else {
        printf("Couldn't fill loc data\n");
    }
    //    deleteArrays();
    return iResult;
}

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
int IDSampler::getAttributes(stringvec &vQDFPops, const char *pPopName, const char *pLocFile, locdata  &mLocData, loctimeids &mmvIDs, idagdatamap &mAgentData, idset &sSelected) {
    int iResult = 0;
    for (uint i = 0; (iResult == 0) && (i < vQDFPops.size()); ++i) {
        stringvidmap mvIDs;
        
        iResult = getAttributes(vQDFPops[i].c_str(), pPopName, pLocFile, mLocData, mvIDs, mAgentData, sSelected);

        if (iResult == 0) {
            // insert mvIDs to mmvIDs
            stringvidmap::const_iterator it;
            for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                std::vector<idtype> &vCur = mmvIDs[it->first][m_fTimeStep];
                vCur.insert(vCur.end(), it->second.begin(), it->second.end());
            }
        }
            
    }    return iResult;
}

//----------------------------------------------------------------------------
// readArrays
//
int IDSampler::readArrays(const char *pQDFTime, const char *pPopName) {
    int iResult = -1;

    deleteArrays();

    m_pIDs        = NULL;
    m_pCellIDs    = NULL;
    m_pGenders    = NULL;

    m_iNumAgents  = 0;
    m_fTimeStep   = -1;

    QDFArray *pQA = QDFArray::create(pQDFTime);
    if (pQA != NULL) {
        m_fTimeStep = pQA->getTimeStep();
        iResult = pQA->openArray(POPGROUP_NAME, pPopName, AGENT_DATASET_NAME);
        if (iResult == 0) {
            m_iNumAgents = pQA->getSize();

            m_pIDs      = new idtype[m_iNumAgents];
            m_pCellIDs  = new int[m_iNumAgents];
            m_pGenders  = new int[m_iNumAgents];
            m_pHits     = new bool[m_iNumAgents];
            
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pIDs, m_iNumAgents, "AgentID");
                if (iCount != m_iNumAgents) {
                    printf("Got %d agent IDs instead of %d\n", iCount, m_iNumAgents);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumAgents, "CellID");
                if (iCount != m_iNumAgents) {
                    printf("Got %d cell IDs instead of %d\n", iCount, m_iNumAgents);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pGenders, m_iNumAgents, "Gender");
                if (iCount != m_iNumAgents) {
                    printf("Got %d genders instead of %d\n", iCount, m_iNumAgents);
                    iResult = -1;
                }
            }
                
            if (iResult == 0) {
                printf("  read agent data: %d items\n", m_iNumAgents);
            }
        } else {
            printf("Couldn't open dataset [%s/%s%s]\n", POPGROUP_NAME, pPopName, AGENT_DATASET_NAME);
        }
        pQA->closeArray();

        delete pQA;
    } else {
        iResult = -1;
        printf("Couldn't open file [%s]\n", pQDFTime);
    }
    return iResult;
}



                     

//----------------------------------------------------------------------------
// fillCoordMap
//
int IDSampler::fillCoordMap(const char *pQDFGeoGrid) {
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
    char sPath[512];
    
    QDFArray *pQA = QDFArray::create(pQDFGeoGrid);
    if (pQA != NULL) {
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //                printf("Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                printf("Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            printf("Couldn't open QDF array for [%s:%s/%s]\n", pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME);
        }

        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLon = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, iNumCells);
                    if (iCount == iNumCells) {
                        //                    printf("Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        printf("Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells);
                    }
                } else {
                    iResult = -1;
                    printf("Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        if (iResult == 0) {
            sprintf(sPath, "%s/%s", GEOGROUP_NAME, GEO_DS_LATITUDE);
            iResult = pQA->openArray(sPath);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        //                        printf("Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        printf("Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells);
                    }
                } else {
                    iResult = -1;
                    printf("Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        printf("Couldn't create QDFArray\n");
    }
     
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < iNumCells; i++) {
            m_mCoords[pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
 
        printf("  read cell coordinates: %zd items\n", m_mCoords.size());
    }

    if (pCellIDs != NULL) {
        delete[] pCellIDs;
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
// fillLocData
//
int IDSampler::fillLocData(const char *pLocFile, stringvec &vNames, locdata &mLocData) {
    int iResult = -1;
    
    LineReader *pLR = LineReader_std::createInstance(pLocFile, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        iResult = 0;
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            char sName[128];

            int iReq = 5;
            int iC = 0;
            locitem li;
            char *pNEnd = strchr(pLine, ']');
            if (pNEnd != NULL) {
                pNEnd++;
                *pNEnd = '\0';
                strcpy(sName, pLine);
                pLine = pNEnd+1;
                iReq = 4;
                iC = sscanf(pLine, "%lf %lf %lf %d", &li.dLon, &li.dLat, &li.dDist, &li.iNum);
            } else {
                iC = sscanf(pLine, "%s %lf %lf %lf %d", sName, &li.dLon, &li.dLat, &li.dDist, &li.iNum);
            }
            if (iC == iReq) {
                // the coordinates are in degrees! make them radians
                li.dLon *= M_PI/180;
                li.dLat *= M_PI/180;
                vNames.push_back(sName);
                mLocData[sName] = li;
            } else {
                printf("Couldn't read enough items from line [%s]\n", pLine);
                iResult = -1;
            }
            pLine = pLR->getNextLine();
        }

        delete pLR;
    } else {
        printf("Couldn't open file [%s]\n", pLocFile);
    }

    return iResult;
}
