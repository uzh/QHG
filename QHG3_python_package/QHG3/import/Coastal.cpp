#include <stdio.h>
#include <string.h>

#include <set>

#include <hdf5.h>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"

#include "QDFUtils.h"

#include "SCellGrid.h"
#include "Geography.h"

#include "GridReader.h"
#include "GridWriter.h"
#include "GeoReader.h"
#include "GeoWriter.h"

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - finding coastal cells and write to QDF\n", pApp);
    printf("Usage:\n");
    printf("  %s -i <input_qdf> -d <distance> -o <output_qdf> [-a]\n", pApp);
    printf("where\n");
    printf("  input_qdf   qdf file with Geography (Altitude) in which to search for coastal cells\n");
    printf("  distance    max distance to sea (in cell hops)\n");
    printf("  output_qdf  output file (coastals written to DS 'Water')\n");
    printf("  -c          copy water array of input_qdf to new water array\n"); 
    printf("\n");
}

//----------------------------------------------------------------------------
// createCellGrid
//
SCellGrid *createCellGrid(const char *pQDFFile) {
    SCellGrid *pCG = NULL;
    int iResult = -1;
    hid_t hFile = qdf_openFile(pQDFFile);
    if (hFile > 0) {
        printf("File opened\n");

        GridReader *pGR = GridReader::createGridReader(hFile);
        if (pGR != NULL) {
            int iNumCells=0;
            stringmap smSurfData;
            iResult = pGR->readAttributes(&iNumCells, smSurfData);
            if (iResult == 0) {
                printf("num cells: %d\n", iNumCells);
                pCG = new SCellGrid(0, iNumCells, smSurfData);
                pCG->m_aCells = new SCell[iNumCells];
                
                iResult = pGR->readCellData(pCG);
                if (iResult == 0) {
                    GeoReader *pGeoR = GeoReader::createGeoReader(hFile);
                    if (pGeoR != NULL) {
                        int iMaxNeighbors;
                        uint iNumCells2;
                        double dRadius;
                        double dSeaLevel;
                        iResult = pGeoR->readAttributes(&iNumCells2, &iMaxNeighbors, &dRadius, &dSeaLevel);
                        if (iResult == 0) {
                            Geography *pGeo = new Geography(iNumCells2, iMaxNeighbors, dRadius);
                            
                            iResult = pGeoR->readData(pGeo);
                            if (iResult == 0) {
                                pCG->setGeography(pGeo);
                            } else {
                                printf("Couldn't read geo data from [%s]\n", pQDFFile);
                            }
                        } else {
                            printf("Couldn't read geo attributes from [%s]\n", pQDFFile);
                        }
                        delete pGeoR;
                    } else {
                        printf("Couldn't create GeoReader for QDF file [%s]\n", pQDFFile);
                    }
                } else {
                    printf("Couldn't read geo attributes from [%s]\n", pQDFFile);
                }
            } else {
                printf("Couldn't get number of cells from [%s]\n", pQDFFile);
            }
            delete pGR;
        } else {
            printf("Couldn't create GridReader for QDF file [%s]\n", pQDFFile);
        }
    } else {
        printf("Couldn't open QDF file [%s]\n", pQDFFile);
    }
    
    if (iResult != 0) {
        delete pCG; 
        pCG = NULL;
    }
    return pCG;
}



//----------------------------------------------------------------------------
// collectNeighborhood
//
void collectNeighborhood(SCellGrid *pCG, int iIndex, int iDistance, intset &s) {
    for (int i = 0; i < iDistance; i++) {
        intset::const_iterator it;
        for (it = s.begin(); it != s.end(); ++it) {
            SCell &sCell = pCG->m_aCells[*it];
            for (int k = 0; k < sCell.m_iNumNeighbors; ++k) {
                s.insert(pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID);
            }
        }
    }
}


//----------------------------------------------------------------------------
// checkNeighbors
//
int checkNeighbors(SCellGrid *pCG, int iIndex, int iNewIndex, int iDistance) {
    int iFoundSea = 0;
    SCell &sCell = pCG->m_aCells[iNewIndex];
    if (iDistance > 0) {
        for (int k = 0; (iFoundSea == 0) && (k < 6/*sCell.m_iNumNeighbors*/); ++k) {
            int iNewIndex = pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
            iFoundSea = checkNeighbors(pCG, iIndex, iNewIndex, iDistance-1);
        }
    } else {
        if (pCG->m_pGeography->m_adAltitude[sCell.m_iGlobalID] < 0) {
            iFoundSea = 1;
        }
    }
    return iFoundSea;
}


//----------------------------------------------------------------------------
// findCoastal
//
int findCoastal(SCellGrid *pCG, int iDistance, double *pCoastal) {
    int iResult = 0;
    int iNumCells = pCG->m_iNumCells;
    Geography *pGeo = pCG->m_pGeography;
    
    printf("Finding coastal cells\n");
#pragma omp parallel for
    for (int iIndex =  0; iIndex < iNumCells; iIndex++) {
        // convoluted condition: we want altitude nan to be positive
        if (!(pGeo->m_adAltitude[iIndex] < 0)) {
           
            int iFoundSea = checkNeighbors(pCG, iIndex, iIndex, iDistance);
            if (iFoundSea) {
                pCoastal[iIndex] = 1;
            }
        }
    }
    

    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char*apArgV[]) {
    int iResult = -1;
    char *sInputQDF  = NULL;
    char *sOutputQDF = NULL;
    int   iDistance  = 0;
    bool  bCopy      =  false;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(4,
                                   "-i:S!",      &sInputQDF,
                                   "-d:i!",      &iDistance,
                                   "-o:S!",      &sOutputQDF,
                                   "-c:0",       &bCopy);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                
                
                printf("creating cell grid\n");
                SCellGrid *pCG = createCellGrid(sInputQDF);
                if (pCG != NULL) {
                    
                    int iNumCells = pCG->m_iNumCells;
                    
                    double *pCoastal =  pCG->m_pGeography->m_adWater;

                    if (!bCopy) {
                        memset(pCoastal, 0, iNumCells*sizeof(double));
                    }

                    iResult = findCoastal(pCG, iDistance, pCoastal);
                    if (iResult == 0) {

                        printf("coastals found; writing output\n"); fflush(stdout);
                        float fTime = 0;
                        hid_t hFile = qdf_createFile(sOutputQDF, fTime);
                        if (hFile >= 0) {
                            GridWriter *pGridW = new GridWriter(pCG, NULL);
                            pGridW->write(hFile);
                            GeoWriter *pGeoW = new GeoWriter(pCG->m_pGeography);
                            pGeoW->write(hFile);
                            qdf_closeFile(hFile);
                            iResult = 0;
                            printf("Written to QDF file [%s]\n", sOutputQDF);
                        } else {
                            printf("Couldn't open QDF file [%s]\n", sOutputQDF);
                            iResult = -1;
                        }

                        if (iResult == 0) {
                            printf("+++ success +++\n");
                        }
                    }
                    delete pCG;
                }
                    
            } else {
                usage(apArgV[0]);
            }
            
            
        } else {
            printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        printf("Couldn't create ParamReader\n");
    }
    

    return iResult;
}
