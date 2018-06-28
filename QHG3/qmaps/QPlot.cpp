#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>

#include "LineReader.h"
#include "QConverter.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapHeader.h"

#define MODE_NONE  -1
#define MODE_POINT  0
#define MODE_LINE   1


#define TEMP_FILE "__temp__"

typedef std::vector<std::pair<double, std::vector<std::pair<int,int> > > >plotstruct;

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - plot in to a qmap\n", pApp);
    printf("Usage:\n");
    printf("  %s <QMapIn> <PlotFile> <QmapOut> <Value> [-b] \n", pApp);
    printf("where\n");
    printf(" QMapIn     QMap to draw intofor background\n");
    printf(" PlotFile   File containing plot commands\n");
    printf(" QmapOut    QMap to save plots and background\n");
    printf(" Value      Value to plot with (will be overridden by values in plot file\n");
    printf(" -b         Use blank QMap (NaN) of same size of QMapIn to plot in\n");
    printf("\n");
    printf("Plotfile consists of lines of the form\n");
    printf("  POINT <xcoord> <ycoord> [<newval>] // plot point (first point of poly line);\n");
    printf("                                     // change plot value to newval\n");
    printf("  LINE  <xcoord> <ycoord>            // draw line to point\n");
    printf("  MARK                               // mark last point for closing loop\n");
    printf("  CLOSE                              // draw line to last marked point\n");
    printf("  # <comment>                        // comments (will be ignored\n");
    printf("\n");
}

//----------------------------------------------------------------------------
// bresenLine
//   Bresenham algorithm adapted from Wikipedia
//
void bresenLine(double **ppOut, int iW, int iH, double dVal, int x0, int y0, int x1, int y1) {
    int dx =  abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);
    int sy = y0<y1 ? 1 : -1; 
    int err = dx+dy;
    int e2; /* error value e_xy */
 
    for(;;){  /* loop */
        if ((0 <= x0) && (x0 < iW) && 
            (0 <= y0) && (y0 < iH)) {
            ppOut[y0][x0]=dVal;
        }
        if (x0==x1 && y0==y1) {
            break;
        }
        e2 = 2*err;
        if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}


