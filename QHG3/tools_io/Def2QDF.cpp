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
    printf("%s - convert a grid definition file to QDF file\n", pApp);
    printf("usage:\n");
    printf("  %s \"<icoline>\" [<specifiers>] <output-QDF-name>\n", pApp);
    printf("or\n");
    printf("  %s \"<flatline>\" [<specifiers>] <output-QDF-name>\n", pApp);
    printf("or\n");
    printf("  %s <grid-def-file> [<specifiers>] <output-QDF-name>\n", pApp);
    printf("where\n");
    printf("  icoline          command to build EQsahedron grid\n");
    printf("                   format: see below under \"icoline\"\n");
    printf("  flatline         command to build a flat grid\n");
    printf("                   format: see below under \"flatline\"\n");
    printf("  grid-def-file    name of grid definition file\n");
    printf("  output-QDF-name  name for output QDF file\n");
    printf("  specifiers       codes for groups to add:\n");
    printf("                     S : grid data\n");
    printf("                     G : geography data\n");
    printf("                     C : climate data\n");
    printf("                     V : vegetation data\n");
    printf("                   if ignored, all groups are added.\n");
    printf("\n");
    printf("Grid file format:\n");
    printf("  gridfile ::= <defline>*\n");
    printf("  defline  ::= <gridline> | <altline> | <iceline> | <templine> | <rainline> | <vegline>\n");
    printf("  gridline ::= <icoline> | <flatline>\n");
    printf("  icoline  ::= \"GRID_TYPE ICO\" (<subdivs> | <ignfile>)\n");
    printf("  subdivs  :   number of subdivisions\n");
    printf("  ignfile  :   name of ico gridnode file\n");
    printf("  flatline ::= \"GRID_TYPE\" <conn> <width>\"x\"<height> [\"PERIODIC\"]\n");
    printf("  conn     ::= \"HEX\" | \"RECT\"\n");
    printf("  width    :   width of grid\n");
    printf("  height   :   height of grid\n");
    printf("  altline  ::= \"ALT FLAT\" <altval>   | \"ALT QMAP\"  <altqmap>\n");
    printf("  iceline  ::= \"ICE QMAP\" <iceqmap>\n");
    printf("  templine ::= \"TEMP FLAT\" <tempval> | \"TEMP QMAP\" <tempqmap>\n");
    printf("  rain     ::= \"RAIN FLAT\" <rainval> | \"RAIN QMAP\" <rainqmap>\n");
    printf("  vegline  ::= \"ANPP FLAT\" <nppval>  | \"ANPP QMAP\" <nppqmap>\n");
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
    GridFactory *pGF = new GridFactory(pInput);
    if (pGF->isReady()) {
        iResult = pGF->readDef();
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
            GridFactory *pGF = createGridFactory(pInput);
            if (pGF != NULL) {
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
