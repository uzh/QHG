#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "trackball.h"
#include "interquat.h"

#include "QSpline.h"
static bool s_bVerbose = false;

#define NSUB 16
//----------------------------------------------------------------------------
// constructor
//
QSpline::QSpline(float *pqIn, int iN, int iEQMethod, int iSamples) 
    : m_pqPoints(pqIn),
      m_iN(iN),
      m_pqInter(NULL),
      m_bEqualize((iEQMethod > 0) && (iSamples > 0)),
      m_iSamples(iSamples), 
      m_afTable(NULL) {

    // allocate
    m_pqInter  = new float [4*m_iN];
    bzero(m_pqInter, 4*m_iN*sizeof(float));
    

 
    // calculate intermediates
    // if we set q_-1 = q_0, log (q_0^-1.q_1) = 0
          
    for (int i = 0; i < m_iN; i++) {
        int iNext = (i<m_iN-1)?i+1:m_iN-1;
        int iPrev = (i>0)?i-1:0;

        if (s_bVerbose) {
            printf("--- step #%d ---\n", i);
            printf("quat prev: ");
            qshow(m_pqPoints+4*iPrev);
            printf("quat cur: ");
            qshow(m_pqPoints+4*i);
            printf("quat next: ");
            qshow(m_pqPoints+4*iNext);
        }

        float fA[4];
        qinvert(m_pqPoints+4*i, fA);
        normalize_quat(fA);
        if (s_bVerbose) {
            printf("inv : ");
            qshow(fA);
        }

        float fB1[4];
        add_quats(m_pqPoints+4*iNext, fA, fB1);
        normalize_quat(fB1);
        if (s_bVerbose) {
            printf("prod1 : ");
            qshow(fB1);
        }

        float fL1[3];
        qlog(fB1, fL1);
        if (s_bVerbose) {
            printf("log1 : ");
            ushow(fL1);
        }

        float fB2[4];
        add_quats(m_pqPoints+4*iPrev, fA, fB2);
        normalize_quat(fB2);
        if (s_bVerbose) {
            printf("prod2 : ");
            qshow(fB2);
        }

        float fL2[3];
        qlog(fB2, fL2);
        if (s_bVerbose) {
            printf("log2 : ");
            ushow(fL2);
        }


        float fC[3];
        for (int j = 0; j < 3; j++) {
            fC[j] =-(fL1[j]+fL2[j])/4;
        }
        if (s_bVerbose) {
            printf("sum : ");
            ushow(fC);
        }
        float fQ[4];
        qexp(fC, fQ);
        normalize_quat(fQ);
        if (s_bVerbose) {
            printf("exp : ");
            qshow(fQ);
        }
        add_quats(fQ, m_pqPoints+4*i, m_pqInter+4*i);
        normalize_quat(m_pqInter+4*i);

        if (s_bVerbose) {
            printf("inter #%d: ", i);
            qshow(m_pqInter+4*i);
        }

    }
  
    if (m_bEqualize) {
        printf("Equalizing for %d samples....\n", m_iSamples);
        int iT=-1;
        switch (iEQMethod) {
        case 1: 
            iT = equalizeV();
            break;
        case 2: 
            iT = equalizeQ();
            break;
        case 3: 
            iT = equalizeQ2();
            break;
        }
        printf("-> %d table entries\n", iT);
        /*
        if (s_bVerbose) {
            for (int i = 0; i < iT; i++) {
                printf("  %d: %f\n", i, m_afTable[i]);
            }
        }
        */
    }
}

//----------------------------------------------------------------------------
// destructor
//
QSpline::~QSpline() {
    if (m_pqInter != NULL) {
        delete[] m_pqInter;
    }
    if (m_afTable != NULL) {
        delete[] m_afTable;
    }
}

    
//----------------------------------------------------------------------------
// getValue
//
void QSpline::getValue(int n, float t, float qCur[4]) {

    squad(t, m_pqPoints+4*n, m_pqInter+4*n, m_pqInter+4*(n+1), m_pqPoints+4*(n+1), qCur);
}

#define EPS 1e-6
//----------------------------------------------------------------------------
// getValue
//
void QSpline::getValue(float t, float qCur[4]) {
    if (m_bEqualize) {
        int i = t*m_iSamples;
        t = m_afTable[i];
    }
    // here we could use an inverse mapping for constant velocity
    float fScaled = t*(m_iN-1);
    int iIntervalNo = (int) fScaled;
    float t0 = fScaled-iIntervalNo;
    if (t > 1) {
          printf("t %f -> interval %d/%d, sub r %f\n", t, iIntervalNo, m_iN, t0);
    }
    getValue(iIntervalNo, t0, qCur);
}