//----------------------------------------------------------------------------
// createPlotStruct
//   Create a structure for the plotting:
//   psPlot: Vector of vector of points.
//   psPlot[i]: vector of points for polyline #i
//
int createPlotStruct(char *sPlotFile, plotstruct &psPlot, double dCurVal) {
    int iResult = -1;
    LineReader *pLR = LineReader_std::createInstance(sPlotFile, "rt");
    if (pLR != NULL) {
        iResult = 0;

        int iMode = MODE_NONE;
        int iMark=-1;
        bool bContinueLine=true;
        while ((!pLR->isEoF()) && (iResult == 0)) {
            bool bNewLine = false;
            char *p = pLR->getNextLine();

            // split line : Command, x, y
            char *p0 = strtok(p, " ,;\t");
            if (p0 != NULL) {
                if (strcmp(p0, "POINT") == 0) {
                    iMode = MODE_POINT;
                    iMark = 0;
                    bNewLine = true;
                    bContinueLine = true;
                } else  if (strcmp(p0, "LINE") == 0) {
                    iMode = MODE_LINE;
                    bContinueLine = true;
                } else  if (strcmp(p0, "CLOSE") == 0) {
                    psPlot.back().second.push_back(psPlot.back().second[iMark]);
                    bContinueLine = false;
                } else  if (strcmp(p0, "MARK") == 0) {
                    iMark = psPlot.back().second.size() - 1;
                    bContinueLine = false;
                } else {
                    printf("Unknown command [%s]\n", p0);
                    iResult = -1;
                }
                if (bContinueLine && (iResult == 0)) {
                    char *pEnd;
                    p0 = strtok(NULL, " ,;\t");
                    if (p0 != NULL) {
                        int x = strtol(p0, &pEnd, 10);
                        if (*pEnd == '\0') {
                            p0 = strtok(NULL, " ,;\t");
                            if (p0 != NULL) {
                                int y = strtol(p0, &pEnd, 10);
                                if (*pEnd == '\0') {
                                    std::pair<int, int> P(x,y);
                                    if (iMode == MODE_POINT) {
                                        p0 = strtok(NULL, " ,;\t");
                                        if (p0 != NULL) {
                                            double dV  = strtod(p0, &pEnd);
                                            if (*pEnd == '\0') {
                                                dCurVal = dV;
                                            } else if (strcasecmp(p0, "inf") == 0) {
                                                dCurVal = dPosInf;
                                            } else if (strcasecmp(p0, "-inf") == 0) {
                                                dCurVal = dNegInf;
                                            } else if (strcasecmp(p0, "nan") == 0) {
                                                dCurVal = dNaN;
                                            } else {
                                                printf("bad value [%s]\n", p0);
                                                iResult = -1;
                                            }
                                        }
                                    }
                                    if (bNewLine) {
                                        std::vector<std::pair<int,int> > v;

                                        std::pair<double, std::vector<std::pair<int,int> > >PL(dCurVal, v);
                                        psPlot.push_back(PL);
                                    }
                                    psPlot.back().second.push_back(P);
                                } else {
                                    iResult = -1;
                                    printf("invalid y-coord [%s]\n", p0);
                                }
                            } else {
                                iResult = -1;
                                printf("expected y-coord\n");
                            }
                            
                        } else {
                            iResult = -1;
                            printf("invalid x-coord [%s]\n", p0);
                        }
                    
                    } else {
                        iResult = -1;
                        printf("expected x-coord\n");
                    }
                }
                
            }
        }
            
            
    } else {
        printf("Couldn't open plot file [%s]\n", sPlotFile);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// plot
//   loop through poly lines and plot them
//
int plot(double **ppOut, int iW, int iH, plotstruct &psPlot) {
    int iResult = 0;
    for (unsigned int i = 0; i < psPlot.size(); i++) {
        double dValue =  psPlot[i].first;
        std::vector<std::pair<int,int> >&vPoints = psPlot[i].second;
        // plot first point of polyline if inside picture
        if ((0 <= vPoints[0].first)  && (vPoints[0].first  < iW) && 
            (0 <= vPoints[0].second) && (vPoints[0].second < iH)) {
                ppOut[vPoints[0].second][vPoints[0].first] = dValue;
        }
        if (vPoints.size() > 1) {
            // connect following points by lines
            for (unsigned int j = 1; j < vPoints.size(); j++) {
                bresenLine(ppOut, iW, iH, dValue,
                           vPoints[j-1].first, vPoints[j-1].second, 
                           vPoints[j].first,   vPoints[j].second);
            }
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    bool bBlank = false;
    if (iArgC > 4) {
        if (iArgC > 5) {
            if (strcmp(apArgV[5], "-b") == 0) {
                bBlank = true;
            }
        }
        char *pEnd;
        double dVal = strtod(apArgV[4], &pEnd);
        if (*pEnd == '\0') {

            int iType = QMAP_TYPE_NONE;
            iResult = QConverter::convert(apArgV[1], TEMP_FILE, QMAP_TYPE_DOUBLE);
            ValReader *pVR = QMapUtils::createValReader(TEMP_FILE, false, &iType);
            
            if (pVR != NULL) {
                QMapReader<double> *pQMR =dynamic_cast<QMapReader<double> *> (pVR);
                if (pQMR != NULL) {
                    

                    plotstruct psPlot;
                    iResult = createPlotStruct(apArgV[2], psPlot, dVal);
                    if (iResult == 0) {
                        int iW = pQMR->getNRLon();
                        int iH = pQMR->getNRLon();
                        double **ppData = pQMR->getData();
                        
                        // create output array
                        double **aadOut = QMapUtils::createArray(iW, iH, dNaN);
                        if (!bBlank) {
                            for (int i = 0; i < iH; i++) {
                                memcpy(aadOut[i], ppData[i], iW *sizeof(double));
                            }
                        }

                        plot(aadOut, iW, iH, psPlot);

                        // now save output data
                        QMapHeader *pQMH = new QMapHeader(iType,
                                                          pVR->getLonMin(), pVR->getLonMax(), pVR->getDLon(),
                                                          pVR->getLatMin(), pVR->getLatMax(), pVR->getDLat(),
                                                          pVR->getVName(),  pVR->getXName(),  pVR->getYName());
                        
                        // add QMap header and write to file
                        FILE *fOut = fopen(apArgV[3], "wb");
                        if (fOut != NULL) {
                            pQMH->addHeader(fOut);
                            bool bOK = QMapUtils::writeArray(fOut, iW, iH, aadOut);
                            
                            fclose(fOut);
                            iResult = bOK?0:-1;

                        } else {
                            printf("Couldn't open %s\n", apArgV[3]);
                        }
                        QMapUtils::deleteArray(iW, iH, aadOut);
                    }
                } else {
                    printf("Error converting qmap to double type\n");
                }
            } else {
                printf("Couldn't open QMap [%s]\n", apArgV[1]);
            }
        } else {
            printf("Invalid Value: [%s]\n", apArgV[3]);
        }
    } else {
        usage(apArgV[0]);
    }
    if (iResult == 0) {
        printf("+++ success +++\n");
    }
    return iResult;

}
