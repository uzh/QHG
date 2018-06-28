#include <gtkmm.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "QSpline.h"
#include "trackball.h"
#include "LineReader.h"
#include "ProjInfo.h"
#include "SnapQuatProjector.h"


//-----------------------------------------------------------------------------
//  usage
//
void usage(char *pApp) {
    printf("%s - creating animation file for IQApp\n", pApp);
    printf("Usage:\n");
    printf("  %s <quatfile> <snapbody> <minstep>:<maxstep>:<delta>:<sub>[:<EqMethod>]\n", pApp);
    printf("     <PNGbody> <outputfile>\n");
    printf("where\n");
    printf("  quatfile       textfile containig quaternions (1 per line)\n");
    printf("                    qx qy qz qw\n");
    printf("                 (should probably be sapced equally over time\n");
    printf("  snapbody       body of snap input files\n");
    printf("  minstep        number of first snapfile to use\n");
    printf("  maxstep        number of last snapfile to use\n");
    printf("  delta          interval between snaps\n");
    printf("  sub            interval to use for interpolated images\n");
    printf("  EqMethod       0:direct, 1:Z-Arc equalize, 2:Q-Arc equalize, 3:Q-Rot equalize. default=0\n");
    printf("  PNGbody        body of PNG files\n");
    printf("  outputbody     output file\n");
    printf("\n");
}

