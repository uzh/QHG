#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>

#include "LineReader.h"
#include "trackball.h"

#include "interquat.h"
#include "QSpline.h"

typedef std::vector<float *> fvv;

float *loadQuats(char *pQuatFile, int *piN) {
    float *f1;
    LineReader *pLR = LineReader_std::createInstance(pQuatFile, "rt");
    if (pLR != NULL) {
        char sIn[512];
        char sPNG[512];
        *sIn = '\0';
        *sPNG = '\0';
        float *qCur=NULL;
        fvv vQ;
        char *p0 =  pLR->getNextLine();
        pLR->seek(0, SEEK_SET);
        int iMode = -1;
        qCur = new float[4];
        int iRead =   sscanf(p0, "%f %f %f %f", &(qCur[0]), &(qCur[1]), &(qCur[2]), &(qCur[3]));
        printf("test1 read %d\n", iRead);
        if (iRead == 4) {
            iMode = 0;
        } else {
            iRead = sscanf(p0, "%s %s %f %f %f %f",sIn, sPNG, &(qCur[0]), &(qCur[1]), &(qCur[2]), &(qCur[3]));
            printf("test2 read %d\n", iRead);
            if (iRead == 6) {
                iMode = 1;
            }
        }
        if (iMode >= 0) {
            printf("Have mode %d\n", iMode);
            while (!pLR->isEoF()) { 
                char *p = pLR->getNextLine();
                if (p != NULL) {
                    qCur = new float[4];
                    if (iMode == 0) {
                        iRead = sscanf(p, "%f %f %f %f", &(qCur[0]), &(qCur[1]), &(qCur[2]), &(qCur[3]));
                    } else {
                        iRead = sscanf(p, "%s %s %f %f %f %f", sIn, sPNG, &(qCur[0]), &(qCur[1]), &(qCur[2]), &(qCur[3]));
                    }
                    printf("read %d\n", iRead);
                    printf("got [%s][%s], %f %f %f %f\n", sIn, sPNG, qCur[0], qCur[1], qCur[2], qCur[3]);
                    vQ.push_back(qCur);
                }
            }
            *piN = vQ.size();
            f1 = new float[vQ.size()*4];
            float *p = f1;
            for (unsigned int i = 0; i < vQ.size(); i++) { 
                printf("copying %f %f %f %f\n", vQ[i][0], vQ[i][1], vQ[i][2], vQ[i][3]);
                memcpy(p, vQ[i], 4*sizeof(float));
                printf("check %f %f %f %f\n", p[0], p[1], p[2], p[3]);
                p+=4;
            }
        } else {
            printf("Unknown line format - only know (%%s %%s %%f %%f %%f %%f) or (%%f %%f %%f %%f)\n");
        }

        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pQuatFile);
    }
    return f1;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    float *f1=NULL;
    int iN=0;
    bool bEqualize = false;
    int iSamp = 0;
    if (iArgC > 1) {
        iSamp  = atoi(apArgV[1]);
        printf("firt check of samp:%d\n",iSamp);
        if (iSamp == 0) {
            f1 = loadQuats(apArgV[1],&iN);
            printf("Quatfile %s\n", apArgV[1]);
        }
        if (iArgC > 2) {
            iSamp = atoi(apArgV[2]);
        }
        if (iSamp > 0) {
            bEqualize = true;
        }
    }
    if (f1 == NULL) {
        printf("Using builtin quats\n");
        // 2 quats:
        // rot 0    around (1,0,0) : (0, 0, 0, 1) 
        // rot pi/2 around (1,0,0) : (1, 0, 0, 0)
        float a1[3];
        a1[0] = 1;
        a1[1] = 0;
        a1[2] = 0;
        float phi00=0;
        float phi01=M_PI/2;
        float phi02=M_PI/2;
        
        f1 = new float[12];
 
        axis_to_quat(a1, phi00, f1);
        a1[0] = 1;
        a1[1] = 0;
        a1[2] = 0;
        axis_to_quat(a1, phi01, f1+4);
        a1[0] = 0;
        a1[1] = 0;
        a1[2] = 1;
                axis_to_quat(a1, phi02, f1+8);

        iN = 3;
        qshow(f1);
        qshow(f1+4);
        qshow(f1+8);
        printf("\n");        
    }
    printf("Equalize ");
    if (bEqualize) {
        printf("on, samp %d\n", iSamp);
    } else {
        printf("off\n");
    }
    printf("Have %d quats\n", iN);
    for (int i = 0; i < iN; i++) {
        printf("  ");qshow(f1+i*4);
    }
    printf("-----------------\n");
    QSpline *pQS1 = new QSpline(f1, iN, bEqualize, iSamp);

    float qPrev[4];
    pQS1->getValue(0.0, qPrev);
    for (int i = 0; i < 4*iN; i++) {
        float qCur1[4];
        pQS1->getValue((1.0*i)/(4*iN), qCur1);
        printf("%d: ",i);qshow(qCur1);
        if (i > 0) {
            double dProd = 1.0*qPrev[0]*qCur1[0]+qPrev[1]*qCur1[1]+qPrev[2]*qCur1[2]+qPrev[3]*qCur1[3];
            if (dProd > 1) {
                dProd = 1;
            } else if (dProd < -1) {
                dProd = -1;
            }
            double d = acos(dProd);
            printf("%d: Dist %f\n", i, d);
        }
        memcpy(qPrev, qCur1, 4*sizeof(float));
    }
    float qCur1[4];
    pQS1->getValue(1.0, qCur1);
    qshow(qCur1);


    delete pQS1;
    

    return iResult;
}
