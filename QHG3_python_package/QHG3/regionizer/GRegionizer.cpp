#include <stdio.h>
#include <string.h>
#include <vector>

#include <hdf5.h>
#include <omp.h>

#include "types.h"
#include "strutils.h"
#include "SCellGrid.h"
#include "GridReader.h"
#include "GeoReader.h"
#include "ClimateReader.h"
#include "GridWriter.h"
#include "PopReader.h"
#include "PopBase.h"
#include "SnapHeader.h"
#include "IDGen.h"
#include "WELL512.h"
#include "PopulationFactory.h"

#include "GRegion.h"
#include "GRegionizer.h"

// use random numbers from a table for default state
static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 
    0x1bf121c5, 
    0xb222a768, 
    0x6c5d388b, 
    0xab99166e, 
    0x326c9f12, 
    0x3354197a, 
    0x7036b9a5, 
    0xb08c9e58, 
    0x3362d8d3, 
    0x037e5e95, 
    0x47a1ff2f, 
    0x740ebb34, 
    0xbf27ef0d, 
    0x70055204, 
    0xd24daa9a,
};


//----------------------------------------------------------------------------
// constructor
//
GRegionizer::GRegionizer() 
    : m_bDeleteObjects(false),
      m_pCG(NULL),
      m_piAgentsPerCell(NULL),
      m_piMarks(NULL),
      m_bTimeSeed(true),
      m_dNeighborFinding(0) {

    m_vGRegions.clear();
}

//----------------------------------------------------------------------------
// destructor
//
GRegionizer::~GRegionizer() {
    if (m_bDeleteObjects) {
        delete m_pCG;
        delete[] m_piAgentsPerCell;
        delete[] m_piMarks;
    }

    for (uint i = 0; i < m_vGRegions.size(); i++) {
        delete m_vGRegions[i];
    }
}

//----------------------------------------------------------------------------
// init
//
int GRegionizer::init(SCellGrid *pCG, int *piAgentsPerCell, int *piMarks) {
    m_pCG             = pCG;
    m_piAgentsPerCell = piAgentsPerCell;
    m_piMarks         = piMarks;
    return 0;
}