//-----------------------------------------------------------------------------
// createQuatArray
//
float *createQuatArray(char *pQuatFile, int *piNumQuats) {
    int iResult = 0;

    float *pfQuats = NULL;
    float q[4];
    LineReader *pLR = LineReader_std::createInstance(pQuatFile, "rt");
    if (pLR != NULL) {
        std::vector<float> vqList;
        while ((iResult == 0) && !pLR->isEoF()) { 
            char *p = pLR->getNextLine();
            if (p != NULL) {
                int iNum = sscanf(p, "%f %f %f %f", &(q[0]), &(q[1]), &(q[2]), &(q[3]));
                if (iNum == 4) {
                    for (int i = 0; i < 4; i++) {
                        vqList.push_back(q[i]);
                    }
                } else {
                    printf("Not enough values on quat file line: [%s]\n", p);
                    iResult = -1;
                }
            }
        }
        if (iResult == 0) {
            *piNumQuats = vqList.size()/4;
            pfQuats = new float[vqList.size()];
            for (unsigned int i =0; i < vqList.size(); i++) {
                pfQuats[i] = vqList[i];
            }
        }
        delete pLR;
    } else {
        printf("Couldn't open quat file for reading: [%s]\n", pQuatFile);
    }
    return pfQuats;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    Gtk::Main kit(iArgC, apArgV);
    //  Gtk::GL::init(iArgC, apArgV);

    if (iArgC > 5) {
        char sQuatFile[512];
        char sSnapBody[512];
        char sPNGBody[512];
        char sOutputFile[512];

        int iMinStep;
        int iMaxStep;
        int iDelta;
        int iSub;
        int iEq;
        bool bEqualize = false;

      
        strcpy(sQuatFile, apArgV[1]);
        strcpy(sSnapBody, apArgV[2]);
        strcpy(sPNGBody, apArgV[4]);
        strcpy(sOutputFile, apArgV[5]);
        
        iResult = 0;
        int iNum = sscanf(apArgV[3], "%d:%d:%d:%d:%d", &iMinStep, &iMaxStep, &iDelta, &iSub, &iEq);
        if (iNum >= 4) {
            if (iNum > 4) {
                bEqualize = (iEq != 0);
                printf("Equalize: %s\n", bEqualize?"On":"Off");
            }
        } else {
            printf("Invalid minval/maxval/delta: [%s]\n", apArgV[3]);
        }


        if (iResult == 0) {
                 
                

            int iNumQuats;
            float *pfQuats = createQuatArray(sQuatFile, &iNumQuats);
            if (pfQuats != NULL) {
                float *p1 = pfQuats;
                for (int i = 0; i < iNumQuats; i++) {
                    normalize_quat(p1);
                    p1+=4;
                }

                printf("created quat array for %d quats\n", iNumQuats);
                QSpline *pQS = new QSpline(pfQuats, iNumQuats, bEqualize, 256*( 1+(iMaxStep-iMinStep)/iSub));

               
                char sCurSnap[512];
                char sPNGFile[512];
                int iNextStep = iMinStep;
                FILE *fOut = fopen(sOutputFile, "wt");

                for (int iStep = iMinStep; (iResult == 0) && (iStep <= iMaxStep); iStep+=iSub) {
                    float tL = (1.0*iStep)/iMaxStep;
                    float qCur[4];
                    pQS->getValue(tL, qCur);
                    normalize_quat(qCur);
                    //                        printf("----- setting new quat [%f %f %f %f]\n", qCur[0], qCur[1], qCur[2], qCur[3]);
                    // load a new snap if necessary
                        
                    if (iStep == iNextStep) {
                        sprintf(sCurSnap, "%s%04d.snap", sSnapBody, iStep);
                        //                            printf("----- setting new snap [%s]\n", sCurSnap);
                        iNextStep = iStep+iDelta;
                    }
                    if (iResult == 0) {
                        sprintf(sPNGFile, "%s_%04d.png", sPNGBody, iStep);
                        fprintf(fOut, "%s %s %f %f %f %f \n", sCurSnap,  sPNGFile, qCur[0], qCur[1], qCur[2], qCur[3]);
                        //  printf( "t:%e %s %s %f %f %f %f \n", tL, sCurSnap,  sPNGFile, qCur[0], qCur[1], qCur[2], qCur[3]);
                        /*
                          sprintf(sOutputFile, "%s_%04d.png", sOutputBody, iStep);
                          printf("----- drawing to output file [%s]\n", sOutputFile);
                          iResult = pSQP->drawProjection(sOutputFile);
                        */
                            
                    }
                }
                fclose(fOut);


                printf("Finding timestamps for input quats\n");
                // finding timestamps for input quats
                float t = 0;
                float dt = 1.0/(4*iMaxStep);
                
                double *atMin = new double[iNumQuats];
                double *adMin = new double[iNumQuats];
                for (int i = 0; i < iNumQuats; i++) {
                    adMin[i] = 1e9;
                }

                int iShow=-1;
                while (t <= 1+dt) {
                    float qCur[4];
                    pQS->getValue(t, qCur);
                    float *p = pfQuats;
                    normalize_quat(qCur);
                    for (int i = 0; i < iNumQuats; i++) {
                        double dP;
                        double dP0 = 1.0*p[0]*qCur[0] + 1.0*p[1]*qCur[1] + 1.0*p[2]*qCur[2] + 1.0*p[3]*qCur[3];
                        if (dP0 > 1) {
                            dP = 1-1.1*(dP0-1);
                        } else if (dP0 < -1) {
                            dP = -1+1.1*(dP0+1);
                        } else {
                            dP = dP0;
                        }
                        double d =2*asin(sqrt((1-dP)/2)); // better way for acos(1-x), where x = 1-dP;
                        if ((i == iShow) && (t < 0.0001)) printf("%d:check %e@%f (%e->%e) for  (%f,%f,%f,%f): (%f,%f,%f,%f) \n", i, d, t, 1-dP0,1-dP,  p[0], p[1], p[2], p[3], qCur[0], qCur[1], qCur[2], qCur[3]);
                        if (fabs(d) < adMin[i]) {
                            if (i == iShow) {
                                        printf("%d: new mind %e@%f (%e->%e) for  (%f,%f,%f,%f): (%f,%f,%f,%f) \n", i, d, t, 1-dP0,1-dP,  p[0], p[1], p[2], p[3], qCur[0], qCur[1], qCur[2], qCur[3]);
                            }
                            adMin[i] = fabs(d);
                            atMin[i] = t; 
                        }
                        p += 4;
                    }                    
                    t += dt;
                }
                for (int i = 0; i < iNumQuats; i++) {
                    printf("Quat %d (%f,%f,%f,%f) : t=%f, d=%f\n", i, pfQuats[4*i], pfQuats[4*i+1], pfQuats[4*i+2], pfQuats[4*i+3], atMin[i], adMin[i]);
                }
                delete[] atMin;
                delete[] adMin;

                delete pQS;
            } else {
                printf("Couldn't create Quat list from quat file [%s]\n", sQuatFile);
            }
        }
        
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
