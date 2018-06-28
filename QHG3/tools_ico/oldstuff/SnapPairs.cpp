#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>

#include "utils.h"
#include "strutils.h"
#include "icoutil.h"
#include "Icosahedron.h"
#include "NodeLister.h"
#include "SnapHeader.h"

//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - extract corresponding pairs from 2 snap files\n", pApp);
    printf("usage:\n");
    printf("  %s <Snap1> <Snap2> <Output>\n", pApp);
    printf("where\n");
    printf("  IcoFile  : name of the ico file\n");
    printf("  Snap1    : name of first input Snap\n");
    printf("  Snap2    : name of second input Snap\n");
    printf("  Output   : name for result text file\n");
    printf("Note that the Snaps must be built for the same icosahedron,\n");
    printf("otherwise the results are undefined\n");
    printf("\n");
}

//-------------------------------------------------------------------------------------------------
// writeCorrespondingPairs
//
int writeCorrespondingPairs(FILE *fOut, Icosahedron *pIco, nodelist &nlVal1, nodelist &nlVal2) {
    int iResult = 0;
    nodelist::const_iterator it1 = nlVal1.begin();
    nodelist::const_iterator it2 = nlVal2.begin();

    while ((it1 != nlVal1.end()) || (it2 != nlVal2.end())) {
        if ((it1 != nlVal1.end()) && (it2 != nlVal2.end())) {
            while ((it1 != nlVal1.end()) && (it1->first < it2->first)) {
                it1++;
            }
            while ((it2 != nlVal1.end()) && (it1->first > it2->first)) {
                it2++;
            }
        }
        if ((it1 != nlVal1.end()) && (it2 != nlVal2.end())) {
            if (it1->first == it2->first) {
	    if (it1->second >= 0) {
                double dLon = dNaN;
                double dLat = dNaN;
                bool bFound =  pIco->findCoords(it1->first, &dLon, &dLat);
                if (bFound) {
                    dLon *= 180/M_PI;
                    dLat *= 180/M_PI;
                } else {
                    printf("Couldn't find coords for node [%d] (incomplete icosahedron?)\n", it1->first);
                    iResult = -1;
                }

                fprintf(fOut, "%f %f %f %f\n", dLon, dLat, it1->second, it2->second);
		}
                it1++;
                it2++;
            }
            
        } else if (it1 != nlVal1.end())  {
            it1++;
        } else if (it2 != nlVal2.end())  {
            it2++;
        }
        
    }
    return iResult;
}












//-------------------------------------------------------------------------------------------------
// main
//  arguments: 
//    <Snap1> <Snap2> <SnapOut>
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char sIcoFile[LONG_INPUT];
    char sInput1[LONG_INPUT];
    char sInput2[LONG_INPUT];
    char sOutput[LONG_INPUT];
    bool bPreSel = false;

    nodelist nlVal1;
    nodelist nlVal2;
    
    if (iArgC > 4) {
        int iPar=2;
        if (iArgC > 5) {
            if (strcmp(apArgV[2],"-p")==0) {
                bPreSel = true;
                iPar++;
            }
        }
        strcpy(sIcoFile, apArgV[1]);
        strcpy(sInput1,  apArgV[iPar++]);
        strcpy(sInput2,  apArgV[iPar++]);
        strcpy(sOutput,  apArgV[iPar++]);




        double   dMax;
        double   dMin;
        Icosahedron  *pIco = NULL;
        
        // extract ico
        if (iResult == 0) {
            pIco = Icosahedron::create(1, POLY_TYPE_ICO);
            pIco->setStrict(true);
            pIco->setPreSel(bPreSel);
            iResult = pIco->load(sIcoFile);
            if (iResult == 0) {
                pIco->relink();
            }
        }

        if (iResult == 0) {

            // check if sInput1 is a filename; if it is, read it
            iResult = NodeLister::createList(sInput1, nlVal1, &dMin, &dMax);
            if (iResult == 0) {
                printf("loaded %zd items from [%s]\n", nlVal1.size(), sInput1);
            } else {
                printf("[%s] is not a valid snap file\n", sInput1);
            }
        }
            
            
        if (iResult == 0) {
            // check if sInput2 is a filename; if it is, read it
            iResult = NodeLister::createList(sInput2, nlVal2, &dMin, &dMax);
            if (iResult == 0) {
                printf("loaded %zd items from [%s]\n", nlVal2.size(), sInput2);
            } else{
                printf("[%s] is not a valid snap file\n", sInput2);
            }
        }
    
        // do the pairing
        if (iResult == 0) {
            FILE *fOut = fopen(sOutput, "wt");
            if (fOut != NULL) {
                // 
                printf("starting the pairingof list1 (%zd items) and list2 (%zd items)\n", nlVal1.size(), nlVal2.size());
                iResult=writeCorrespondingPairs(fOut, pIco, nlVal1, nlVal2);
                if (iResult != 0) {
                    printf("[%s] an error occured during pair extraction\n", sInput1);
                }


                fclose(fOut);
            }
        }

        if (pIco != NULL) {
            delete pIco;
        }

    
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
