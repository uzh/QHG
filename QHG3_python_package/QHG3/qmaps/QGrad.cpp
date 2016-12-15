
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "Vec3D.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "GeoInfo.h"
#include "Projector.h"
#include "GridProjection.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapUtils.h"
#include "QMapReader.cpp"
#include "QGradientFinder.h"

const ProjType DEF_PT_0(
                        PR_LAMBERT_AZIMUTHAL_EQUAL_AREA,
                        0.0*M_PI/180,
                        0.0*M_PI/180,
                        0,
                        NULL);


const ProjGrid DEF_PD_0(
    500,
    500,
    4.0,
    4.0,
    -250,
    -250,
    1.0);
    



#define DEF_MIN 3
//-----------------------------------------------------------------------------
// usage
//   expect string of the form <x-coord>:<y-coords>:<min>
void usage(char *pApp) {
    printf("%s - tracing paths along gradient from one or more starting points\n", pApp);
    printf("usage:\n");
    printf("  %s -i <QMap> -o <path-out>\n", pApp);
    printf("    (-c <startcoords> | -f <coordfile>)  -m <minval>\n");
    printf("     [-p] [-b][-z <stepsize>] \n");
    printf("where\n");
    printf("  QMap               QMap in which to find the gradient path (e.g. an ARRIVAL_HOPS map)\n");
    printf("  path-out           output file name\n");
    printf("  coordfile          list of starting points\n");
    printf("                     coordfile file consists of lines\n");
    printf("                       \"<iX>:<iY>\"\n");
    printf("                     iX, iY: unsigned int (grid coordinates)\n");
    printf("  startcoords        grid coordinates of starting points\n");
    printf("                     string of form\n");
    printf("                       \"<iX>:<iY>\"\n");
    printf("                     iX, iY: unsigned int (grid coords)\n");
    printf("  minval             value at which to stop path\n");
    printf("  -p                 output as PLOT commands\n");
    printf("  -b                 plot on a blank QMap\n");
    printf("  stepsize           only save every stepsize-th point found\n");
    printf("\n");
}

//-----------------------------------------------------------------------------
// splitCoords
//   expect string of the form <x-coord>:<y-coords>
//
int splitCoords(char *pCoords, int &iX, int &iY, const char *pSeparators) {
    int iResult = -1;
    
    char *pCtx;
    char *p = strtok_r(pCoords, pSeparators, &pCtx);
    if (p != NULL) {
        char *pEnd;
        iX = strtol(p, &pEnd, 10);
        if (*pEnd == '\0') {
            p = strtok_r(NULL, pSeparators, &pCtx);
            if (p != NULL) {
                iY = strtol(p, &pEnd, 10);
                if (*pEnd == '\0') {
                    iResult = 0;
                } else {
                    printf("Invalid number [%s]\n", p);
                }
            } else {
                printf("Incomplete coord string\n");
            }

        } else {
            printf("Invalid number [%s]\n", p);
        }
    } else {
        printf("Expected coords\n");
    }
    

    return iResult;
}



//-----------------------------------------------------------------------------
// outputPath
//
void outputPath(std::vector<Vec3D> &vPath, FILE *fOut, bool bPlot, int iStep, unsigned int iOffs) {
    fprintf(fOut, "# NEW PATH\n");
    for (unsigned int i = iOffs; i < vPath.size()-1; i+=iStep) {
        fprintf(fOut, "%s%d %d\n", (bPlot?((i==iOffs)?"POINT ":"LINE "):""), (int)vPath[i].m_fX, (int) vPath[i].m_fY);
    }
}

//-----------------------------------------------------------------------------
// doLocations
//
 int doLocations(QGradientFinder *pG, char *pCoordFile, double dMin, FILE *fOut, bool bPlot, int iStepSize, unsigned int iOffs) {
     
     std::vector<Vec3D> vPath;
 
     int iResult = 0;
     LineReader *pLR = LineReader_std::createInstance(pCoordFile, "rt");
     if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            vPath.clear();
            // first ine should be "population longitude latitude
            char *p = pLR->getNextLine(GNL_IGNORE_ALL);
            if (p != NULL) {
                printf("Line [%s]\n", p);
                int iX;
                int iY;
                iResult = splitCoords(p, iX, iY, " \t,;");
                printf("Doing %d %d\n", iX, iY);
                if (iResult == 0) {
                    iResult = pG->calcFlow(iX, iY, dMin, vPath);
                    if (iResult == 0) {
                        outputPath(vPath, fOut, bPlot, iStepSize, iOffs);
                    }
                }        
            }                        
        }
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pCoordFile);
    }
    return iResult;   
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sQMap[256];
    char sOut[256];
    char sCoords[256];
    char sCoordFile[256];
  
    *sQMap        = '\0';
    *sOut         = '\0';
    *sCoords      = '\0';
    *sCoordFile   = '\0';
    bool bPlot    = false;
    double dMin = dNaN;
    int iStepSize = 1;
    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(7,  
                               "-i:s!",     sQMap,
                               "-o:s!",     sOut,
                               "-c:s",      sCoords,
                               "-f:s",      sCoordFile,
                               "-p:0",     &bPlot,
                               "-m:d!",    &dMin,
                               "-z:i",     &iStepSize);

    if (bOK) {     
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if ((*sCoords != '\0') || (*sCoordFile != '\0')) {
                
                FILE *fOut = fopen(sOut, "wt");
                if (fOut != NULL) {

                    QGradientFinder *pG = QGradientFinder::createQGradientFinder(sQMap);
                    if (pG != NULL) {
                        if (*sCoordFile != '\0') {
                            iResult = doLocations(pG, sCoordFile, dMin, fOut, bPlot, iStepSize, 0);
                        } else if (*sCoords != '\0') {
                            std::vector<Vec3D> vPath;

                            int iX; 
                            int iY;
                           
                            iResult = splitCoords(sCoords, iX, iY, ":");
                            if (iResult == 0) {
                                iResult = pG->calcFlow(iX, iY, dMin, vPath);
                                if (iResult == 0) {
                                    outputPath(vPath, fOut, bPlot, iStepSize, 0);
                                }
                            }
                        } else {
                            printf("No coords or coord file given\n");
                        }
                        delete pG;
                    } else {
                        printf("Couldn't create gradient finder\n");
                    }
                    fclose(fOut);
                } else {
                    printf("Couldn't open output file [%s]\n", sOut);
                }
            } else {
                usage(apArgV[0]);
            }
        } else { 
            usage(apArgV[0]);
        }
    } else {
        printf("ParamReader option error\n");
    }

    return iResult;
}
