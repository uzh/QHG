#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "ValReader.h"
#include "QMapUtils.h"
#include "Vec3D.h"
#include "Quat.h"
#include "IcoFace.h"
#include "EQTriangle.h"
#include "EQsahedron.h"

void testinv(EQsahedron *pEQS, Vec3D *pV) {
    printf("\n");
    printf("Have original vector [% f % f % f]\n", pV->m_fX, pV->m_fY, pV->m_fZ);
    pV->normalize();
    printf("Have normalized      [% f % f % f]\n", pV->m_fX, pV->m_fY, pV->m_fZ);
    printf("finding node\n");
    gridtype lID1 = pEQS->findNodeSlow(pV);
    printf("method1: %lld\n", lID1);
    gridtype lID2 = pEQS->findNode(pV);
    printf("method2: %lld\n", lID2);
    /* for the moement
    printf("finding face\n");
    
    double dLat = asin(pV->m_fZ);
    double dLon = atan2(pV->m_fY,pV->m_fX);

    PolyFace *pF1 = pEQS->findFaceSlow(dLon, dLat);
    printf("method1F: %p (% f % f % f)(% f % f % f)(% f % f % f)\n", pF1,
           pF1->getVertex(0)->m_fX, pF1->getVertex(0)->m_fY, pF1->getVertex(0)->m_fZ,
           pF1->getVertex(1)->m_fX, pF1->getVertex(1)->m_fY, pF1->getVertex(1)->m_fZ,
           pF1->getVertex(2)->m_fX, pF1->getVertex(2)->m_fY, pF1->getVertex(2)->m_fZ);

    PolyFace *pF2 = pEQS->findFace(dLon, dLat);
    printf("have face %p\n", pF2);

    printf("method2F: %p (% f % f % f)(% f % f % f)(% f % f % f)\n", pF2,
           pF2->getVertex(0)->m_fX, pF2->getVertex(0)->m_fY, pF2->getVertex(0)->m_fZ,
           pF2->getVertex(1)->m_fX, pF2->getVertex(1)->m_fY, pF2->getVertex(1)->m_fZ,
           pF2->getVertex(2)->m_fX, pF2->getVertex(2)->m_fY, pF2->getVertex(2)->m_fZ);
    */
}


void many1(EQsahedron *pEQS, Vec3D **av, int iN, int iRepeats) {
    for (int j = 0; j < iRepeats; j++) {
    for (int i = 0; i < iN; i++) {
        pEQS->findNodeSlow(av[i]);
    }
    }
}
void many2(EQsahedron *pEQS, Vec3D **av, int iN, int iRepeats) {
    for (int j = 0; j < iRepeats; j++) {
    for (int i = 0; i < iN; i++) {
        pEQS->findNode(av[i]);
    }
    }
}
void many1F(EQsahedron *pEQS, Vec3D **av, int iN, int iRepeats) {
    for (int j = 0; j < iRepeats; j++) {
    for (int i = 0; i < iN; i++) {
        double dLat = asin(av[i]->m_fZ);
        double dLon = atan2(av[i]->m_fY,av[i]->m_fX);

        pEQS->findFaceSlow(dLon, dLat);
    }
    }
}
void many2F(EQsahedron *pEQS, Vec3D **av, int iN, int iRepeats) {
    for (int j = 0; j < iRepeats; j++) {
    for (int i = 0; i < iN; i++) {
        double dLat = asin(av[i]->m_fZ);
        double dLon = atan2(av[i]->m_fY,av[i]->m_fX);

        pEQS->findFace(dLon, dLat);
    }
    }
}

int compNode(EQsahedron *pEQS, Vec3D **av, int iN) {
    int iDiff = 0;
    for (int i = 0; i < iN; i++) {
        gridtype l1 = pEQS->findNodeSlow(av[i]);
        gridtype l2 = pEQS->findNode(av[i]);
        if (l1 != l2) {
            iDiff++;
        }
    }
    return iDiff;
}

