#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ParamReader.h"
#include "GeoInfo.h"
#include "Projector.h"
#include "GridProjection.h"
//#include <ncurses/curses.h>


    
//---------------------------------------------------
// usage
//
void usage(char *pName) {
    printf("%s - convert between projections\n", pName);
    printf("Uge:\n");
    printf("  %s  -t <ProjectionType> -d <ProjectionData1> [-g] \n", pName);
    printf("where\n");
    printf("  ProjectionType1: Data for ProjectionType of input\n");
    printf("  ProjectionData1: Data for ProjectionData of input\n");
    printf("  -g             : all angles in degrees\n");
    printf("\n");
    printf("Data for ProjectionType has form \n");
    printf("  <Type>:<long0>:<lat0>:<NumPar>(:<Par>)*\n");
    printf("where\n");
    printf("  Type: ProjectionType\n");
    printf("    0:EQUIRECTANGULAR\n");
    printf("    1:ORTHOGRAPHIC\n");
    printf("    2:AZIMUTHAL_EQUIDISTANT\n");
    printf("    3:TRANSVERSE_CYLINDRCAL_EQUAL_AREA\n");
    printf("    4:LAMBERT_AZIMUTHAL_EQUAL_AREA\n");
    printf("    5:LAMBERT_CONFORMAL_CONIC\n");
    printf("  long0,lat0: longitude and latitude of projection center (radians)\n");
    printf("  NumPar:     Number of additional parameters\n");
    printf("  Par:        Parameter\n");
    printf("\n");
    printf("Data for ProjectionData has form \n");
    printf("  <GridW>:<GridH>:<RealW>:<RealH>:<OffX>:<OffY>:<R>\n");
    printf("where\n");
    printf("  GridW, GridH: size of grid\n");
    printf("  RealW, RealH: real size of area\n");
    printf("  OffX, OffY:   offsets\n");
    printf("  R:            radius of sphere\n");
}
   
bool showHelp() {
    printf(">s X Y    : convert plane coordinates (X,Y) to longitude, latitude\n");
    printf(">p L P    : convert sphere coordinates (L,P) to plane coords\n");
    printf(">>s X Y   : convert grid coordinates (X,Y) to longitude, latitude\n");
    printf(">>p L P   : convert sphere coordinates (L,P) to grid coords\n");
    //    printf(":P <pt>  : set new projection\n");
    printf("           pt ::= <Type>:<long0>:<lat0>:<NumPar>(:<Par>)*\n");
    printf("!        : show projection\n");
    printf("?        : show this text\n");
    printf("q        : quit\n");
    return true;
}

bool conversion(Projector *pP0, char *pLine, bool bDegrees, bool bVerbose) {
    bool bOK = false;
    if ((*pLine == 's') || (*pLine == 'p')) {
        char c = *pLine;
        pLine++;
        char *p0 = strtok(pLine, " \n\t");
        if (p0 != NULL) {
            double dI1 = atof(p0);
            p0 = strtok(NULL, " \n\t");
            if (p0 != NULL) {
                double dI2 = atof(p0);
                double dO1;
                double dO2;
                if (c == 's') {
                    double dO1b;
                    double dO2b;

                    
                    pP0->plane2Sphere(dI1, dI2, dO1, dO2);
                    if (bDegrees) {
                        dO1b = RAD2DEG(dO1);
                        dO2b = RAD2DEG(dO2);
                    } else {
                        dO1b = dO1;
                        dO2b = dO2;
                    }
                    if (bVerbose) {
                        printf("  plane %f, %f --> sphere:  %f, %f\n", dI1, dI2, dO1b, dO2b);
                    } else {
                        printf("%f, %f\n", dO1b, dO2b);
                    }
                    double dX1;
                    double dX2;
                    pP0->sphere2Plane(dO1, dO2, dX1, dX2);
                    if (bVerbose) {
                        printf("     back --> %f, %f\n", dX1, dX2);
                    }
                    bOK = true;
                } else if  (c == 'p') {
                    double dI1b;
                    double dI2b;
                    if (bDegrees) {
                        dI1b = DEG2RAD(dI1);
                        dI2b = DEG2RAD(dI2);
                    } else {
                        dI1b = dI1;
                        dI2b = dI2;
                    }

                    pP0->sphere2Plane(dI1b, dI2b, dO1, dO2);
                    if (bVerbose) {
                        printf("  sphere %f, %f --> plane: %f, %f\n",  dI1, dI2, dO1, dO2);
                    } else {
                        printf("%f, %f\n", dO1, dO2);
                    }
                    double dX1;
                    double dX2;
                    pP0->plane2Sphere(dO1, dO2, dX1, dX2);
                    if (bDegrees) {
                        dX1 = RAD2DEG(dX1);                    
                        dX2 = RAD2DEG(dX2);
                    }                    

                    if (bVerbose) {
                        printf("     back --> %f, %f\n", dX1, dX2);
                    }
                    bOK = true;
                }


            
            }
        }
    }
    return bOK;
}

