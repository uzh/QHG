#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <set>

#include "utils.h"
#include "Vec3D.h"
#include "PolyFace.h"
#include "IcoFace.h"
#include "Icosahedron.h"
#include "VertexLinkage.h"

int K(int N) {
    return (N*(N+1))/2;
}

typedef std::set<IcoFace *> tfacelist;

void variableSubDiv(Icosahedron *pI, int iMaxLevel, double dLonMin, double dLonMax, double dDLon, double dLatMin, double dLatMax, double dDLat) {
    
    tfacelist *pFL1=new tfacelist;

    for (int i = 0; i < 20; i++) {
        pFL1->insert(pI->getFace(i));
    }
    /*
    pI->relink();
    pI->display();
    */
    int iLevel = 0;
    while (iLevel < iMaxLevel) {
        printf("Doing level %d/%d on %zd faces\n", iLevel, iMaxLevel, pFL1->size());
        tfacelist *pFL2  = new tfacelist;
        for (double dLat = dLatMin; dLat < dLatMax+dDLat; dLat += dDLat) {
            for (double dLon = dLonMin; dLon < dLonMax+dDLon; dLon += dDLon) {
                Vec3D v(cos(dLat)*cos(dLon), cos(dLat)*sin(dLon), sin(dLat));
                tfacelist::iterator it;
                //                printf("Checking (%f,%f)->(%f,%f,%f)\n", dLon, dLat, v.m_fX, v.m_fY, v.m_fZ);
                for (it = pFL1->begin(); it != pFL1->end(); ++it) {
                    IcoFace *pF = *it;
                    if (pF->contains(&v)) {
                        pFL2->insert(pF);
                    }
                }
            }
        }
        printf("involved faces: %zd\n", pFL2->size());
        delete pFL1;
        pFL1  = new tfacelist;
        tfacelist::iterator it;
        for (it = pFL2->begin(); it != pFL2->end(); ++it) {
            //            (*it)->display();
            (*it)->subdivide(1);
            IcoFace *pFS = (*it)->getFirstSubFace();
            while (pFS != NULL) {
                //                printf("  inserting ");
                //                pFS->display();
                pFL1->insert(pFS);
                pFS = pFS->getNextSubFace();
            }
        }
        /*
        pI->relink();
        for (int i = 0; i < 20; i++) {
            pI->getFace(i)->displayRec("");
        }
        */
        delete pFL2;
        ++iLevel;

    }

    //    pI->relink();
}



