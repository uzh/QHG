#include <gtkmm.h>
#include <stdio.h>
#include <string.h>

#include "LineReader.h"
#include "ProjInfo.h"
#include "Icosahedron.h"
#include "SnapQuatProjector.h"

void usage(char *pApp) {
    printf("%s - testing SnapQuatProjector\n", pApp);
    printf("Usage:\n");
    printf("  %s <icofile> <input file> <outputbody>\n", pApp);
    printf("where\n");
    printf("  icofile        an icosahedron file\n");
    printf("  inputfile      a text file with lines of the form\n");
    printf("                   snapfile qx qy qz qw\n");
    printf("                 snap file:   a snap file for the given icosahedron\n");
    printf("                 qx,qy,qz,qw: quaternion for projection\n");
    printf("  outputbody     body of output files\n");
    printf("\n");
}

int main(int iArgC, char *apArgV[]) {
    Gtk::Main kit(iArgC, apArgV);
    //  Gtk::GL::init(iArgC, apArgV);

    if (iArgC > 3) {
        char sIcoFile[512];
        char sSQFile[512];
        char sOutputBody[512];
        float q[4];
        
        strcpy(sIcoFile, apArgV[1]);
        strcpy(sSQFile, apArgV[2]);
        strcpy(sOutputBody, apArgV[3]);
        
        Icosahedron *pIco = Icosahedron::create(1);
        pIco->setStrict(true);
        int iResult = pIco->load(sIcoFile);
        if (iResult == 0) {
            pIco->relink();

            SnapQuatProjector *pSQP = new SnapQuatProjector(pIco, 200, 200, 60);

            LineReader *pLR = LineReader_std::createInstance(sSQFile, "rt");
            if (pLR != NULL) {
                char sSnap[512];
                char sOutputFile[512];
                int iCount = 0;
                while(!pLR->isEoF()) { 
                    char *p = pLR->getNextLine();
                    if (p != NULL) {
                        printf("Line [%s]\n", p);
                        sscanf(p, "%s %f %f %f %f", sSnap, &(q[0]), &(q[1]), &(q[2]), &(q[3]));
                        printf("got [%s], %f %f %f %f\n", sSnap, q[0], q[1], q[2], q[3]);
                        sprintf(sOutputFile, "%s_%04d.png", sOutputBody, iCount);
                        pSQP->drawProjection(sSnap, q, sOutputFile); 
                        iCount++;
                    }
                }
                
                delete pLR;
                delete pSQP;
            } else {
                printf("Couldn't open [%s] for reading\n", sSQFile);
            }
        } else {
            printf("Couldn't read icofile [%s]\n", sIcoFile);
        }
        delete pIco;
    } else {
        usage(apArgV[0]);
    }
}