//----------------------------------------------------
// compFace
//  calls findNode and findNodeSlow for Points in av
//  and reports different results.
//  As it turns out, all misses are on the edges 
//  between two faces. Interestingly none of the 
//  edges leading to a base vertex is ever involved
//  in a miss event...
//
//  Use 
//   ./RITest 2 5000 | grep Miss | gawk -F\( '{ print $2 }' | gawk -F\) '{ print $1 }' > misses.dat
//  to get a datafile splottable by gnuplot
//
int compFace(EQsahedron *pEQS, Vec3D **av, int iN) {
    int iDiff = 0;
    for (int i = 0; i < iN; i++) {
       
        double dLat = asin(av[i]->m_fZ);
        double dLon = atan2(av[i]->m_fY,av[i]->m_fX);

        PolyFace *pF1 = pEQS->findFaceSlow(dLon, dLat);
        PolyFace *pF2 = pEQS->findFace(dLon, dLat);
        if (pF1 != pF2) {
            iDiff++;
            printf("%d: Miss for (%f %f %f) -> %f %f\n", i, av[i]->m_fX, av[i]->m_fY, av[i]->m_fZ, dLon, dLat);
            printf("method1F: %p (% f % f % f)(% f % f % f)(% f % f % f)\n", pF1,
                   pF1->getVertex(0)->m_fX, pF1->getVertex(0)->m_fY, pF1->getVertex(0)->m_fZ,
                   pF1->getVertex(1)->m_fX, pF1->getVertex(1)->m_fY, pF1->getVertex(1)->m_fZ,
                   pF1->getVertex(2)->m_fX, pF1->getVertex(2)->m_fY, pF1->getVertex(2)->m_fZ);
            

            printf("method2F: %p (% f % f % f)(% f % f % f)(% f % f % f)\n", pF2,
                   pF2->getVertex(0)->m_fX, pF2->getVertex(0)->m_fY, pF2->getVertex(0)->m_fZ,
                   pF2->getVertex(1)->m_fX, pF2->getVertex(1)->m_fY, pF2->getVertex(1)->m_fZ,
                   pF2->getVertex(2)->m_fX, pF2->getVertex(2)->m_fY, pF2->getVertex(2)->m_fZ);
            bool bSearchEdge = true;
            int e1 = 0;
            int e2 = -1;
            while (bSearchEdge && (e1 < 3)) {
                for (int j = 0; bSearchEdge && (j < 3); j++) {
                    if (pF1->getVertex(e1) == pF2->getVertex(j)) {
                        if  (pF1->getVertex((e1+1)%3) == pF2->getVertex((j+2)%3)) {
                            bSearchEdge = false;
                            e2 = j;
                            if  (pF1->getVertex((e1+2)%3) == pF2->getVertex((j+1)%3)) {
                                printf("They are equal!?\n");
                            }
                        }
                    }
                }
                e1++;

            }
            e1--;
            if (bSearchEdge) { 
                printf("\e[0;31mNo common edge!\n\e[0m");
            } else {
                printf("common edge: startvertex %d in PF1, %d in pF22\n", e1, e2);

                // If the plane spanned by pF1->getVertex(e1) and pF1->getVertex((e1+1)%3) is the same
                // as the one spanned by  pF1->getVertex(e1) and av, the three points are spherically
                // collinear,

                Vec3D *N1 = pF1->getVertex(e1)->crossProduct(av[i]);
                Vec3D *N2 = pF1->getVertex(e1)->crossProduct(pF1->getVertex((e1+1)%3));
                double dA = N1->getAngle(N2);
                printf("pF1: Angle N0-N1: %f\n", dA);
                delete N1;
                delete N2;
            }
       }
    }
    return iDiff;
}

double rand1() {
    return (2.0*rand())/RAND_MAX - 1;
}

//-----------------------------------------------------------------------------
// makeSpherRand
//   make iN points randomly distributed on the sphere
//
Vec3D **makeSpherRand(int iN) {
    Vec3D **pav = new Vec3D*[iN];
    for (int i = 0; i < iN; i++) {
        // longitude has uniformn distribution
        double dLon = M_PI*rand1();
        // latitude has a asin distribution
        double ds = rand1(); 
        double dc = cos(asin(ds));
        pav[i] = new Vec3D(dc*cos(dLon), dc*sin(dLon), ds); // rand1(), rand1(), rand1());
        pav[i]->normalize();
    }
    return pav;
}
void deleteRand(Vec3D **pav, int iN) {
    if (pav != NULL) {
        for (int i = 0; i < iN; i++) {
            delete pav[i];
        }
        delete[] pav;
    }
}

