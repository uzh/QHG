#include <stdio.h>
#include <string.h>


#include "utils.h"
#include "strutils.h"
#include "ParamReader.h"
#include "Icosahedron.h"
#include "VertexLinkage.h"
#include "IcoFace.h"
#include "NodeLister.h"

//-----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - extract a range of data from a ico snap\n", pApp);
    printf("Usage\n");
    printf("  %s -i <icofile> -d <snapfile> [-R <range>] -r<deltaLon>x<deltaLat> -o <outputfile>\n", pApp);
    printf("  %s -i <icofile> -d <snapfile> [-R <range>] -s<width>x<height> -o <outputfile>\n", pApp);
    printf("where\n");
    printf("  icofile    icosahedron file used to calculate the snap data\n");
    printf("  snapfile   snap file\n");
    printf("  range      range to extract <LonMin>\":\"<LonMax>\":\"<LatMin>\":\"<LatMax>\n");
    printf("  deltaLon   longitude step size (width will be calculated)\n");
    printf("  deltaLat   latitude step size (heigh will be calculated)\n");
    printf("  width      table width (longitude step size will be calculated)\n");
    printf("  height     table height (latitude step size will be calculated)\n");
    printf("\n");
}

//-----------------------------------------------------------------------------
// splitRange
//
int splitRange(char *pRange, double *pdLonMin, double *pdLonMax, double *pdLatMin, double *pdLatMax) {
    int iResult = -1;
    char *p = strtok(pRange, ":");
    if (p != NULL) {
        if (strToNum(p, pdLonMin)) {
            p = strtok(NULL, ":");
            if (p != NULL) {
                if (strToNum(p, pdLonMax)) {
                    p = strtok(NULL, ":");
                    if (p != NULL) {
                        if (strToNum(p, pdLatMin)) {
                            p = strtok(NULL, ":");
                            if (p != NULL) {
                                if (strToNum(p, pdLatMax)) {
                                    iResult =  0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// splitDelta
//
int splitDelta(char *pDelta, double *pdDeltaLon, double *pdDeltaLat) {
    int iResult = -1;
 
    char *p = strtok(pDelta, "x");
    if (p != NULL) {
         if (strToNum(p, pdDeltaLon)) {
            p = strtok(NULL, "x");
            if (p != NULL) {
                if (strToNum(p, pdDeltaLat)) {
                    iResult = 0;
                }
            }
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// interpolateValue 
//
double interpolateValue(double dLon, double dLat, Icosahedron *pIco, nodelist &mNodeValues) {
    double dVal = dNaN;

    VertexLinkage *pVL = pIco->getLinkage();

    // find containing face of current lon/lat pair
    Vec3D v(cos(dLat)*cos(dLon),
            cos(dLat)*sin(dLon),
            sin(dLat));
    
    IcoFace *pF = NULL;
    for (int u = 0; (pF == NULL) && (u < 20); u++) {
        pF = pIco->getFace(u)->contains(&v);
    }
    if (pF != NULL) {
        // calclate barycentric coordinates of point
        double dL[3];
        pF->calcBary(v, dL, dL+1);
        dL[2] = 1-dL[0]-dL[1];
        //        printf("(%f,%f) ", dLon, dLat);
        // do the interpolation
        dVal = 0;
        for (int k = 0; k < 3; k++) {
            //            gridtype lID = pVL->getVertexID(pF->getVertex(k));
            gridtype lID = pF->getVertexID(k);
            if (lID < 0) {
                printf("vertex has no id!\n");
            }
            nodelist::const_iterator it = mNodeValues.find(lID);
            if (it != mNodeValues.end()) {
                dVal += dL[k]*it->second;
                //                printf("have ID %lld(%f->%f) ", lID,it->second,dVal);
            } else {
                dVal = dNaN;
                printf("missing ID %lld\n", lID);
            } 
        }
        
        
    } else {
        printf("Point [%f,%f]=(%f,%f,%f) not in face\n", dLon, dLat, v.m_fX, v.m_fY, v.m_fZ);
    }
    return dVal;
}
    
//-----------------------------------------------------------------------------
// interpolateValue 
//
double interpolateValueLand(double dLon, double dLat, Icosahedron *pIco, nodelist &mNodeValues) {
    double dVal = dNaN;

    VertexLinkage *pVL = pIco->getLinkage();

    // find containing face of current lon/lat pair
    Vec3D v(cos(dLat)*cos(dLon),
            cos(dLat)*sin(dLon),
            sin(dLat));
    
    IcoFace *pF = NULL;
    for (int u = 0; (pF == NULL) && (u < 20); u++) {
        pF = pIco->getFace(u)->contains(&v);
    }
    if (pF != NULL) {
        dVal = 0;
        // calclate barycentric coordinates of point
        double dL[3];
        pF->calcBary(v, dL, dL+1);
        dL[2] = 1-dL[0]-dL[1];
        double dDiv = 0;
        for (int k = 0; k < 3; k++) {
            //            gridtype lID = pVL->getVertexID(pF->getVertex(k));
            gridtype lID = pF->getVertexID(k);
            if (lID < 0) {
                printf("vertex has no id!\n");
            }
            nodelist::const_iterator it = mNodeValues.find(lID);
            if (it != mNodeValues.end()) {
                if (it->second > 0) {
                    dVal += dL[k]*it->second;
                    dDiv += dL[k];
                }
                //                printf("have ID %lld(%f->%f) ", lID,it->second,dVal);
            } else {
                dVal = dNaN;
                printf("missing ID %lld\n", lID);
            } 
        }

        if (dDiv == 0) {
            dVal = 0;
        } else {
            dVal /= dDiv;
        }
        
        
    } else {
        printf("Point [%f,%f]=(%f,%f,%f) not in face\n", dLon, dLat, v.m_fX, v.m_fY, v.m_fZ);
    }
    return dVal;
}
    
    
//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    char sIcoFile[512];
    char sSnapFile[512];
    char sOutputFile[512];
    char sRange[256];
    char sDelta[256];
    char sSize[256];

    *sIcoFile    = '\0';
    *sSnapFile   = '\0';
    *sOutputFile = '\0';
    *sDelta      = '\0';
    *sSize       = '\0';

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(6,  
                               "-i:s!",    sIcoFile,
                               "-d:s!",    sSnapFile,
                               "-o:s!",    sOutputFile,
                               "-R:s",     sRange,
                               "-r:s",     sDelta,
                               "-s:s",     sSize);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if ((*sDelta != '\0') || (*sSize != '\0')) {
                double dLonMin;
                double dLonMax;
                double dLatMin;
                double dLatMax;
                double dDeltaLon;
                double dDeltaLat;
                int iW;
                int iH;

                iResult = splitRange(sRange, &dLonMin, &dLonMax, &dLatMin, &dLatMax);
                if (iResult == 0) {
                    if (*sSize != '\0') {
                        iResult = splitSizeString(sSize, &iW, &iH);
                        if (iResult == 0) {
                            // calc deltas
                            dDeltaLon = (dLonMax - dLonMin)/iW;
                            dDeltaLat = (dLatMax - dLatMin)/iH;
                        } else {
                            printf("Bad size string\n");
                        }
                    } else {
                        iResult = splitDelta(sDelta, &dDeltaLon, &dDeltaLat);
                        if (iResult == 0) {
                            // calc size
                            iW = (dLonMax - dLonMin)/dDeltaLon;
                            iH = (dLatMax - dLatMin)/dDeltaLat;
                        } else {
                            printf("Bad delta string\n");
                        }
                    }
                    if (iResult == 0) {
                        printf("[%f,%f]%f\n", dLonMin, dLonMax, dDeltaLon);
                        printf("[%f,%f]%f\n", dLatMin, dLatMax, dDeltaLat);
                        dLonMin *= M_PI/180;
                        dLonMax *= M_PI/180;
                        dLatMin *= M_PI/180;
                        dLatMax *= M_PI/180;
                        dDeltaLon *= M_PI/180;
                        dDeltaLat *= M_PI/180;
                        Icosahedron  *pIco = Icosahedron::create(1);
                        pIco->setStrict(true);
                        pIco->setPreSel(false);
                        iResult = pIco->load(sIcoFile);
                        if (iResult == 0) {
                            pIco->relink();
                            nodelist mNodeValues;
                            mNodeValues.clear();
                            double dMinData;
                            double dMaxData;
                            iResult = NodeLister::createList(sSnapFile, mNodeValues, &dMinData, &dMaxData);
                            if (iResult == 0) {
                                FILE *fOut = fopen(sOutputFile, "wt");
                                if (fOut != NULL) {
                                    printf("Now do the calcs\n");
                                    double dLat = dLatMin;
                                    fprintf(stdout/*fOut*/, "# Latitudes ");
                                    while (dLat < dLatMax) {
                                        fprintf(stdout/*fOut*/, "%f ", dLat*180/M_PI);
                                        dLat += dDeltaLat;
                                    }
                                    fprintf(stdout/*fOut*/, "\n");
                                    double dLon = dLonMin;
                                    fprintf(stdout/*fOut*/, "# Longitudes ");
                                    while (dLon < dLonMax) {
                                        fprintf(stdout/*fOut*/, "%f ", dLon*180/M_PI);
                                        dLon += dDeltaLon;
                                    }
                                    fprintf(stdout/*fOut*/, "\n");

                                    dLat = dLatMin;
                                    while (dLat < dLatMax) {
                                        double dLon = dLonMin;
                                        while (dLon < dLonMax) {
                                            double dVal = interpolateValueLand(dLon, dLat, pIco, mNodeValues);
                                            fprintf(fOut, "%e ", dVal);
                                            dLon += dDeltaLon;
                                        }
                                        fprintf(fOut, "\n");
                                        dLat += dDeltaLat;
                                    }
                                    fclose(fOut);
                                } else {
                                    printf("Coudn't open output file [%s]\n", sOutputFile);
                                }
                            } else {
                                printf("Couldn't load snap file [%s]\n", sSnapFile);
                            }
                        } else {
                            printf("Couldn't load ico file [%s]\n", sIcoFile);
                        }
                    }

                } else {
                    printf("Bad range string\n");
                }
            } else {
                printf("One of '-r' and '-s' must be specified\n");
                usage(apArgV[0]);
            }
        } else {
            usage(apArgV[0]);
        }
    } else {
        printf("Error in setOptions\n");
    }

    return iResult;
}

