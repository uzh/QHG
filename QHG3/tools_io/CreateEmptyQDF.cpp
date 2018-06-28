#include <stdio.h>
#include <string.h>

#include <hdf5.h>

#include "SCellGrid.h"
#include "Geography.h"
#include "Climate.h"
#include "Vegetation.h"

#include "GridFactory.h"
#include "StatusWriter.h"


//-----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - create an empty QDF file\n", pApp);
    printf("usage:\n");
    printf("  %s <ignfile> [<specifiers>] <output-QDF-name>\n", pApp);
    printf("where\n");
    printf("  ignfile          name of ico gridnode file\n");
    printf("  output-QDF-name  name for output QDF file\n");
    printf("  specifiers       codes for data to convert:\n");
    printf("                     S : grid data\n");
    printf("                     G : geography data\n");
    printf("                     C : climate data\n");
    printf("                     V : vegetation data\n");
    printf("\n");
}

//-----------------------------------------------------------------------------
// parseSpecs
//
int parseSpecs(char *pSpecs) {
    int iSpecs = WR_NONE;
    char *p = pSpecs;
    bool bOK = true;
    while (bOK && (*p != '\0')) {
        switch (*p) {
        case 'S':
            iSpecs |= WR_GRID;
            break;
        case 'G':
            iSpecs |= WR_GEO;
            break;
        case 'C':
            iSpecs |= WR_CLI;
            break;
        case 'V':
            iSpecs |= WR_VEG;
            break;
        default:
            printf("Unknown specifier: [%c]\n", *p);
            bOK = false;
            iSpecs = WR_NONE;
        }
        p++;
    }
    return iSpecs;
}

//-----------------------------------------------------------------------------
// createGridFactory
//
GridFactory *createGridFactory(const char *pInput) {
    int iResult = 0;
    GridFactory *pGF = new GridFactory();
    if (pGF->isReady()) {
        if (iResult == 0) {
            printf("Successfully read\n");
            if (pGF->getCellGrid() != NULL) {
                printf("  - grid data\n");
            }
            if (pGF->getGeography() != NULL) {
                printf("  - geo data\n");
            }
            if (pGF->getClimate() != NULL) {
                pGF->getCellGrid()->setClimate(pGF->getClimate());
                printf("  - climate data\n");
            }
            if (pGF->getVeg() != NULL) {
                pGF->getCellGrid()->setVegetation(pGF->getVeg());
                printf("  - vegetation data\n");
            }
            printf("from def file [%s]\n", pInput);
        } else {
            printf("Couldn't read definintion file [%s]\n", pInput);
            iResult = -1;
        }
    } else {
        printf("Couldn't open [%s] for reading\n", pInput);
        iResult = -1;
    }
    if (iResult != 0) {
        delete pGF;
        pGF = NULL;
    }
    return pGF;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 2) {
        char *pInput = apArgV[1];
        char *pOutput = apArgV[2];
        uint iSpecs = WR_ALL;
        
        if (iArgC > 3) {
            pOutput = apArgV[3];
            iSpecs = parseSpecs(apArgV[2]);
        }
        if (iSpecs != WR_NONE) {
            GridFactory *pGF = new GridFactory();
            if (pGF != NULL) {
                iResult = pGF->createEmptyQDF(pInput);
                if (iResult == 0) {
                    float fNoTime = -1;
                    std::vector<PopBase *> vDummy;
                    StatusWriter *pSW = StatusWriter::createInstance(pGF->getCellGrid(), vDummy);
                    if (pSW != NULL) {
                        iResult = pSW->write(pOutput, fNoTime, iSpecs);
                        if (iResult >= 0) {
                            if (iResult > 0) {
                                printf("couldn't write:\n");
                                if ((iResult & WR_POP) != 0) {
                                    printf("- Populations\n");
                                }
                                if ((iResult & WR_GRID) != 0) {
                                    printf("- Grid\n");
                                }
                                if ((iResult & WR_GEO) != 0) {
                                    printf("- Geography\n");
                                }
                                if ((iResult & WR_CLI) != 0) {
                                    printf("- Climate\n");
                                }
                                if ((iResult & WR_VEG) != 0) {
                                    printf("- Vegetation\n");
                                }
                                if ((iResult & WR_STAT) != 0) {
                                    printf("- Move stats\n");
                                }
                                if (iResult < (int)iSpecs) {
                                    iResult = 0;
                                }
                            }
                            if (iResult == 0) {
                                printf("Successfully exported data to QDF format\n");
                                printf("+++success+++\n");
                            }
                        } else {
                            printf("failed to write\n");
                        }
                        delete pSW;
                    } else {
                        printf("Couldn't create status writer\n");
                        iResult = -1;
                    }
                    delete pGF;
                } else {
                    printf("Failed to create cell grid\n");
                    iResult = -1;
                }
            } else {
                printf("Couldn^'f create GridFactory\n");
                iResult = -1;
            }

        } else {
            printf("At least one data item must be specified\n");
            iResult = -1;
        }
        
    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
