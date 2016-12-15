#include <stdio.h>
#include <string.h>

#include <hdf5.h>

#include "types.h"
#include "strutils.h"
#include "SnapHeader.h"
#include "QDFUtils.h"
#include "QDF2SnapBase.h"
#include "QDF2GeoSnap.h"
#include "QDF2ClimateSnap.h"
#include "QDF2VegSnap.h"
#include "QDF2PopSnap.h"
#include "QDF2MoveStatSnap.h"



int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    if (iArgC > 3) {
        // get snaptype
        char *pAttr = NULL;
      

        // open file & get handle
        hid_t hFile = qdf_openFile(apArgV[1]);
        if (hFile > 0) {

            printf("Opened [%s]\n", apArgV[1]);
            int iNumCells = -1;
            int iOutputNameIndex = 3;

            bool bGridExists = H5Lexists(hFile, GRIDGROUP_NAME, H5P_DEFAULT);
            if (bGridExists) {
                hid_t hGridGroup = qdf_openGroup(hFile, GRIDGROUP_NAME);
                printf("G:Opened group [%s] %d\n", GRIDGROUP_NAME, hGridGroup);
                iResult = qdf_extractAttribute(hGridGroup, GRID_ATTR_NUM_CELLS, 1, &iNumCells);
                printf("G:Have %d cells\n", iNumCells);
                qdf_closeGroup(hGridGroup);
            } else {
                hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
                pAttr = apArgV[3];
                iOutputNameIndex = 4;
            
                hid_t hSpecies = qdf_openGroup(hPopGroup, pAttr);
                printf("P:Opened group [%s]\n", pAttr);
                iResult = qdf_extractAttribute(hSpecies, SPOP_ATTR_NUM_CELL, 1, &iNumCells);
                printf("P:Have %d cells\n", iNumCells);
                qdf_closeGroup(hSpecies);
                qdf_closeGroup(hPopGroup);
            }
         


            if (iNumCells > 0) {
                char *pBuffer = new char[iNumCells*(sizeof(int)+1*sizeof(double))];
                memset(pBuffer, 0, iNumCells*(sizeof(int)+1*sizeof(double)));
                // get snaptype
                char *pAttr = NULL;
                char *pSnap = apArgV[2];
                
                
                if (strcasecmp(pSnap, GEOGROUP_NAME) == 0) {
                    iOutputNameIndex++;
                    if (iArgC > iOutputNameIndex) {
                        pAttr = apArgV[3];
                        char *pParam = NULL;
                        if (strcasecmp(pAttr, GEO_DS_DISTANCES) == 0) {
                            iOutputNameIndex++;
                            if (iArgC > iOutputNameIndex) {
                                pParam = apArgV[4];
                            } else {
                                printf("Snap for %s:Distances needs additional parameter (0, 1, 2, 3, 4, 5 or \"avg\")\n", GEOGROUP_NAME);
                                iResult = -1;
                            }
                        }
                        if (iResult == 0) {
                            QDF2GeoSnap *pQ2G = QDF2GeoSnap::createInstance(hFile, iNumCells, pAttr, pParam);
                            if (pQ2G != NULL) {
                                iResult = pQ2G->fillSnapData(pBuffer);
                                delete pQ2G;
                            } else {
                                iResult = -1;
                                printf("Couldn't create QDF2ClimateSnap\n");
                            }
                        }
                    } else {
                        printf("Snap for %s needs at least 1 parameter\n", CLIGROUP_NAME);
                        iResult = -1;
                    }
                } else if (strcasecmp(pSnap, CLIGROUP_NAME) == 0) {
                    iOutputNameIndex++;
                    if (iArgC > iOutputNameIndex) {
                        pAttr = apArgV[3];
                        char *pParam = NULL;
                        QDF2ClimateSnap *pQ2C = QDF2ClimateSnap::createInstance(hFile, iNumCells, pAttr, pParam);
                        if (pQ2C != NULL) {
                            iResult = pQ2C->fillSnapData(pBuffer);
                            delete pQ2C;
                            } else {
                            iResult = -1;
                                printf("Couldn't create QDF2ClimateSnap\n");
                        }
                    }
                    
                } else if (strcasecmp(pSnap, VEGGROUP_NAME) == 0) {
                    iOutputNameIndex++;
                    char *pParam = NULL;
                    if (iArgC > iOutputNameIndex) {
                        pAttr = apArgV[3];
                        iOutputNameIndex++;
                        if (iArgC > iOutputNameIndex) {
                            pParam = apArgV[4];
                        } else {
                            printf("Snap for %s: needs additional vegetation number as parameter (0, 1, 2)\n", GEOGROUP_NAME);
                            iResult = -1;
                        }
                    }
                    if (iResult == 0) {
                        QDF2VegSnap *pQ2V = QDF2VegSnap::createInstance(hFile, iNumCells, pAttr, pParam);
                        if (pQ2V != NULL) {
                            iResult = pQ2V->fillSnapData(pBuffer);
                            delete pQ2V;
                            } else {
                            iResult = -1;
                                printf("Couldn't create QDF2ClimateSnap\n");
                        }
                    }
                    
                } else if (strcasecmp(pSnap, POPGROUP_NAME) == 0) {
                    iOutputNameIndex++;
                    pAttr = apArgV[3];
                    char *pParam = apArgV[4];
                    if (bGridExists) {
                        pParam = apArgV[1];
                    }
                    printf("creating instance of QDF2PopSnap nc %d, attr[%s]\n", iNumCells, pAttr);
                    QDF2PopSnap *pQ2P = QDF2PopSnap::createInstance(hFile, iNumCells, pAttr, pParam);
                    printf("pQ2P: %p\n", pQ2P);
                    if (pQ2P != NULL) {
                        iResult = pQ2P->fillSnapData(pBuffer);
                        delete pQ2P;
                    } else {
                        iResult = -1;
                        printf("Couldn't create QDF2PopSnap\n");
                    }
                    
                } else if (strcasecmp(pSnap, MSTATGROUP_NAME) == 0) {
                    iOutputNameIndex++;
                    pAttr = apArgV[3];
                    char *pParam = apArgV[4];
                    QDF2MoveStatSnap *pQ2M = QDF2MoveStatSnap::createInstance(hFile, iNumCells, pAttr, pParam);
                    if (pQ2M != NULL) {
                        iResult = pQ2M->fillSnapData(pBuffer);
                        delete pQ2M;
                    } else {
                        iResult = -1;
                        printf("Couldn't create QDF2MoveStatSnap\n");
                    }
                    
                } else {
                    printf("snap type [%s] not iplemented yet\n", pSnap);
                    iResult = -1;
                }

                if (iResult == 0) {
                    
                    int iStep = 0;
                    float fTime = 0.0;
                    
                    SnapHeader *pSH = new SnapHeader("3.0", COORD_NODE, iStep , fTime, "ld", "dummy.ico",false, 0, pAttr,0,NULL);
                    FILE *fOut = fopen(apArgV[iOutputNameIndex], "wb");
                    pSH->write(fOut, true);
                    
                    int iW = fwrite(pBuffer, sizeof(int)+sizeof(double), iNumCells, fOut);
                    if (iW == iNumCells) {
                        printf("Written data to [%s]\n", apArgV[iOutputNameIndex]);
                        printf("+++success+++\n");
                    } else {
                        printf("Error at writeAltSnap\n");
                        iResult = -1;
                    }
                    delete pSH;
                    fclose(fOut);
                    
                }
                
                delete[] pBuffer;
    // get number of cells: open grid group; read attribute
    // or directly: hAttr = H5Aopen_by_name(hFile, "/Grid", "NumCells", H5P_DEFAULT, H5P_DEFAULT);
            }
            qdf_closeFile(hFile);
        } else {
            printf("--- Couldn't open [%s]: doesn't exist, or no QDF file\n", apArgV[1]);

        }
    } else {
        printf("Usage\n");
        printf("  %s <QDF-File> <Snap-Type> [<Param>*] <Snap-File> \n",apArgV[0]);
        printf("where\n");
        printf("  QDF-File    QDF file to read from\n");
        printf("  Snap-Type   what data to extract:\n");
        printf("              - %s\n", GEOGROUP_NAME);
        printf("                 Params:\n");
        printf("                 * %s\n", GEO_DS_ALTITUDE);
        printf("                 * %s\n", GEO_DS_AREA);
        printf("                 * %s\n", GEO_DS_LONGITUDE);
        printf("                 * %s\n", GEO_DS_LATITUDE);
        printf("                 * %s\n", GEO_DS_ICE_COVER);
        printf("                 * %s (0 | 1 | 2 | 3 | 4 | 5)\n", GEO_DS_DISTANCES);
        printf("                 * %s avg\n", GEO_DS_DISTANCES);
        printf("              - %s\n", CLIGROUP_NAME);
        printf("                 Params:\n");
        printf("                 * %s\n", CLI_DS_ACTUAL_TEMPS);
        printf("                 * %s\n", CLI_DS_ACTUAL_RAINS);
        printf("                 * %s\n", CLI_DS_ANN_MEAN_TEMP);
        printf("                 * %s\n", CLI_DS_ANN_TOT_RAIN);
        printf("              - %s\n", VEGGROUP_NAME);
        printf("                 Params:\n");
        printf("                 * %s <vegnumber>\n", VEG_DS_MASS);
        printf("                 * %s <vegnumber>\n", VEG_DS_NPP);
        printf("              - %s\n", POPGROUP_NAME);
        printf("                 Params:\n");
        printf("                 * <species-name>\n");
        printf("                 * <pure-gridfile> (optional)\n");
        printf("              - %s\n", MSTATGROUP_NAME);
        printf("                 Params:\n");
        printf("                 * Hops\n");
        printf("                 * Dist\n");
        printf("  Param       parameters (see above)\n");
        printf("  Snap-File   name of output snap file\n");
        printf("\n");
    }
    return iResult;
}