//----------------------------------------------------------------------------
// equalizeV
//
int QSpline::equalizeV() {
    float dT = 1.0/(NSUB*m_iSamples);
    
    // temporariliy disable equalize
    m_bEqualize = false;
    double dDist = 0;
    
    float axPrev0[]={0,0,1};
    float axPrev[3];
    float qPrev[4];
    getValue(0, qPrev);

    qapply(qPrev, axPrev0, axPrev);
    int iNC0 = 0;
    for (int i = 0; i <= NSUB*m_iSamples; i++) {
        float qCur[4];
        printf("getting value for %d*%f = , %f\n", i, dT, i*dT);
        
        getValue(i*dT, qCur);
        if (isnan(qCur[0])) {
            printf("nan quat at %d, T%f, %f\n", i, i*dT,dT);
            
            iNC0++;
            getValue(i*dT, qCur);
            printf("second try qCur[0] %f\n", qCur[0]);
        }
        float axCur[3];
        
        qapply(qCur, axPrev0, axCur);
        
        double dProd = 1.0*axPrev[0]*axCur[0]+axPrev[1]*axCur[1]+axPrev[2]*axCur[2];
        if (dProd > 1) {
            dProd = 1;
        } else if (dProd < -1) {
            dProd = -1;
        }
        double d = acos(dProd);
       if (isnan(d)) {
           /*
           //           printf("nan at %d, T%f, after dDist %f:\n", i, i*dT, dDist);
           qshow(qPrev);
           qshow(qCur);
           */
        } else {
           //        printf("dist((%f,%f,%f),(%f,%f,%f))=%e (prod-1: %e)\n",  axPrev[0], axPrev[1], axPrev[2],axCur[0], axCur[1], axCur[2], d, dProd-1); 
        dDist+=d;
       }
        memcpy(axPrev, axCur, 3*sizeof(float));
        memcpy(qPrev, qCur, 4*sizeof(float));

    }
    printf("Total Dist is %f\n", dDist);
    printf("AvgStep %f\n", dDist/(NSUB*m_iSamples));
    printf("Num nan quats %d\n", iNC0);
    sleep(5);

    m_afTable = new float[m_iSamples+1];
    printf("Allocating table (%d): %p\n", m_iSamples+1, m_afTable);
    int iC = 0;
    double dDeltaS = dDist/m_iSamples;
    printf("Filling table with deltaS %f ...\n", dDeltaS);
    getValue(0, qPrev);
    qapply(qPrev, axPrev0, axPrev);

    double dS = 0;
    double dDist1 = 0;
    int i = 0;
    double dDistPrev=0;
    while (dS <= dDist) {
        int iNC = 0;
        while (dDist1 < dS) {
            float qCur[4];
            getValue(i*dT, qCur);
            if (isnan(qCur[0])) {
                printf("nan quat second loop at %d, T%f,%f\n", i, i*dT, dT);
                iNC++; 
            }
            float axCur[3];
            qapply(qCur, axPrev0, axCur);

            double dProd = 1.0*axPrev[0]*axCur[0]+axPrev[1]*axCur[1]+axPrev[2]*axCur[2];
            if (dProd > 1) {
                dProd = 1;
            } else if (dProd < -1) {
                dProd = -1;
            }
            double d = acos(dProd);

            dDist1+=d;
            i++;
            memcpy(axPrev, axCur, 3*sizeof(float));

        }
        if (dDist1 >= dS) {
            //   printf("%f >= %f:  %d -> %f=%d*%f, nans: %d, dist to prev %f\n", dDist1, dS, iC, i*dT, i , dT, iNC, dDist1-dDistPrev); 
            
            m_afTable[iC++] = i*dT;
           
            dS += dDeltaS;
        }

    }
    printf("  %d table entries\n", iC);  
    // turn equalize back on
    m_bEqualize = true;
    // m_afTable[iC++] =1;
    return iC;
}