//----------------------------------------------------------------------------
// init
//
int GRegionizer::init(const char *pQDFGrid, const char *pQDFPop, const char *pSpecies) {
    int iResult =  -1;
    m_bDeleteObjects = true;
    hid_t hGrid = qdf_openFile(pQDFGrid);
    if (hGrid != H5P_DEFAULT) {
        hid_t hPop = qdf_openFile(pQDFPop);
        if (hPop != H5P_DEFAULT) {
            iResult = setGrid(hGrid);
            if (iResult == 0) {
                iResult = setPop(hPop, pSpecies);
            }
            qdf_closeFile(hPop);
        } else {
            printf("Couldn't open pop qdf file [%s]\n", pQDFPop);
        }
        qdf_closeFile(hGrid);
    } else {
        printf("Couldn't open grid qdf file [%s]\n", pQDFGrid);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// createInstance
//
GRegionizer *GRegionizer::createInstance(const char *pQDFGrid, const char *pQDFPop, const char *pSpecies) {
    GRegionizer *pGR = new GRegionizer();
    int iResult = pGR->init(pQDFGrid, pQDFPop, pSpecies);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createInstance
//
GRegionizer *GRegionizer::createInstance(SCellGrid *pCG, int *piAgentsPerCell, int *piMarks) {
    GRegionizer *pGR = new GRegionizer();
    int iResult = pGR->init(pCG, piAgentsPerCell, piMarks);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// setGrid
//
int GRegionizer::setGrid(hid_t hGrid) {
    int iResult = -1;

    GridReader *pGR = GridReader::createGridReader(hGrid);
    if (pGR != NULL) {
        int iNumCells = 0;
        
        stringmap smSurfData;
        iResult = pGR->readAttributes(&iNumCells, smSurfData);
        if (iResult == 0) {
            m_pCG = new SCellGrid(0, iNumCells, smSurfData);
            m_pCG->m_aCells = new SCell[iNumCells];
            iResult = pGR->readCellData(m_pCG);
            if (iResult == 0) {
                m_piMarks = new int[m_pCG->m_iNumCells];
                for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                    m_piMarks[i] = -1;
                }
                
                m_piAgentsPerCell = new int[m_pCG->m_iNumCells];
                memset(m_piAgentsPerCell, 0, m_pCG->m_iNumCells*sizeof(int));
                
                GeoReader *pGeoR = GeoReader::createGeoReader(hGrid);
                if (pGeoR != NULL) {
                    uint iNumCells2;
                    int iMaxNeighbors;
                    double dRadius;
                    double dSeaLevel;
                    iResult = pGeoR->readAttributes(&iNumCells2, &iMaxNeighbors, &dRadius, &dSeaLevel);
                    if ((iResult == 0) && (iNumCells == (int)iNumCells2)) {
                        m_pCG->m_pGeography = new Geography(iNumCells, iMaxNeighbors, dRadius, dSeaLevel);
                        iResult = pGeoR->readData(m_pCG->m_pGeography);
                        if (iResult == 0) {
                            printf("[setGrid] Geography data read\n");
                        } else {
                            printf("[setGrid] GeoReader couldn't read data\n");
                        }
                    } else {
                        printf("[setGrid] GeoReader couldn't read attributes\n");
                    }
                    delete pGeoR;
                } else {
                    printf("[setGrid] No geo found in file\n");
                }

                ClimateReader *pCliR = ClimateReader::createClimateReader(hGrid);
                if (pCliR != NULL) {
                    uint iNumCells3;
                    int iNumSeason;
                    bool bDynamic;
                    iResult = pCliR->readAttributes(&iNumCells3, &iNumSeason, &bDynamic);
                    if ((iResult == 0) && (iNumCells == (int)iNumCells3)) {
                        m_pCG->m_pClimate = new Climate(iNumCells, iNumSeason, m_pCG->m_pGeography);
                        iResult = pCliR->readData(m_pCG->m_pClimate);
                        if (iResult == 0) {
                            printf("[setGrid] Climate data read\n");
                        } else {
                            printf("[setGrid] CliReader couldn't read data\n");
                        }
                        
                    } else {
                        printf("[setGrid] CliReader couldn't read attributes\n");
                    }
                    delete pGeoR;
                } else {
                    printf("[setGrid] No geo found in file\n");
                }

                printf("[setGrid] Grid read successfully: %p\n", m_pCG);
            } else {
                printf("[setGrid] GridReader couldn't read data\n");
            }
        } else {
            printf("[setGrid] GridReader couldn't read attributes\n");
        }
        delete pGR;
    } else {
        printf("[setGrid] Couldn't create GridReader\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setPops
//  use QDF file
//  if pSpecies is NULL, read all populations in the QDF file
//  otherwise read only specified species
//
int GRegionizer::setPop(hid_t hPop, const char *pSpecies) {
    int iResult = -1;
    int iNumThreads = 1;

    // prepare data needed to create populations
    IDGen **apIDG = new IDGen*[iNumThreads];
    memset(apIDG, 0, iNumThreads*sizeof(IDGen*));

    uint32_t aulState[STATE_SIZE];
    memcpy(aulState, s_aulDefaultState, STATE_SIZE*sizeof(uint32_t));
    int iLayerSize = 8192;

    // get apopulation factory
    PopulationFactory *pPopFac = new PopulationFactory(m_pCG, iLayerSize, apIDG, aulState);

    // read population data from file
    PopReader *pPR = PopReader::create(hPop);
    if (pPR != NULL) {
        const popinfolist &pil = pPR->getPopList();
        if (pil.size() > 0) {
            for (uint i = 0; i < pil.size(); i++) {

                // if no species is specified, use all populations
                if ((pSpecies == NULL) || (strcmp(pil[i].m_sSpeciesName, pSpecies) == 0)) {
                    PopBase *pPop = pPopFac->createPopulation(pil[i].m_sClassName);
                    if (pPop != NULL) {

                        // read agent data
                        iResult = pPR->read(pPop, pil[i].m_sSpeciesName, m_pCG->m_iNumCells);
                        if (iResult == 0) {
                            // get agent counts and move them 
                            for (uint k = 0; k < m_pCG->m_iNumCells; k++) {
                                m_piAgentsPerCell[k] += pPop->getNumAgents(k);
                            }
                        }
                    } else {
                        if (iResult == POP_READER_ERR_CELL_MISMATCH) {
                            printf("[setPops] Cell number mismatch: CG(%d), pop[%s](%d)\n",  
                                   m_pCG->m_iNumCells, pil[i].m_sSpeciesName, pPop->getNumCells());
                        } else if (iResult == POP_READER_ERR_READ_SPECIES_DATA) {
                            printf("[setPops] Couldn't read species data for [%s]\n",  pil[i].m_sSpeciesName);
                        } else if (iResult == POP_READER_ERR_NO_SPECIES_GROUP) { 
                            printf("[setPops] No group for [%s] found in QDF file\n",  pil[i].m_sSpeciesName);
                        }
                    }
                    delete pPop;
                } else {
                    printf("[setPops] Couldn't create Population %s(%d) %s(%d)\n", 
                           pil[i].m_sSpeciesName, pil[i].m_iSpeciesID, pil[i].m_sClassName, pil[i].m_iClassID);
                }
            }
        } else {
            printf("[setPops] poplist size %zd\n", pil.size());
            iResult = -1;
        }

        delete pPR;
    } else {
        printf("[setPops] Couldn't create PopReader:(\n");
    }

    // clean up 
    for (int i = 0; i < iNumThreads; i++) {
        delete apIDG[i];
    }
    delete[] apIDG;
    delete pPopFac;

    return iResult;
}

//----------------------------------------------------------------------------
// initializeRegions
//  piInitial should contain iNumRegions*iNumInitial *differen* ints,
//  or be NULL
//
int GRegionizer::initializeRegions(int iNumRegions, int *piInitial, int iNumInitial) {
    int iResult = 0;

    if (m_bTimeSeed) {
        time_t t = time(NULL);
        srand(t);
    }

    // create iNumRegions regions
    for (int i = 0; (iResult == 0) && (i < iNumRegions); i++) {
        GRegion *pGR = new GRegion(i, m_pCG, m_piAgentsPerCell, m_piMarks);

        if (piInitial != NULL) {
            // add iNumInitial indexes to region
            for (int j = 0; j < iNumInitial; j++) {
                pGR->addCell(piInitial[i*iNumInitial+j]);
            }
            
        } else {
            // if no seeds are given, add iNumInitial randsom indexes to region
            for (int j = 0; (iResult == 0) && (j < iNumInitial); j++) {
                bool bSearching = true;
                for (int t = 0; bSearching && (t < 50); t++) {
                    int z = (int)((1.0*m_pCG->m_iNumCells*rand())/RAND_MAX);
                    // make sure it has not been used yet
                    if (m_piMarks[z] < 0) {
                        pGR->addCell(z);
                        bSearching = false;
                    } 
                }
                if (bSearching) {
                    iResult = -1;
                }
            }
        }
        pGR->findNeighbors();
        m_vGRegions.push_back(pGR);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// growStep
//   
int GRegionizer::growStep(int iMode) {
    int iResult = 0;
    
    // find regions with smallest number of agents
    int iMin = INT_MAX;
    int iMinIdx = -1;
    for (uint i = 0; i < m_vGRegions.size(); i++) {
        if ((m_vGRegions[i]->getTotal() < iMin) && (m_vGRegions[i]->getNumNeighbors()>0)) {
            iMin = m_vGRegions[i]->getTotal();
        }
    }
    std::vector<int> vMins;
    for (uint i = 0; i < m_vGRegions.size(); i++) {
        if ((m_vGRegions[i]->getTotal() == iMin) && (m_vGRegions[i]->getNumNeighbors()>0)){
            vMins.push_back(i);
        }
    }

    if (vMins.size() > 0) {
        int z = (int)((1.0*vMins.size()*rand())/RAND_MAX);
        iMinIdx = vMins[z];
        
        //        printf("Region #%d can choose\n", iMinIdx);
        
        GRegion *pGR = m_vGRegions[iMinIdx];
        double d1;
        double d2;
        switch (iMode) {
        case MODE_ALL:
            pGR->addAllNeighbors();
            // to update neighbor vectors:
            // remove ids in pGRs neighbors from all other neighbor vectors
            d1 = omp_get_wtime();
            // modify othe neighbor sets by removing this neighbor set
            for (uint i = 0; i < m_vGRegions.size(); i++) {
                if ((int)i != iMinIdx) {
                    m_vGRegions[i]->removeFromNeighbors(pGR->getNeighbors());
                }
            }
            pGR->findNeighbors();
            d2 = omp_get_wtime();
            m_dNeighborFinding += d2-d1;
            
            break;
            
        case MODE_RAND:
            pGR->addRandomNeighbor();
            break;
            
        case MODE_SMALLEST:
            pGR->addSmallestNeighbor();
            break;
            
        case MODE_LARGEST:
            pGR->addLargestNeighbor();
            break;
        default:
            iResult = -1;
        }

        // naive solution: recalculate all neighbors
        /* you are too slow
        d1 = omp_get_wtime();
        for (uint i = 0; i < m_vGRegions.size(); i++) {
            m_vGRegions[i]->findNeighbors();
        }
        d2 = omp_get_wtime();
        m_dNeighborFinding += d2-d1;
        */

    } else {
        iResult = 2;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeRegions
//   
int GRegionizer::writeRegions(const char *pOutputQDF) {
    int iResult = 0;

    GridWriter *pGW = new GridWriter(m_pCG, &m_pCG->m_smSurfaceData);
    hid_t hFile = qdf_createFile(pOutputQDF, 0);
    pGW->write(hFile);
    hid_t hGroup = qdf_createGroup(hFile, "RegionData");
    iResult = qdf_writeArray(hGroup, "Marks", m_pCG->m_iNumCells, m_piMarks);
    qdf_closeGroup(hGroup);
    qdf_closeFile(hFile);
    delete pGW;


    // now do a snapfile
    char *pBuffer = new char[m_pCG->m_iNumCells*(sizeof(int)+1*sizeof(double))];
    memset(pBuffer, 0, m_pCG->m_iNumCells*(sizeof(int)+1*sizeof(double)));
    char *pCur = pBuffer;
    for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
        int iID = m_pCG->m_aCells[i].m_iGlobalID;
        pCur = putMem(pCur, &iID, sizeof(int));
        double d = m_piMarks[i];
        pCur = putMem(pCur, &d, sizeof(double));
    }

    char sSnapName[256];
    strcpy(sSnapName, pOutputQDF);
    strcat(sSnapName, ".snap");
    SnapHeader *pSH = new SnapHeader("3.0", COORD_NODE, 0 , 0, "ld", "dummy.ico",false, 0, "Regions",0,NULL);
    FILE *fOut = fopen(sSnapName, "wb");
    pSH->write(fOut, true);
    uint iW = fwrite(pBuffer, sizeof(int)+sizeof(double), m_pCG->m_iNumCells, fOut);
    if (iW == m_pCG->m_iNumCells) {
        printf("Written data to [%s]\n", sSnapName);
    } else {
        printf("Error at writing Snap\n");
        //        iResult = -1;
    }
    delete pSH;
    fclose(fOut);
    delete[] pBuffer;
    

    return iResult;
}

//----------------------------------------------------------------------------
// showRegions
//   
void GRegionizer::showRegions() {
    uint iTotal = 0;
    float fSum  = 0;
    float fSum2 = 0;
    for (uint i = 0; i < m_vGRegions.size(); i++) {
        m_vGRegions[i]->showRegion();
        iTotal += m_vGRegions[i]->getNumCells();

        float fN = m_vGRegions[i]->getTotal();
        fSum  += fN;
        fSum2 += fN*fN;
    }
    float fAvg = fSum/m_vGRegions.size();
    fSum2  /= m_vGRegions.size();
    fSum2 = fSum2 - fAvg*fAvg;
    printf("Total %d agents in %zd regions; avg %f (var %f)\n", (int)fSum, m_vGRegions.size(), fAvg, sqrt(fSum2));
    
    printf("Total %d cells in regions\n", iTotal);
    

    uint ium = 0;
    uint im = 0;
    for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
        if (m_piMarks[i] < 0) {
            ium++;
        } else {
            im++;
        }
    }
    printf("Marked cells %d, unmarked cells %d, sum %d %s %d\n", im, ium, ium+im, (ium+im == m_pCG->m_iNumCells)?"=":"!=", m_pCG->m_iNumCells);
    if (im != iTotal) {
        printf("****Total %d != marked %d\n", iTotal, im);
    }
}
