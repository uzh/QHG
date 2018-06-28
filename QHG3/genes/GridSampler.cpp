#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <map>
#include <vector>
#include <algorithm>

#include "types.h"
#include "utils.h"
#include "geomutils.h"
#include "strutils.h"
#include "GeneUtils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArray.cpp"

#include "GridSampler.h"


//----------------------------------------------------------------------------
// constructor
//
GridSampler::GridSampler() 
    : m_dDLon(0),
      m_dDLat(0),
      m_dR(0),
      m_iNumCells(0),
      m_iNumAgents(0),
      m_iGenomeSize(0),
      m_pQDFGeo(NULL), 
      m_pQDFStat(NULL), 
      m_pQDFPop(NULL), 
      m_pSpecies(NULL), 
      m_pdLongs(NULL),
      m_pdLats(NULL),
      m_pdAlts(NULL),
      m_pAgentIDs(NULL),
      m_pCellIDs(NULL),
      m_pdDists(NULL),
      m_ppGenomes(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
GridSampler::~GridSampler() {

    deleteArrays();

    if (m_pQDFGeo != NULL) {
        delete[] m_pQDFGeo;
    }
    if (m_pQDFStat != NULL) {
        delete[] m_pQDFStat;
    }
    if (m_pQDFPop != NULL) {
        delete[] m_pQDFPop;
    }
    if (m_pSpecies != NULL) {
        delete[] m_pSpecies;
    }
}


//----------------------------------------------------------------------------
// createInstance
//
GridSampler *GridSampler::createInstance(const char *pQDFGeo, const char *pQDFStat, const char *pQDFPop, const char *pSpecies) {
    GridSampler *pGS = new GridSampler();
    int iResult = pGS->init(pQDFGeo, pQDFStat, pQDFPop, pSpecies);
    if (iResult != 0) {
        delete pGS;
        pGS = NULL;
    }
    return pGS;
}


//----------------------------------------------------------------------------
// deleteArrays
//
void GridSampler::deleteArrays() {
    if (m_pdLongs != NULL) {
        delete[] m_pdLongs;
        m_pdLongs = NULL;
    }
    if (m_pdLats != NULL) {
        delete[] m_pdLats;
        m_pdLats = NULL;
    }
    if (m_pdAlts != NULL) {
        delete[] m_pdAlts;
        m_pdLats = NULL;
    }
    if (m_pAgentIDs != NULL) {
        delete[] m_pAgentIDs;
        m_pAgentIDs = NULL;
    }
    if (m_pCellIDs != NULL) {
        delete[] m_pCellIDs;
        m_pCellIDs = NULL;
    }
    if (m_pdDists != NULL) {
        delete[] m_pdDists;
        m_pdDists = NULL;
    }
    if (m_ppGenomes != NULL) {
        for (int i =0; i < m_iNumAgents; i++) {
            delete[] m_ppGenomes[i];
        }
        delete[] m_ppGenomes;
        m_ppGenomes = NULL;
    }

}


//----------------------------------------------------------------------------
// init
//   
int GridSampler::init(const char *pQDFGeo, const char *pQDFStat, const char *pQDFPop, const char *pSpecies) {
    int iResult = 0;
    m_pQDFGeo = new char[strlen(pQDFGeo)+1];
    strcpy(m_pQDFGeo, pQDFGeo);
    m_pQDFStat = new char[strlen(pQDFStat)+1];
    strcpy(m_pQDFStat, pQDFStat);
    m_pQDFPop = new char[strlen(pQDFPop)+1];
    strcpy(m_pQDFPop, pQDFPop);
    m_pSpecies = new char[strlen(pSpecies)+1];
    strcpy(m_pSpecies, pSpecies);

    iResult = loadArrays();

    return iResult;
}


//----------------------------------------------------------------------------
// loadGeoArray
//   
double *GridSampler::loadGeoArray(const char *pDataSet) {
    int iResult = -1;
    double *pArray = NULL;
 
    QDFArray *pQA = QDFArray::create(m_pQDFGeo);
    if (pQA != NULL) {
        iResult = pQA->openArray(GEOGROUP_NAME, pDataSet);
        if (iResult == 0) {
            m_iNumCells = pQA->getSize();
            pArray = new double[m_iNumCells];
            int iCount = pQA->getFirstSlab(pArray, m_iNumCells);
            if (iCount == m_iNumCells) {
                iResult = 0;
            } else {
                iResult = 1;
            }
        }
        pQA->closeArray();
        delete pQA;

    } else {
        printf("Couldn't open [%s] as QDF\n", m_pQDFGeo);
    }

    return pArray;
}


//----------------------------------------------------------------------------
// loadGeoArrays
//   
int GridSampler::loadGeoArrays() {
    int iResult = -1;

    m_pdLongs = loadGeoArray(GEO_DS_LONGITUDE);
    printf("Loaded %d longs\n", m_iNumCells);

    if (m_pdLongs != NULL) {
        m_pdLats  = loadGeoArray(GEO_DS_LATITUDE);
        printf("Loaded %d lats\n", m_iNumCells);
    }

    if (m_pdLats != NULL) {
        m_pdAlts  = loadGeoArray(GEO_DS_ALTITUDE);
        printf("Loaded %d alts\n", m_iNumCells);
    }

    if (m_pdAlts != NULL) {
        iResult = 0;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// loadPopArrays
//   
int GridSampler::loadPopArrays() {
    int iResult = -1;
    
    QDFArray *pQA = QDFArray::create(m_pQDFPop);
    if (pQA != NULL) { 
        iResult = pQA->openArray(POPGROUP_NAME, m_pSpecies, AGENT_DATASET_NAME);
        if (iResult == 0) {
            m_iNumAgents = pQA->getSize();

            m_pAgentIDs      = new idtype[m_iNumAgents];
            m_pCellIDs       = new int[m_iNumAgents];

            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pAgentIDs, m_iNumAgents, "AgentID");
                if (iCount != m_iNumAgents) {
                    printf("Got %d agent IDs instead of %d\n", iCount, m_iNumAgents);
                    iResult = -1;
                } else {
                    printf("Loaded %d Agent IDs\n", m_iNumAgents);
                }
            }

            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumAgents, "CellID");
                if (iCount != m_iNumAgents) {
                    printf("Got %d agent IDs instead of %d\n", iCount, m_iNumAgents);
                    iResult = -1;
                } else {
                    printf("Loaded %d Cell IDs\n", m_iNumAgents);
                }
            }

        }

        pQA->closeArray();
        delete pQA;
    } else {
        printf("Couldn't open [%s] as QDF\n", m_pQDFPop);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// loadStatArrays
//   
int GridSampler::loadStatArrays() {
    int iResult = -1;
    
    QDFArray *pQA = QDFArray::create(m_pQDFStat);
    if (pQA != NULL) { 
        iResult = pQA->openArray(MSTATGROUP_NAME, MSTAT_DS_DIST);
        if (iResult == 0) {

            int iNumCells = pQA->getSize();
            if (iNumCells == m_iNumCells) {
                m_pdDists = new double[m_iNumCells];

                if (iResult == 0) {
                    int iCount = pQA->getFirstSlab(m_pdDists, m_iNumCells);
                    if (iCount != m_iNumCells) {
                        printf("Got %d distances instead of %d\n", iCount, m_iNumCells);
                        iResult = -1;
                    } else {
                        printf("Loaded %d travel dists\n", m_iNumCells);
                    }
                }
            } else {
                printf("Number of cells ins Stat file does not match Geo file\n");
            }
        }

        pQA->closeArray();
        delete pQA;
    } else {
        printf("Couldn't open [%s] as QDF\n", m_pQDFStat);
    }
    return iResult;
}



//----------------------------------------------------------------------------
// loadArrays
//   
int GridSampler::loadArrays() {
    int iResult = -1;

    iResult = loadGeoArrays();
    if (iResult == 0) {
        iResult = loadPopArrays();
    }
    if (iResult == 0) {
        iResult = loadStatArrays();
    }

    if (iResult != 0) {
        deleteArrays();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// loadGenomes
//   
int GridSampler::loadGenomes() {
   int iResult = 0;
    
   QDFArray *pQA = QDFArray::create(m_pQDFPop);
   if (pQA != NULL) { 
       iResult = pQA->openArray(POPGROUP_NAME, m_pSpecies, GENOME_DATASET_NAME);
       if (iResult == 0) {
           int iNumBlocks = pQA->getSize()/(2*m_iNumAgents);
           m_iGenomeSize = GeneUtils::NUCSINBLOCK*iNumBlocks;
           
           printf("loading %d genome of size %d\n", m_iNumAgents, m_iGenomeSize);
           m_ppGenomes = new ulong*[m_iNumAgents];
           for (int i = 0; i < m_iNumAgents; i++) {
               m_ppGenomes[i] = new ulong[2*iNumBlocks];
               
               int iCount = 0;
               if (i == 0) {
                   iCount = pQA->getFirstSlab(m_ppGenomes[i], 2*iNumBlocks);
               } else {
                   iCount = pQA->getNextSlab(m_ppGenomes[i], 2*iNumBlocks);
               }
               if (iCount != 2*iNumBlocks) {
                   printf("Got %d agent IDs instead of %d\n", iCount, 2*iNumBlocks);
                   iResult = -1;
               } else {
                   //                    printf("Loaded %d Agent IDs\n", m_iNumAgents);
               }
           }
           
       } else {
           iResult = -1;
           printf("Couldn't open genome array [%s]\n", m_pQDFPop);
       }
       pQA->closeArray();
       delete pQA;
   } else {
       printf("Couldn't open [%s] as QDF\n", m_pQDFPop);
   }
   
   return iResult;
}


//----------------------------------------------------------------------------
// findCandidates
//   
int GridSampler::findCandidates(double dDLon, double dDLat, double dR) {
    int iResult = 0;
    m_dDLon = dDLon;
    m_dDLat = dDLat;
    m_dR    = dR;
    int iC = 0;
    m_mGridAgents.clear();
    for (int iAgentIndex = 0; iAgentIndex < m_iNumAgents; iAgentIndex++) {
        int iCell = m_pCellIDs[iAgentIndex];
        double dLon = m_pdLongs[iCell];
        double dLat = m_pdLats[iCell];
        double dGLon = dNaN;
        double dGLat = dNaN;
        findClosestGrid(dLon, dLat, &dGLon, &dGLat);
        if (spherdist(dLon, dLat, dGLon, dGLat, RADIUS_EARTH_KM) < m_dR) {
            m_mGridAgents[coords(dGLon, dGLat)].push_back(iAgentIndex);
            iC++;
        }

    }
    printf("Total: %d agents collected at %zd locations\n", iC, m_mGridAgents.size());
    return iResult;
}


//----------------------------------------------------------------------------
// findClosestGrid
//   
void GridSampler::findClosestGrid(double dLon, double dLat, double *pdGLon, double *pdGLat) {
    double dBaseLon = (int)dLon/m_dDLon;
    double dFracLon = fmod(dLon, m_dDLon);
    int iLonTweak = 0;
    if (dFracLon > m_dDLon/2) {
        iLonTweak = 1;
    }
    *pdGLon = (dBaseLon+iLonTweak)*m_dDLon;

    double dBaseLat = (int)dLat/m_dDLat;
    double dFracLat = fmod(dLat, m_dDLat);
    int iLatTweak = 0;
    if (dFracLat > m_dDLat/2) {
        iLatTweak = 1;
    }
    *pdGLat = (dBaseLat+iLatTweak)*m_dDLat;
}


//----------------------------------------------------------------------------
// selectAtGrids
//   
int GridSampler::selectAtGrids(uint iNSel) {
    int iResult = 0;
    int iC = 0;
    m_mvSelected.clear();
    mgridags::const_iterator it;
    for (it = m_mGridAgents.begin(); it != m_mGridAgents.end(); ++it) {
        vagidx v = it->second;
        vagidx &vOut = m_mvSelected[it->first];
        if (v.size() < iNSel) {
            vOut.insert(vOut.begin(), v.begin(), v.end());
        } else {
            // Fisher-Yates shuffle
            for (int i =v.size()-1; i > 1; i--) {
                int r = (int)((1.0*i*rand())/RAND_MAX);
                idtype t = v[i];
                v[i] = v[r];
                v[r] = t;
            }
            vOut.insert(vOut.end(), v.begin(), v.begin()+iNSel);
        }
        iC += vOut.size();
    }
    printf("Selected %d agents\n", iC);
    return iResult;
}


//----------------------------------------------------------------------------
// calcGeoGenomeDists
//   
int GridSampler::calcGeoGenomeDists(ulong *pRefGenome) {
    int iResult = 0;

    m_vDistPairs.clear();
    mgridags::const_iterator it;
    for (it = m_mvSelected.begin(); it != m_mvSelected.end(); ++it) {
        int iCell0 = -1;
        vagidx v = it->second;
        double dTDist = 0;
        double dGDist = 0;
        for (uint i = 0; i < v.size(); i++) {
            idtype iIdx = v[i];
            int iCell = m_pCellIDs[iIdx];
            // travelled distance
            dTDist += m_pdDists[iCell];
            // genetic distance
            dGDist += GeneUtils::calcDist(pRefGenome, m_ppGenomes[iIdx], m_iGenomeSize);

            if (iCell0 < 0) {
                iCell0 = iCell;
            }
        }
        // average it
        dTDist /= v.size();
        dGDist /= v.size();

        m_vDistPairs.push_back(distpair(dTDist, dGDist));

        m_vCoords.push_back(coords(m_pdLongs[iCell0], m_pdLats[iCell0]));
    }
    printf("Calculated %zd averaged distances\n", m_vDistPairs.size());
    return iResult;
}

//----------------------------------------------------------------------------
// writeFile
//   
int GridSampler::writeFile(const char *pOutput) {
    int iResult = 0;

    FILE *fOut = fopen(pOutput, "wt");
    if (fOut != NULL) {
        distpairs::const_iterator it;
        for (it = m_vDistPairs.begin(); it != m_vDistPairs.end(); ++it) {
            fprintf(fOut, "%8.3f %8.3f\n", it->first, it->second);
        }
        fclose(fOut);
    } else {
        iResult = -1;
        printf("Couldn't open [%s]\n", pOutput);
    }

    FILE *fOut2 = fopen("cococo", "wt");
    if (fOut2 != NULL) {
        std::vector<coords>::const_iterator it;
        for (it = m_vCoords.begin(); it != m_vCoords.end(); ++it) {
            fprintf(fOut2, "%8.3f %8.3f\n", it->first, it->second);
        }
        fclose(fOut2);
    } else {
        iResult = -1;
        printf("Couldn't open [%s]\n", pOutput);
    }

    return iResult;
}