int main1(int iArgC, char *apArgV[]) {
    /*
    Vec3D v0(8, 0, 0);
    Vec3D v1(0, 8, 0);
    Vec3D v2(0, 0, 8);


    IcoFace *pF = IcoFace::createFace(&v0, &v1, &v2);
    if (pF != NULL) {
        printf("Top Face: ");
        pF->display();
        
        printf("All Faces:\n");
        pF->displayRec(" ");
        
        pF->subdivide(3);
        
        printf("All Faces (subdiv):\n");
        pF->displayRec(" ");
    
        
        Vec3D vP(5,0.8,2.2);
        IcoFace *pFC = pF->contains(&vP);
        if (pFC != NULL) {
            printf("Point (%f,%f,%f) contained in ", vP.m_fX, vP.m_fY, vP.m_fZ);
            pFC->display();

            Vec3D *pV = pFC->closestVertex(&vP);
            printf("Closest vertex  (%f,%f,%f)\n", pV->m_fX, pV->m_fY, pV->m_fZ);
        } else {
            printf("Point (%f,%f,%f) not contained in ", vP.m_fX, vP.m_fY, vP.m_fZ);
            pF->display();
        }
        
        VertexLinkage *pVL = new VertexLinkage();
        printf("the list\n");
        IcoFace *pFF = pF->getFirstSubFace();
        while (pFF != NULL) {
            pFF->display();
            pVL->addFace(pFF);
            pFF = pFF->getNextSubFace();
        }

        
        pVL->display();
        delete pVL;
        
        
        delete pF;
    } else {
        printf("Creation of face failed\n");
    }

      */
        
    printf("\nIco Linkage\n\n");
    printf("------------------------------------------\n");
    
    if (iArgC > 1) {
        double dLon0 =  5;
        double dLon1 = 15;
        double dDLon =  1;
        double dLat0 = 20;
        double dLat1 = 50;
        double dDLat =  1;
        if (iArgC > 7) {
            dLon0 = atof(apArgV[2]);
            dLon1 = atof(apArgV[3]);
            dDLon = atof(apArgV[4]);
            dLat0 = atof(apArgV[5]);
            dLat1 = atof(apArgV[6]);
            dDLon = atof(apArgV[7]);
        }
        
        Icosahedron *pI = Icosahedron::create(5);
        //pI->subdivide(atoi(apArgV[1]));
               variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(dLon0), DEG2RAD(dLon1), DEG2RAD(dDLon), DEG2RAD(dLat0), DEG2RAD(dLat1), DEG2RAD(dDLat));
               //     pI->variableSubDiv(atoi(apArgV[1]), DEG2RAD(dLon0), DEG2RAD(dLon1), DEG2RAD(dDLon), DEG2RAD(dLat0), DEG2RAD(dLat1), DEG2RAD(dDLat));
        
        //ok variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(15), DEG2RAD(15), DEG2RAD(1), DEG2RAD(24), DEG2RAD(24), DEG2RAD(1));
        //ok variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(15), DEG2RAD(15), DEG2RAD(1), DEG2RAD(84), DEG2RAD(84), DEG2RAD(1));
        //bad 2 hits variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(15), DEG2RAD(15), DEG2RAD(1), DEG2RAD(63), DEG2RAD(63), DEG2RAD(1));
        //bad 2 hits variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(15), DEG2RAD(15), DEG2RAD(1), DEG2RAD(53), DEG2RAD(53), DEG2RAD(1));
        // bad 3 hits variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(15), DEG2RAD(15), DEG2RAD(1), DEG2RAD(54), DEG2RAD(54), DEG2RAD(1));
        //variableSubDiv(pI,atoi(apArgV[1]), DEG2RAD(15), DEG2RAD(15), DEG2RAD(1), DEG2RAD(58), DEG2RAD(58), DEG2RAD(1));
        
        pI->relink();
        
        pI->display();
        /*
        pI->merge(0);
        pI->relink();
        */
        //  pI->getLinkage()->display();
        int iNumFaces = 0;
        for (int i = 0; i < 20; i++) {
            iNumFaces += pI->getFace(i)->getNumFaces();
        }
        int iNumVerts = pI->getLinkage()->getNumVertices();
        printf("Num faces: %d\n", iNumFaces);
        printf("Num verts: %d\n", iNumVerts);
        int N = 1 << atoi(apArgV[1]);
        printf("control: N=%d, K(N-2)=%d, total %d\n", N, K(N-2), 12 + 20*K(N-2) + 30*(N-1)); 
        printf("done\n");
        // delete pVL2;
        
               
        int iCount = 0;
        PolyFace *pF = pI->getFirstFace();
        while (pF != NULL) {
            printf("%02d: %f %f %f\n    %f %f %f\n    %f %f %f\n", iCount++, 
                   pF->getVertex(0)->m_fX,pF->getVertex(0)->m_fY,pF->getVertex(0)->m_fZ,
                   pF->getVertex(1)->m_fX,pF->getVertex(1)->m_fY,pF->getVertex(1)->m_fZ,
                   pF->getVertex(2)->m_fX,pF->getVertex(2)->m_fY,pF->getVertex(2)->m_fZ);
            pF = pI->getNextFace();
        }
        
        delete pI;
    } else {
        printf("usage: %s <subdivlevels> [<dLonMin> <dLonMax> <dDLon> <dlatMin> <dLatMax> <dDLat>\n", apArgV[0]); 
    }
    
    
    // todo: check correctness of Face::contains
    return 0;
};