int mainA(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        int iNumSubDivs = atoi(apArgV[1]);
        clock_t c1 = clock();
        EQsahedron *pEQS = EQsahedron::createInstance(iNumSubDivs,true,NULL,fNaN,NULL);
        clock_t c2 = clock();
        iResult = pEQS->save("test1.ieq");
        printf("now relink\n");
        pEQS->relink();
        
        if (iArgC == 3) {
            int iN = atoi(apArgV[2]);
            Vec3D **pav = makeSpherRand(iN);
            int iDiff1 = compNode(pEQS, pav, iN);
            printf("Node misses: %d\n", iDiff1);
            int iDiff2 = compFace(pEQS, pav, iN);
            printf("Face misses: %d\n", iDiff2);
            deleteRand(pav,iN);
        } else if (iArgC > 3){
            int iN = atoi(apArgV[2]);
            int iR = atoi(apArgV[3]);
            Vec3D **pav = makeSpherRand(iN);
            clock_t c00 = clock();
            many1(pEQS, pav, iN, iR);
            clock_t c01 = clock();
            many2(pEQS, pav, iN, iR);
            clock_t c02 = clock();

            printf("many1 %zd\n", c01-c00);
            printf("many2 %zd\n", c02-c01);

            c00 = clock();
            many1F(pEQS, pav, iN, iR);
            c01 = clock();
            many2F(pEQS, pav, iN, iR);
            c02 = clock();

            printf("many1F %zd\n", c01-c00);
            printf("many2F %zd\n", c02-c01);
            deleteRand(pav, iN);
        } else {
        
            Vec3D vtest1( 0.5286,   -0.1111,   -0.8344); // mix 6:353, 6:355, 6:356 (on 19)
            Vec3D vtest2(0.361803, -0.262866, -0.22360); // on 13
            Vec3D vtest3( 0.6854,  0.1662, -0.7001);  // mix 6:357, 6:360, 6:361 (on 19)
            Vec3D vtest4( 0.796692,  0.350487, -0.492383); // 6:132 (on 19)

            testinv(pEQS, &vtest1);
            testinv(pEQS, &vtest2);
            testinv(pEQS, &vtest3);
            testinv(pEQS, &vtest4);
        }
        /*
        //        pEQS->display();
        iResult = pEQS->save("test1.ieq");
        if (iResult == 0) {
            EQsahedron *pEQS2 = EQsahedron::createEmpty();
            iResult = pEQS2->load("test1.ieq");
            if (iResult == 0) {
                pEQS2->display();
                 iResult = pEQS2->save("test2.ieq");
                 if (iResult == 0) {
                     printf("EQS2 relink\n");
                     pEQS2->relink();
                 } else {
                     printf("Failed to write EQsahedron\n");
                 }
            } else {
                printf("Failed to read EQsahedron\n");
            }
            delete pEQS2;
        } else {
            printf("Failed to write EQsahedron\n");
        }
        */
        clock_t c3 = clock();

        printf("Creation: %f\n", 1.0*(c2-c1)/CLOCKS_PER_SEC);
        printf("Relink:   %f\n", 1.0*(c3-c2)/CLOCKS_PER_SEC);
        delete pEQS;

        printf("sizeof(int): %zd\n", sizeof(int));
        printf("sizeof(long): %zd\n", sizeof(long));
        printf("sizeof(long long): %zd\n", sizeof(long long));
    } else {
        iResult = -1;
        printf("%s <numsubdiv>\n", apArgV[0]);
    }
    return iResult;
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 3) {
        int iNumSubDivs = atoi(apArgV[1]);
        
        
        float dAlt = atof(apArgV[3]);
        
        ValReader *pVR = QMapUtils::createValReader(apArgV[2], true);
        if ((iNumSubDivs > 0) && (pVR != NULL)) {
            
            clock_t c0 = clock();
            EQsahedron *pEQS = EQsahedron::createInstance(iNumSubDivs, true, pVR, dAlt, NULL);
            clock_t c1 = clock();
  
            iResult = pEQS->save("test1.ieq");
            printf("now relink\n");
            clock_t c3 = clock();
            pEQS->relink();
            clock_t c4 = clock();

            printf("creation:  %f\n", (1.0*c1 - c0)/CLOCKS_PER_SEC);
            printf("relinking: %f\n", (1.0*c4 - c3)/CLOCKS_PER_SEC);
            // collect points below alt
           
            // find affected faces


            delete pEQS;
            delete pVR;
        } else {
            if (pVR == NULL) {
                printf("couldn't open ValReader for [%s]\n", apArgV[2]);
            } 
            if (iNumSubDivs < 1) {
                printf("can't do %s subdivisions\n", apArgV[1]);
            } 

        }
    } else {
        iResult = -1;
        printf("%s <numsubdiv> <altmap> <minalt>\n", apArgV[0]);
    }
    return iResult;
}
