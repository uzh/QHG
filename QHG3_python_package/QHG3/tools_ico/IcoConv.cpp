#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strutils.h"
/*
#include "LineReader.h"
#include "DescReader.h"
#include "HeaderBase.h"
#include "WorldHeader.h"
#include "SnapHeader.h"
#include "PopHeader.h"
*/
#include "GeoInfo.h"
#include "Projector.h"
#include "GridProjection.h"

#include "IcoLoc.h"
#include "Icosahedron.h"
#include "RectLoc.h"
#include "IcoConverter.h"

#define NUMBLOCKS 10000

#define FTYPE_NONE -1
#define FTYPE_POP   1
#define FTYPE_WRLD  2
#define FTYPE_SNAP  3
bool s_bVerbose = false;
//-----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - converting Pop files to Ico style or back\n", pApp);
    printf("   (replacing coordinates by IDs of corresponding nodes\n");
    printf("    of a reference subdivided Icosahedron or of a plane projection)\n");
    printf("Usage:\n");
    printf("  %s <IcoFile> (\"-n\" | \"-c\") [-p] <popfile> [<popfile>*]\n", pApp);
    printf("or\n");
    printf("  %s <projType> <projGrid> (\"-n\" | \"-c\") [-g] <popfile> [<popfile>*]\n", pApp);
    printf("where\n");
    printf("  IcoFile    an icosahedron file to which nodes the popfiles should be mapped\n");
    printf("  projType   projection type for a grid rect\n");
    printf("  projGrid   projection grid for a grid rect\n");
    printf("  popfile    a POP file to be converted (ASC or BIN)\n");
    printf("  -n         convert from coordinate style to ico style\n");
    printf("  -r         convert from ico style to coordinate style\n");
    printf("  -p         set preSelect option of icosahedron\n");
    printf("  -g         all angles are degrees\n");
    printf("\n");
    printf("Data for ProjectionType has form \n");
    printf("  <Type>:<long0>:<lat0>:<NumPar>(:<Par>)*\n");
    printf("where\n");
    printf("  Type: ProjectionType\n");
    for (unsigned int i = 0; i < sizeof(asProjNames)/sizeof(char *); i++) {
        printf("    %d:%s\n", i, asProjNames[i]);
    }
    printf("  long0,lat0: longitude and latitude of projection center (radians)\n");
    printf("  NumPar:     Number of additional parameters\n");
    printf("  Par:        Additiojnal parameter for projection\n");
    printf("\n");
    printf("Data for ProjectionData has form \n");
    printf("  <GridW>:<GridH>:<RealW>:<RealH>:<OffX>:<OffY>:<R>\n");
    printf("where\n");
    printf("  GridW, GridH: size of grid\n");
    printf("  RealW, RealH: real size of area\n");
    printf("  OffX, OffY:   offsets\n");
    printf("  R:            radius of sphere\n");
    printf("\n");
    printf("The converted files have names with an inserted\n");
    printf("   \"-n\" or \"-c\" before the suffix\n");
    printf("\n");
    printf("Examples\n");
    printf("  %s euro_9.ico -n qhg_Cows.pop qhg_Wolves.pop\n", pApp);
    printf("  %s 3:0:0:0 400:200:3.14:1.57:c:c:1 -n qhg_Cows.pop qhg_Wolves.pop\n", pApp);
    printf("Output files in these examples:  qhg_Cows-n.pop qhg_Wolves-n.pop\n");
    printf("\n");
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sIcoFile[256];
    if (iArgC > 3) {
        int iOutPar = 3; // start of input files
        int iConvPar = 2;
        bool bPreSel = false;
        IcoLoc *pIcoLoc = NULL;
        
        ProjType *pPT0 = NULL;
        ProjGrid *pPG0 = NULL;
        Projector *pP0 = NULL;
        GridProjection *pGP0 = NULL;
            
        // check if the first and second argument contain ":" -> projT, projG
        if ((strchr(apArgV[1], ':') != NULL) && (strchr(apArgV[2], ':') != NULL)) {
            bool bDegrees = false;
            if (iArgC > 4) {
                if (strcmp(apArgV[3], "-g")==0) {
                    bDegrees = true;
                }
            }
            pPT0 = ProjType::createPT(apArgV[1],bDegrees);
            pPG0 = ProjGrid::createPG(apArgV[2]);
            GeoInfo *pGI = GeoInfo::instance();
            pP0 = pGI->createProjector(pPT0);
            pGP0 = new GridProjection(pPG0, pP0, false, false);
            pIcoLoc = new RectLoc(pGP0);
            iOutPar++; // the input file start one pos later as in the minimu case
            iConvPar++;
        } else {
            if (iArgC > 4) {
                if (strcmp(apArgV[3],"-p")==0) {
                    bPreSel = true;
                    iOutPar++;
                }
            }
            // load the *entire* Icodaherdron (strict=false)
            strcpy(sIcoFile, apArgV[1]);
            Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
            pIcoLoc = pIco;
            pIco->setStrict(true);
            pIco->setPreSel(bPreSel);
            iResult = pIco->load(sIcoFile);
            if (iResult == 0) {
                pIco->relink();
            }
        }
        if (iResult == 0) {
            int iOp = -1;
            if (strcmp(apArgV[iConvPar], "-n") == 0) {
                iOp = 0;
            } else if (strcmp(apArgV[iConvPar], "-c") == 0) {
                iOp = 1;
            }
            if (iOp >= 0) {
                int i = iOutPar;

                IcoConverter *pIC = IcoConverter::createInstance(pIcoLoc, (pGP0 == NULL)?apArgV[1]:"(none)", bPreSel);
                if (pIC != NULL) {
                
                while ((iResult == 0) && (i < iArgC)) {
                    char sOut[256];
                    strcpy(sOut, apArgV[i]);
                    char *p = strrchr(sOut, '.');
                    if (p != NULL) {
                        *p = '\0';
                        p++;
                        strcat(sOut, apArgV[iConvPar]);
                        strcat(sOut, ".pop");
                        //                        strcat(sOut, p);

                                                
                        if (iOp == 0) {
                            iResult = pIC->nodifyFile(apArgV[i], sOut);
                        } else  if (iOp == 1) {
                            iResult = pIC->coordifyFile(apArgV[i], sOut);
                        }
                        if (iResult == 0) {
                            printf("+++ success +++\n");
                        }
                    } else {
                        printf("Filename without suffix [%s]\n", apArgV[i]);
                        iResult = -1;
                    }
                    i++;
                }
                delete pIC;
                } else {
                    printf("Couldn't create converter\n");
                    iResult = -1;
                }
            } else {
                printf("Invalid option [%s]\n", apArgV[2]);
                iResult = -1;
            }
        } else {
            printf("Couldn' load Iccosahedron [%s]\n", apArgV[2]);
            iResult = -1;
        }
        delete pIcoLoc;
        if (pGP0 != NULL) {
            GeoInfo::free();
            delete pP0;
            delete pGP0;
            delete pPT0;
            delete pPG0;
        }

    } else {
        iResult = -1;
        usage(apArgV[0]);
    }
    return iResult;
}

