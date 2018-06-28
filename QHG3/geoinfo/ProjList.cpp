#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ParamReader.h"
#include "LineReader.h"
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
    printf("  %s  -l <PointList> (-s | -p)  -t <ProjectionType> -d <ProjectionData1> [-g] \n", pName);
    printf("where\n");
    printf("  PointList      : List of points(either gridx,gridy or lon,lat)\n");
    printf("  ProjectionType1: Data for ProjectionType of input\n");
    printf("  ProjectionData1: Data for ProjectionData of input\n");
    printf("  -s             :project to sphere (points are gridx,gridy)\n");
    printf("  -p             :project to plane (points are lon, lat)\n");
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
   

int convertToSphere(GridProjection *pGP0, char *pLine, double *pdLon, double *pdLat, bool bDegrees) {
    int iResult = -1;
    char *p0 = strtok(pLine, " ,;\n\t");
    if (p0 != NULL) {
        double dI1 = atof(p0);
        p0 = strtok(NULL, " ,;\n\t");
        if (p0 != NULL) {
            double dI2 = atof(p0);
            double dO1;
            double dO2;
                
            pGP0->gridToSphere(dI1, dI2, dO1, dO2);
            if (bDegrees) {
                *pdLon = RAD2DEG(dO1);
                *pdLat = RAD2DEG(dO2);
            } else {
                *pdLon = dO1;
                *pdLat = dO2;
            }
            
            iResult = 0;
        }
        
    }
    return iResult;
}

int convertToGrid(GridProjection *pGP0, char *pLine, int *px, int *py, bool bDegrees) {
    int iResult = -1;
    char *p0 = strtok(pLine, " ,;\n\t");
    if (p0 != NULL) {
        double dI1 = atof(p0);
        p0 = strtok(NULL, " ,;\n\t");
        if (p0 != NULL) {
            double dI2 = atof(p0);

            double dO1;
            double dO2;

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

            *px = (int)dO1;
            *py = (int)dO2;

            iResult = -1;
         }
    }
    return iResult;
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


void convertListToGrid(LineReader *pLR, GridProjection *pGP0, bool bDegrees, FILE *fOut) {
    //    printf("COnverting to grid\n");
    int x=-1;
    int y=-1;
    while (!pLR->isEoF()) {
        char *p = pLR->getNextLine();
        if (p != NULL) {
            convertToGrid(pGP0, p, &x, &y, bDegrees);
            fprintf(fOut, "%d %d\n", x, y);
        }
    }
    
}

void convertListToSphere(LineReader *pLR, GridProjection *pGP0, bool bDegrees, FILE *fOut) {
    //    printf("COnverting to sphere\n");
    double dLon=dNaN;
    double dLat=dNaN;

    while (!pLR->isEoF()) {
        char *p = pLR->getNextLine();
        if (p != NULL) {
            convertToSphere(pGP0, p, &dLon, &dLat, bDegrees);
            fprintf(fOut, "%f %f\n", dLon, dLat);
        }
    }
    
}


int main(int iArgC, char *apArgV[]) {
    

    char sType1[SHORT_INPUT];
    char sData1[SHORT_INPUT];
    char sPointList[SHORT_INPUT];
    char sOutput[SHORT_INPUT];
    *sType1 = '\0';
    *sData1 = '\0';
    *sOutput = '\0';
    *sPointList = '\0';
    bool bDegrees = false;
    bool bToSphere  = false;
    bool bToGrid =false;
    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(7,  
                               "-t:s!", sType1,
                               "-d:s!", sData1,
                               "-l:s",  sPointList,
                               "-s:0",  &bToSphere,
                               "-p:0",  &bToGrid,
                               "-o:s",  sOutput,
                               "-g:0",  &bDegrees);
    if (bOK) {
        int iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            FILE *fOut = NULL;
            if (*sOutput == '\0') {
                fOut = stdout;
            } else {
                fOut = fopen(sOutput, "wt");
            }
            
            //            if (bToGrid || bToSphere) {
            ProjType *pPT0 = ProjType::createPT(sType1, bDegrees);
            ProjGrid *pPG0 = ProjGrid::createPG(sData1);
            GeoInfo *pGI = GeoInfo::instance();


            //            Projector *pP0 = pGI->createProjector(pPT0->m_iProjType, pPT0->m_dLambda0, pPT0->m_dPhi0);
            //            GridProjection *pGP0 = new GridProjection(pPD0->m_iGridW, pPD0->m_iGridH, pPD0->m_dRealW, pPD0->m_dRealH, pPD0->m_dRadius, pP0, false);
            
            Projector *pP0 = pGI->createProjector(pPT0);
            GridProjection *pGP0 = new GridProjection(pPG0, pP0, false, false);

            
            LineReader *pLR = LineReader_std::createInstance(sPointList, "rt");
            if (pLR != NULL) {
                if (bToGrid) {
                    convertListToGrid(pLR, pGP0, bDegrees, fOut);
                } else {
                    convertListToSphere(pLR, pGP0, bDegrees, fOut);
                }
                

                delete pLR;
            } else {
                printf("Couldn't open [%s]\n", sPointList);
            }
            GeoInfo::free();
            delete pP0;
            delete pGP0;
            delete pPT0;
            delete pPG0;

            if (*sOutput != '\0') {
                fclose(fOut);
            }

        } else {
            usage(apArgV[0]);
        }
    
 
    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;
}