void check(IcoFace *pF, Vec3D *pv) {
    IcoFace *pF1 = pF->contains(pv);
    if (pF1 != NULL) {
        printf("(%f, %f, %f) is contained\n", pv->m_fX, pv->m_fY, pv->m_fZ);
        Vec3D *pv1 = pF->closestVertex(pv);
        printf("  closest Vertex (%f, %f, %f)\n", pv1->m_fX, pv1->m_fY, pv1->m_fZ);
    } else {
        printf("(%f, %f, %f) is not contained\n", pv->m_fX, pv->m_fY, pv->m_fZ);
    }
    printf("\n");
}

void check(IcoFace *apF[], int iC, double dLon, double dLat) {
    IcoFace *pF = NULL;
    Vec3D v(cos(dLat*M_PI/180)*cos(dLon*M_PI/180),
            cos(dLat*M_PI/180)*sin(dLon*M_PI/180),
            sin(dLat*M_PI/180));

    for (int i = 0; (pF == NULL) && (i < iC); i++) {
        pF = apF[i]->contains(&v);
        if (pF != NULL) {printf(">>>%d<<<\n", i);}
    }
    if (pF != NULL) {
        printf("(%f, %f)=(%f, %f, %f) is contained in face ", dLon, dLat, v.m_fX, v.m_fY, v.m_fZ);
        pF->display();
        Vec3D *pv1 = pF->closestVertex(&v);
        printf("  closest Vertex (%f, %f, %f)\n", pv1->m_fX, pv1->m_fY, pv1->m_fZ);
    } else {
        printf("(%f, %f)=(%f, %f, %f) is not contained anywhere\n", dLon, dLat, v.m_fX, v.m_fY, v.m_fZ);
    }
    printf("\n");

}

int main2(int iArgC, char *apArgV[]) {
    IcoFace *apF[8];
    Vec3D vx(1,0,0);
    Vec3D vy(0,1,0);
    Vec3D vz(0,0,1);
    int iC = 0;
    int iP = 1;
    for (int i = 0; i < 2; ++i) {
        int x = 2*i-1;
        Vec3D *vA= new Vec3D(vx);
        vA->scale(x);
        for (int j = 0; j < 2; ++j) {
            int y = 2*j-1;
            Vec3D *vB = new Vec3D(vy);
            vB->scale(y);
            for (int k = 1; k < 2; ++k) {
                int z = 2*k-1;
                Vec3D *vC= new Vec3D(vz);
                vC->scale(z);
                iP = x*y*z;
                printf("par: %d\n", iP);
                
                if (iP > 0) {
                    apF[iC++] = IcoFace::createFace(vA, vB, vC);
                } else {
                    apF[iC++] = IcoFace::createFace(vA, vC, vB);
                }
                printf("Face %d: ", iC-1);
                apF[iC-1]->display();
            }
        }
    }
        printf("\n---------\n");

        for(int i = 0; i < iC; i++) {
            printf("Face %d (second): ", i);
            apF[i]->display();
        }

        check(apF, iC, 30, 30);
        check(apF, iC, 0, 0);
        check(apF, iC, -1.01, 30);
        check(apF, iC, 130, -60);
    /*
    check(pF, &vx);
    check(pF, &vy);
    check(pF, &vz);
    
    Vec3D v0(0.5,0,0.5);
    Vec3D v1(0.8,0.2,0);
    Vec3D v2(0.1,0.2, 0.7);
    
      check(pF, &v0);
      check(pF, &v1);
      check(pF, &v2);
   
    Vec3D v3(0.1,0.2, 0.8);
    check(pF, &v3);

    Vec3D v4(0,0, 0);
    check(pF, &v4);

    Vec3D v5(0.2,0.2, 0.8);
    check(pF, &v5);

    Vec3D v6(cos(90.0*M_PI/180)*0.5, sin(90.0*M_PI/180)*0.5, 0.86603);
    check(pF, &v6);
    */
    return 0;
}



int main(int iArgC, char *apArgV[]) {
    // int iResult = main1(iArgC, apArtgV);

    int iResult = main2(iArgC, apArgV);

    return iResult;
}

