#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <omp.h>

#include "types.h"
#include "utils.h"
#include "colors.h"
#include "geomutils.h"
#include "LineReader.h"
#include "QDFUtils.h" 
#include "QDFArray.h" 
#include "IDSample.h"
#include "IDSampler2.h"

#define RADIUS 6371.3
#define SPECIAL_CHAR "~"

//----------------------------------------------------------------------------
// constructor
//
IDSampler2::IDSampler2() 
    : m_iNumAgents(0),
      m_pIDs(NULL), 
      m_pCellIDs(NULL),
      m_pGenders(NULL),
      m_pCurSample(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
IDSampler2::~IDSampler2() {

    deleteArrays();

}


//----------------------------------------------------------------------------
// createInstance
//
IDSampler2 *IDSampler2::createInstance(const char *pQDFGrid) {
    IDSampler2 *pIS = new IDSampler2();
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
void IDSampler2::deleteArrays() {
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


}


//----------------------------------------------------------------------------
// init
//
int IDSampler2::init(const char *pQDFGrid) {
    int iResult = -1;

    iResult = fillCoordMap(pQDFGrid);

    return iResult;
}


//----------------------------------------------------------------------------
// makeSelectionList
//
int IDSampler2::makeSelectionList(int iNumTotal, int iNumSelect, intset &sSelectedIndexes) {

    int *aTemp = new int[iNumTotal];
    for (int i = 0; i< iNumTotal; i++) {
        aTemp[i] = i;
    }

    if (iNumSelect > iNumTotal) {
        iNumSelect = iNumTotal;
    }
    
    // repeated swapping of array elements will result in
    // iNumSelect randomly selected elements at the end of the array
    for (int i = iNumTotal-1; i > iNumTotal-iNumSelect-1; i--) {
        // swap the current element with a random lower
        int iIndex = (int)(((double)(i+1)*rand())/RAND_MAX);
        int t = aTemp[i];
        aTemp[i] = aTemp[iIndex];
        aTemp[iIndex] = t;
    }

    // copy to intset
    sSelectedIndexes.insert(aTemp+iNumTotal-iNumSelect, aTemp+iNumTotal);

    delete[] aTemp;
    return 0;
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamplesCore(const char *pQDFTime, const char *pPopName, locdata  &mLocData) {
    int iResult = 0;
    
    /*
    iResult = readArrays(pQDFTime, pPopName);

    if (bSingle) {
        m_pCurSample = new IDSample();
    }

    stringvec vNames;
    if (iResult == 0) {
        iResult = fillLocData(pLocSpec, vNames, mLocData);
    }
*/
    if (iResult == 0) {
        // loop through agents; for each region (M_i, d_i) get indexes of candidates
        locindexes mvCandidates;

        for (int i = 0; i < m_iNumAgents; i++) {
            double dLon0 = m_mCoords[m_pCellIDs[i]].first;
            double dLat0 = m_mCoords[m_pCellIDs[i]].second;
            
           
            // each agent should only belong to one region
            bool bSearching = true;
            locdata::const_iterator it;
            for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
                double d = 0;
                if (it->first.find(SPECIAL_CHAR) == 0) {
                    // use flat grid distance
                    double dLo = it->second.dLon;
                    double dLa = it->second.dLat;
                    d = sqrt((dLon0 - dLo) * (dLon0 - dLo) + (dLat0 - dLa) * (dLat0 - dLa));
                } else {
                    // the coordinates are in degrees! make them radians
                    // normal spherical distance
                    d = spherdist(dLon0*M_PI/180, dLat0*M_PI/180, it->second.dLon*M_PI/180, it->second.dLat*M_PI/180, RADIUS);
                }

                if (d < it->second.dDist) {
                    mvCandidates[it->first].push_back(i);
                    bSearching = false;
                }
            }
        }
    
        // create selection indexes for the indexes in mvCandidates 
        std::map<std::string, intset> msSelectedIndexes;
        locindexes::const_iterator it2;
        for (it2 = mvCandidates.begin(); it2 != mvCandidates.end(); ++it2) {
            std::set<int> sSelectedIndexes;
            makeSelectionList(it2->second.size(), mLocData[it2->first].iNum, sSelectedIndexes);
            msSelectedIndexes[it2->first] = sSelectedIndexes;
        }
        

        // now get details for selected candidates
        std::map<std::string, intset>::const_iterator itm;

        for (itm = msSelectedIndexes.begin(); (iResult == 0) && (itm != msSelectedIndexes.end()); ++itm) {
            const intset &sSelIdx = itm->second;
            const std::vector<int> &vCandidates = mvCandidates[itm->first];

            std::vector<agdata*> vAGD;
            intset_cit its;
            for (its = sSelIdx.begin(); its != sSelIdx.end(); ++its) {
                agdata *pAD = new agdata;
                int iCand = vCandidates[*its];
                pAD->iID = m_pIDs[iCand];
                pAD->iMomID = -1;
                pAD->iDadID = -1;
                pAD->iGender = m_pGenders[iCand];
                pAD->iCellID = m_pCellIDs[iCand];
                pAD->dLon = m_mCoords[pAD->iCellID].first;
                pAD->dLat = m_mCoords[pAD->iCellID].second;
                pAD->iIndex = iCand;
                vAGD.push_back(pAD);
            }
            m_pCurSample->addAgentDataVec(itm->first.c_str(), m_fTimeStamp, vAGD); 
        }
    } else {
        // mlocdata fail
    }
    if (iResult != 0) {
        delete m_pCurSample;
        m_pCurSample = NULL;
    }
    return m_pCurSample;
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(const char *pQDFTime, const char *pPopName, const locspec *pLocSpec, locdata  &mLocData) {
    
    IDSample *pNew = NULL;
    int iResult = 0;

    iResult = readArrays(pQDFTime, pPopName);
    if (iResult == 0) {
        m_pCurSample = new IDSample();
    
        stringvec vNames;
        if (iResult == 0) {
            iResult = fillLocData(pLocSpec, vNames, mLocData);
        }
    }
    if (iResult == 0) {
        pNew = getSamplesCore(pQDFTime, pPopName, mLocData);
    }
    return pNew;
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(stringvec &vQDFPops, const char *pPopName, const locspec *pLocSpec, locdata &mLocData) {

    int iResult = 0;

    m_pCurSample = new IDSample();

    stringvec vNames;
    iResult = fillLocData(pLocSpec, vNames, mLocData);
    if (iResult == 0) {
        IDSample *pNew = m_pCurSample;
        for (uint i = 0; (pNew != NULL) && (i < vQDFPops.size()); ++i) {
            printf("Getting samples from [%s]\n", vQDFPops[i].c_str());fflush(stdout);
            iResult = readArrays(vQDFPops[i].c_str(), pPopName);
            if (iResult == 0) {
                pNew = getSamplesCore(vQDFPops[i].c_str(), pPopName, mLocData);
            } else {
                pNew = NULL;
            }
        }
    }
    
    return m_pCurSample;
}

//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(const char *pQDFTime, const char *pPopName, const char *pLocFile, locdata &mLocData) {
    locspec ls(pLocFile);
    return getSamples(pQDFTime, pPopName, &ls, mLocData);
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(stringvec &vQDFPops, const char *pPopName, const char *pLocFile, locdata &mLocData) {
    locspec ls(pLocFile);
    return getSamples(vQDFPops, pPopName, &ls, mLocData);
}


    

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributesCore(const char *pQDFTime, const char *pPopName, 
                                    locdata  &mLocData, idset &sSelected) {
    int iResult = 0;

    /*
    iResult = readArrays(pQDFTime, pPopName);

    if (bSingle) {
        m_pCurSample = new IDSample();
    }

    stringvec vNames;
    if (iResult == 0) {
        iResult = fillLocData(pLocSpec, vNames, mLocData);
    }
    */
    if (iResult == 0) {
        // transform CellIDs to coords
        // we need the indexes to find gender etc
        for (int i = 0; (i < m_iNumAgents) && (iResult == 0); i++) {
            idset_cit it0 = sSelected.find(m_pIDs[i]);
            if (it0 != sSelected.end()) {
                // the coordinates are in degrees! make them radians
                double dLon0 = m_mCoords[m_pCellIDs[i]].first*M_PI/180;
                double dLat0 = m_mCoords[m_pCellIDs[i]].second*M_PI/180;
                bool bSearching = true;
                locdata::const_iterator it;
                for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
                    double d = spherdist(dLon0, dLat0, it->second.dLon*M_PI/180, it->second.dLat*M_PI/180, RADIUS);
                    if (d < it->second.dDist) {

                        agdata *pAD = new agdata;
                        pAD->iID     = m_pIDs[i];
                        pAD->iMomID  = -1;
                        pAD->iDadID  = -1;
                        pAD->iGender = m_pGenders[i];
                        pAD->iCellID = m_pCellIDs[i];
                        pAD->dLon = dLon0;
                        pAD->dLat = dLat0;
                        pAD->iIndex = i;
                        m_pCurSample->addAgentData(it->first.c_str(), m_fTimeStamp, pAD);
                        
                        bSearching = false;

                    }
                }
                if (bSearching) {
                    printf("%selement %ld not in range of any location - maybe different location list was used\n", RED, m_pIDs[i]);
                    iResult = -1;
                }
            }
        }
        
        if (iResult == 0) {
            //            idset sSelected;
            m_pCurSample->getFullIDSet(sSelected);
            for (uint i = 0; i < sSelected.size(); i++) {
                rand();
            }
        }
    } else {
        printf("%sCouldn't fill loc data\n", RED);
    }
    
    if (iResult != 0) {
        delete m_pCurSample;
        m_pCurSample = NULL;
    }

    return m_pCurSample;
}

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(const char *pQDFTime, const char *pPopName, const locspec *pLocSpec, 
                                    locdata  &mLocData, idset &sSelected) {
    
    IDSample *pNew = NULL;
    int iResult = 0;


    m_pCurSample = new IDSample();
    

    stringvec vNames;
    iResult = fillLocData(pLocSpec, vNames, mLocData);
    
    
    if (iResult == 0) {
        pNew = getAttributesCore(pQDFTime, pPopName, mLocData, sSelected);
    }
    return pNew;
}

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(stringvec &vQDFPops, const char *pPopName, const locspec *pLocSpec, locdata  &mLocData, idset &sSelected) {
    m_pCurSample = new IDSample();

    IDSample *pNew = m_pCurSample;

    for (uint i = 0; (pNew != NULL) && (i < vQDFPops.size()); ++i) {
        int iResult = readArrays(vQDFPops[i].c_str(), pPopName);
        if (iResult == 0) {
            pNew = getAttributesCore(vQDFPops[i].c_str(), pPopName, mLocData, sSelected);
        } else {
            pNew = NULL;
        }
    }    

    return m_pCurSample;
}

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(const char *pQDFTime, const char *pPopName, const char *pLocFile, locdata  &mLocData, idset &sSelected) {
    locspec ls(pLocFile);
    return getAttributes(pQDFTime, pPopName, &ls, mLocData, sSelected);
}

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(stringvec &vpQDFPops, const char *pPopName, const char *pLocFile, locdata  &mLocData, idset &sSelected) {
    locspec ls(pLocFile);
    return getAttributes(vpQDFPops, pPopName, &ls, mLocData, sSelected);
}


//----------------------------------------------------------------------------
// getFullIDSet
//
void IDSampler2::getFullIDSet(idset &sSelected) {
    m_pCurSample->getFullIDSet(sSelected);
}


//----------------------------------------------------------------------------
// getFullIndexIDMap
//
void IDSampler2::getFullIndexIDMap(indexids &mSelected) {
    m_pCurSample->getFullIndexIDMap(mSelected);
}


//----------------------------------------------------------------------------
// getSelectedIDSet
//
void IDSampler2::getLocationIDSet(locids &msSelected) {
    m_pCurSample->getLocationIDSet(msSelected);
}


//----------------------------------------------------------------------------
// readArrays
//
int IDSampler2::readArrays(const char *pQDFTime, const char *pPopName) {
    int iResult = -1;

    deleteArrays();

    m_pIDs        = NULL;
    m_pCellIDs    = NULL;
    m_pGenders    = NULL;

    m_iNumAgents  = 0;
    m_fTimeStamp  = -1;

    QDFArray *pQA = QDFArray::create(pQDFTime);
    if (pQA != NULL) {
        m_fTimeStamp = pQA->getTimeStep();
        iResult = pQA->openArray(POPGROUP_NAME, pPopName, AGENT_DATASET_NAME);
        if (iResult == 0) {
            m_iNumAgents = pQA->getSize();

            m_pIDs      = new idtype[m_iNumAgents];
            m_pCellIDs  = new int[m_iNumAgents];
            m_pGenders  = new int[m_iNumAgents];
            
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pIDs, m_iNumAgents, "AgentID");
                if (iCount != m_iNumAgents) {
                    printf("%sGot %d agent IDs instead of %d\n", RED, iCount, m_iNumAgents);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumAgents, "CellID");
                if (iCount != m_iNumAgents) {
                    printf("%sGot %d cell IDs instead of %d\n", RED, iCount, m_iNumAgents);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pGenders, m_iNumAgents, "Gender");
                if (iCount != m_iNumAgents) {
                    printf("%sGot %d genders instead of %d\n", RED, iCount, m_iNumAgents);
                    iResult = -1;
                }
            }
                
            if (iResult == 0) {
                printf("  read agent data: %d items\n", m_iNumAgents);
            }
        } else {
            printf("%sCouldn't open dataset [%s/%s%s]\n", RED, POPGROUP_NAME, pPopName, AGENT_DATASET_NAME);
        }
        pQA->closeArray();

        delete pQA;
    } else {
        iResult = -1;
        printf("%sCouldn't open file [%s]\n", RED, pQDFTime);
    }
    return iResult;
}



                     

//----------------------------------------------------------------------------
// fillCoordMap
//
int IDSampler2::fillCoordMap(const char *pQDFGeoGrid) {
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
                printf("%sRead bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", RED, pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            printf("%sCouldn't open QDF array for [%s:%s/%s]\n", RED, pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME);
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
                        printf("%sRead bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", RED, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells);
                    }
                } else {
                    iResult = -1;
                    printf("%sNumber of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", RED, iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE);
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
                        printf("%sCouldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", RED, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells);
                    }
                } else {
                    iResult = -1;
                    printf("%sNumber of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", RED, iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        printf("%sCouldn't create QDFArray\n", RED);
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
int IDSampler2::fillLocData(const locspec *pLocSpec, stringvec &vNames, locdata &mLocData) {
    int iResult = -1;
    
    LineReader *pLR = LineReader_std::createInstance(pLocSpec->pLocFile, "rt");
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
            if (pLocSpec->dDistance > 0) {
                li.dDist = pLocSpec->dDistance;
            }
            if (pLocSpec->iNum > 0) {
                li.iNum = pLocSpec->iNum;
            }

            if (iC == iReq) {
                // valid data: save it
                vNames.push_back(sName);
                mLocData[sName] = li;
            } else {
                printf("%sCouldn't read enough items from line [%s]\n", RED, pLine);
                iResult = -1;
            }
            pLine = pLR->getNextLine();
        }

        delete pLR;
    } else {
        printf("%sCouldn't open file [%s]\n", RED, pLocSpec->pLocFile);
    }

    return iResult;
}