bool conversionG(GridProjection *pGP0, char *pLine, bool bDegrees, bool bVerbose) {
    bool bOK = false;
    if ((*pLine == 's') || (*pLine == 'p')) {
        char c = *pLine;
        pLine++;
        char *p0 = strtok(pLine, " \n\t");
        if (p0 != NULL) {
            double dI1 = atof(p0);
            p0 = strtok(NULL, " \n\t");
            if (p0 != NULL) {
                double dI2 = atof(p0);
                double dO1;
                double dO2;
                if (c == 's') {
                    double dO1b;
                    double dO2b;

                    pGP0->gridToSphere(dI1, dI2, dO1, dO2);
                    if (bDegrees) {
                        dO1b = RAD2DEG(dO1);
                        dO2b = RAD2DEG(dO2);
                    } else {
                        dO1b = dO1;
                        dO2b = dO2;
                    }
                    if (bVerbose) {
                        printf("  plane %f, %f --> sphere:  %f, %f\n", dI1, dI2, dO1b, dO2b);
                    } else {
                        printf("%f, %f\n", dO1b, dO2b);
                    }
                    double dX1;
                    double dX2;
                    pGP0->sphereToGrid(dO1, dO2, dX1, dX2);
                    if (bVerbose) {
                        printf("     back --> %f, %f\n", dX1, dX2);
                    }
                    bOK = true;
                } else if  (c == 'p') {
                    double dI1b;
                    double dI2b;
                    if (bDegrees) {
                        dI1b = DEG2RAD(dI1);
                        dI2b = DEG2RAD(dI2);
                    } else {
                        dI1b = dI1;
                        dI2b = dI2;
                    }
                    pGP0->sphereToGrid(dI1b, dI2b, dO1, dO2);
                    if (bVerbose) {
                        printf("  sphere %f, %f --> plane: %f, %f\n",  dI1, dI2, dO1, dO2);
                    } else {
                        printf("%f, %f\n", dO1, dO2);
                    }
                    double dX1;
                    double dX2;
                    pGP0->gridToSphere(dO1, dO2, dX1, dX2);
                    if (bDegrees) {
                        dX1 = RAD2DEG(dX1);                    
                        dX2 = RAD2DEG(dX2);
                    }                    
                    if (bVerbose) {
                        printf("     back --> %f, %f\n", dX1, dX2);
                    }
                    bOK = true;
                }


            
            }
        }
    }
    return bOK;
}


bool setProj(Projector **ppP0, ProjType **ppPT, char *p) {
    bool bOK = false;
    if (*p == 'P') {
        ProjType  *pPTOld = *ppPT;
        p++;
        *ppPT = ProjType::createPT(p, false);
        delete pPTOld;
        delete *ppP0;
        *ppP0 = GeoInfo::instance()->createProjector(*ppPT);
        bOK = true;
    }
    return bOK;
}

const int NHIST = 10;

char sHistory[NHIST][256];
int iHStart = 0; 
int iHEnd   = iHEnd;
int iHNum   = 0;

char *getInput(char *pLine) {
    int i = 0;
    char c = '\0';
    while (c != 0x0d) {
        
        c = getchar(); 
        
        if (c == 0x1b) {
            int j = 0;
            do {
                c = getchar(); 
                if (c == 0x5b) {
                    c = getchar(); 
                    if (c == 0x41) {
                        j++;
                        int k = iHEnd-j;
                        if (k < 0) {
                            k += NHIST;
                        }
                        if (k != iHStart) {
                            printf("\r>> %s", sHistory[j]);fflush(stdout);
                            strcpy(pLine, sHistory[j]);
                            i = strlen(sHistory[j]);
                        }
                    } else if (c == 0x42) {
                        j--;
                        if (j >= 0) {
                            printf("\r>> %s", sHistory[j]);fflush(stdout);
                            strcpy(pLine, sHistory[j]);
                            i = strlen(sHistory[j]);
                        }
                    }
                }
                c = getchar(); 
            } while (c == 0x1b);
            
        } 
        if (c!=0x0d) {
            printf("%c",c);fflush(stdout);
            pLine[i++] = c;
            
        } else {
            if (iHNum < NHIST) {
                strcpy(sHistory[iHNum++], pLine);
                iHEnd = iHNum;
            } else {
                iHEnd = iHStart;
                strcpy(sHistory[iHStart++], pLine);
            }
        }
    }
    return pLine;
}