//----------------------------------------------------------------------------
// equalizeQ
//
int QSpline::equalizeQ() {
    float dT = 1.0/(NSUB*m_iSamples);
    
    // temporariliy disable equalize
    m_bEqualize = false;
    double dDist = 0;
    
    float qPrev[4];
    getValue(0, qPrev);

    double dMin = 1e9;
    double dMax = -1;
    printf("For %d samples -> dT=%f\n", m_iSamples, dT);
    for (int i = 0; i <= NSUB*m_iSamples; i++) {
        float qCur[4];
        getValue(i*dT, qCur);
        
        double dProd = 1.0*qPrev[0]*qCur[0]+qPrev[1]*qCur[1]+qPrev[2]*qCur[2]+qPrev[3]*qCur[3];
        if (dProd > 1) {
            dProd = 1;
        } else if (dProd < -1) {
            dProd = -1;
        }
        double d = acos(dProd);
        if (isnan(d)) {
            printf("nan at %d, T%f:\n", i, i*dT);
            qshow(qPrev);
            qshow(qCur);
        } else {
            //            printf("%d, T%f - d %f:\n", i, i*dT, d);
            dDist+=d;
            if (d < dMin) {
                dMin = d;
            }
            if (d > dMax) {
                dMax = d;
            }
            //            printf("subdist %d: %f (tot %f)\n", i, d, dDist); 
        }
        memcpy(qPrev, qCur, 4*sizeof(float));
    }
    printf("Total Dist is %f\n", dDist);
    printf("avg dist : %f, min %f, nax %f\n", dDist/(NSUB*m_iSamples), dMin, dMax);
    sleep(5);
    m_afTable = new float[m_iSamples+1];
    printf("Allocating table (%d): %p\n", m_iSamples+1, m_afTable);
    int iC = 0;
    double dDeltaS = dDist/m_iSamples;
  
    printf("Filling table with deltaS %f ...\n", dDeltaS);
    getValue(0, qPrev);
    double dS = 0;
    double dDist1 = 0;

    int i = 0;
    int iNN = 0;
    double dDistPrev=0;
    while (dS < dDist) {
        int iNC = 0;
        int iSubSteps = 0;
        while (dDist1 < dS) {
            float qCur[4];
            getValue(i*dT, qCur);

            double dProd = 1.0*qPrev[0]*qCur[0]+qPrev[1]*qCur[1]+qPrev[2]*qCur[2]+qPrev[3]*qCur[3];
            if (dProd > 1) {
                dProd = 1;
            } else if (dProd < -1) {
                dProd = -1;
            }
            double d = acos(dProd);
            if (isnan(d)) {
                // ignore
                iNC++;
            } else {
                dDist1+=d;
            }
            i++;
            iSubSteps++;
            memcpy(qPrev, qCur, 4*sizeof(float));

        }
        if (dDist1 >= dS) {
            //      printf("%f >= %f:  %d -> %f=%d*%f, nans: %d, dist to prev %f,substeps %d\n", dDist1, dS, iC, i*dT, i , dT, iNC, dDist1-dDistPrev,iSubSteps); 
            if (dDist1==dDistPrev) {
                iNN++;
            }
            dDistPrev=dDist1;
            m_afTable[iC++] = i*dT;
            dS += dDeltaS;
        }

    }
    printf("  %d table entries (%d null dists)\n", iC, iNN);  
    // turn equalize back on
    m_bEqualize = true;
    return iC;
}

//----------------------------------------------------------------------------
// equalizeQ2
//
int QSpline::equalizeQ2() {
    float dT = 1.0/(NSUB*m_iSamples);
    
    // temporariliy disable equalize
    m_bEqualize = false;
    double dDist = 0;
    
    float qPrev[4];
    float qPrevInv[4];
    getValue(0, qPrevInv);
    qinvert(qPrev, qPrevInv);

    double dMin = 1e9;
    double dMax = -1;
    printf("For %d samples -> dT=%f\n", m_iSamples, dT);
    for (int i = 0; i <= NSUB*m_iSamples; i++) {
        float qCur[4];
        float qR[4];
        getValue(i*dT, qCur);
        add_quats(qCur, qPrevInv, qR);

        
        double dProd = qR[3];
        if (dProd > 1) {
            dProd = 1;
        } else if (dProd < -1) {
            dProd = -1;
        }
        double d = acos(dProd);
        if (isnan(d)) {
            printf("nan at %d, T%f:\n", i, i*dT);
            qshow(qPrevInv);
            qshow(qCur);
        } else {
            //            printf("%d, T%f - d %f:\n", i, i*dT, d);
            dDist+=d;
            if (d < dMin) {
                dMin = d;
            }
            if (d > dMax) {
                dMax = d;
            }
            //            printf("subdist %d: %f (tot %f)\n", i, d, dDist); 
        }
        qinvert(qCur, qPrevInv);
    }
    printf("Total Dist is %f\n", dDist);
    printf("avg dist : %f, min %f, nax %f\n", dDist/(NSUB*m_iSamples), dMin, dMax);
    sleep(5);
    m_afTable = new float[m_iSamples+1];
    printf("Allocating table (%d): %p\n", m_iSamples+1, m_afTable);
    int iC = 0;
    double dDeltaS = dDist/m_iSamples;
  
    printf("Filling table with deltaS %f ...\n", dDeltaS);
    qinvert(qPrev, qPrevInv);
    double dS = 0;
    double dDist1 = 0;

    int i = 0;
    int iNN = 0;
    double dDistPrev=0;
    while (dS < dDist) {
        int iNC = 0;
        int iSubSteps = 0;
        while (dDist1 < dS) {
            float qCur[4];
            float qR[4];
            getValue(i*dT, qCur);
            add_quats(qCur, qPrevInv, qR);

            double dProd = qR[3];
            if (dProd > 1) {
                dProd = 1;
            } else if (dProd < -1) {
                dProd = -1;
            }
            double d = acos(dProd);
            if (isnan(d)) {
                // ignore
                iNC++;
            } else {
                dDist1+=d;
            }
            i++;
            iSubSteps++;
            qinvert(qCur, qPrevInv);

        }
        if (dDist1 >= dS) {
            //      printf("%f >= %f:  %d -> %f=%d*%f, nans: %d, dist to prev %f,substeps %d\n", dDist1, dS, iC, i*dT, i , dT, iNC, dDist1-dDistPrev,iSubSteps); 
            if (dDist1==dDistPrev) {
                iNN++;
            }
            dDistPrev=dDist1;
            m_afTable[iC++] = i*dT;
            dS += dDeltaS;
        }

    }
    printf("  %d table entries (%d null dists)\n", iC, iNN);  
    // turn equalize back on
    m_bEqualize = true;
    return iC;
}