int main(int iArgC, char *apArgV[]) {
    
    for (int i =0; i < NHIST; i++) {
        *sHistory[i] = '\0';
    }


    char sType1[SHORT_INPUT];
    char sData1[SHORT_INPUT];
    char sCommand[SHORT_INPUT];
    *sType1 = '\0';
    *sData1 = '\0';
    *sCommand = '\0';
    bool bDegrees = false;
    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(4,  
                               "-t:s!", sType1,
                               "-d:s!", sData1,
                               "-c:s",  sCommand,
                               "-g:0",  &bDegrees);
    if (bOK) {
        int iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            ProjType *pPT0 = ProjType::createPT(sType1, bDegrees);
            ProjGrid *pPG0 = ProjGrid::createPG(sData1);
            if (*sCommand == '\0') {
                printf("pt1: %s\n", pPT0->toString(bDegrees));
                printf("pg1: %s\n", pPG0->toString());
            }
            GeoInfo *pGI = GeoInfo::instance();


            //            Projector *pP0 = pGI->createProjector(pPT0->m_iProjType, pPT0->m_dLambda0, pPT0->m_dPhi0);
            //            GridProjection *pGP0 = new GridProjection(pPD0->m_iGridW, pPD0->m_iGridH, pPD0->m_dRealW, pPD0->m_dRealH, pPD0->m_dRadius, pP0, false);
            
            Projector *pP0 = pGI->createProjector(pPT0);
            GridProjection *pGP0 = new GridProjection(pPG0, pP0, false, false);
            

            if (*sCommand != '\0') {
                char *p = sCommand;
                if (*p == '>') {
                    p++;
                    if (*p == '>') {
                        bOK = conversionG(pGP0, ++p, bDegrees, false);
                    } else {
                        bOK = conversion(pP0, p, bDegrees, false);
                    }
                } 
            } else {
                char sInput[256];
                char sSave[256];
                bool bCont = true;
            
                showHelp();fflush(stdout);
                while (bCont) {
                    printf("::");
                    char *p = fgets(sInput, 256, stdin);
                    //                char *p = fgets(sInput, 256, stdin);
                    if (p != NULL) {
                        p[strlen(p)-1] = '\0';
                        strcpy(sSave, p);
                        switch (*p) {
                        case '>':
                            p++;
                            if (*p == '>') {
                                bOK = conversionG(pGP0, ++p, bDegrees, true);
                            } else {
                                bOK = conversion(pP0, p, bDegrees, true);
                            }
                            break;
                        case 'h':
                        case '?':
                            bOK = showHelp();
                            break;
                        case ':':
                            bOK = setProj(&pP0, &pPT0, ++p);
                            break;
                        case '!':
                            printf("pt1: %s\n", pPT0->toString(bDegrees));
                            break;
                        case 'q':
                        case '\0':
                        case '\n':
                            bCont = false;
                            break;
                        default:
                            printf(" ???\n");
                        }

                        if (iHNum < NHIST) {
                            strcpy(sHistory[iHNum++], sSave);
                            iHEnd = iHNum;
                        } else {
                            iHEnd = iHStart;
                            strcpy(sHistory[iHStart++], sSave);
                        }

                    }
                }
            }
            GeoInfo::free();
            delete pP0;
            delete pGP0;
            delete pPT0;
            delete pPG0;
        } else {
            usage(apArgV[0]);
        }
    
        
        /*
        char c = '\0';
        while (c != 0x0d) {
            c = getchar(); 
            printf("*");fflush(stdout);
        }
        echo();
        endwin();
        */

        /*
        initscr();
        noecho();

        printf(">> ");fflush(stdout);
        char sLine[256];
        char sLast[256];
        strcpy(sLast, "guguseli");
        int i = 0;
        char c = '\0';
        while (c != 0x0d) {
            
            c = getchar(); 
 
            if (c == 0x1b) {
                int j = -1;
                do {
                    c = getchar(); 
                    if (c == 0x5b) {
                        c = getchar(); 
                        if (c == 0x41) {
                            j++;
                            int k = iHEnd-j;
                            if (k < 0) {
                                k += NHIST;
                            }
                            if (k != iHStart) {
                                printf("                                                                                               \r>> %s", sHistory[j]);fflush(stdout);
                                strcpy(sLine, sLast);
                                i = strlen(sLast);
                            }
                        } else if (c == 0x42) {
                            j--;
                            if (j >= 0) {
                                int k = iHEnd-j;
                                if (k < 0) {
                                    k += NHIST;
                                }
                                if (k != iHStart) {
                                    printf("                                                                                               \r>> %s", sHistory[j]);fflush(stdout);
                                    strcpy(sLine, sLast);
                                    i = strlen(sLast);
                                }
                            }
                        }
                    }
                    c = getchar(); 
                } while (c == 0x1b);
 
            } 
            if (c!=0x0d) {
                printf("%c",c);fflush(stdout);
                sLine[i++] = c;
                
            } else {
                if (iHNum < NHIST) {
                    strcpy(sHistory[iHNum++], sLine);
                    iHEnd = iHNum;
                } else {
                    iHEnd = iHStart;
                    strcpy(sHistory[iHStart++], sLine);
                }
            }
        }
        echo();
        endwin();
        sLine[i] = '\0';
        printf("\n");
        printf("%s\n", sLine);
        
        */
    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;
}


