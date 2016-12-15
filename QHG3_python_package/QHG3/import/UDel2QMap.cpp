#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "utils.h"
#include "QMapUtils.h"
#include "QMapHeader.h"
#include "UDelReader.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"


const char *TEMP_NAME = "__TEMP_U2B_";


//-------------------------------------------------------------------------------------------------
// doPadding
//  set value of every nan or inf field to the average of its neighbors in a neighborhood
//  of width iP
//
bool doPadding(char *pIn, char*pOut, int iP, bool bCirc, double dDef) {
    printf("padding\n");
    bool bOK = false;
    QMapReader<float> *pUT = dynamic_cast<QMapReader<float>* >(QMapUtils::createValReader(pIn, false));
    if (pUT != NULL) {
        QMapHeader *pQMH3 = new QMapHeader(QMAP_TYPE_FLOAT,  
                                           pUT->getLonMin(),  pUT->getLonMax(),  pUT->getDLon(), 
                                           pUT->getLatMin(),  pUT->getLatMax(),  pUT->getDLat());
        FILE *fpP = fopen(pOut, "wb");
        pQMH3->addHeader(fpP);

        int iW = pUT->getNRLon();
        int iH = pUT->getNRLat();

        float **aadData = pUT->getData();

        // shifting window of 2*iP+1 (lines relevant for averaging
        float **adBuf = new float*[2*iP+1];
        for (int i = 0; i < iP+1; i++)  {
            if (i < iP+1) {
                adBuf[i] = aadData[iP-i];
            } else {
                adBuf[i] = NULL;
            }
        }

        float *adBufOut = new float[iW];
        printf("really padding now\n");

        float dR  = 0;
        float dRY = 0;
        float dP2 = iP*iP;
        for (int iY = 0; iY < iH; iY++) {
            for (int iX = 0; iX < iW; iX++) {
            
                if (!isfinite(aadData[iY][iX])) {
                    float dSum = 0;
                    int iCount = 0;
                    bool bFinite = false;
                    for (int iPY = -iP; iPY <= iP; iPY++) {
                        int iOY = iY + iPY;
                        if (bCirc) {
                            dRY = iPY*iPY;
                        }
                        if ((iOY >= 0) && (iOY < iH)) {
                            for (int iPX = -iP; iPX <= iP; iPX++) {
                                int iOX = iX + iPX;
                                if (bCirc) {
                                    dR = dRY+iPX*iPX;
                                }

                                if ((iOX >= 0) && (iOX < iW) && (!bCirc || (dR <= dP2))) {
                                    float dV = aadData[iOY][iOX];
                                    if (isfinite(dV)) {
                                        bFinite = true;
                                    } else if (isfinite(dDef)) {
                                        dV = dDef;
                                    }
                                    if (isfinite(dV)) {
                                        if (bCirc && (dR > 0)) {
                                            //  dV /= sqrt(dR);
                                        }
                                        dSum += dV;
                                        iCount++;
                                    }
                                }
                            }
                        }
                    }
                    if ((iCount > 0) && bFinite) {
                        adBufOut[iX] = dSum/iCount;
                        //printf("Bal %d,%d (%f): %f\n", iX, iY, aadData[iY][iX], adBufOut[iX]);
                    } else {
                        adBufOut[iX] = UDEL_BAD_VAL;
                    }
                } else {
                    adBufOut[iX] = aadData[iY][iX];
                }
            }
            fwrite(adBufOut, sizeof(float), iW, fpP);
            // shift all line pointers
            for (int iL = 2*iP; iL > 0; iL--) {
                adBuf[iL] = adBuf[iL-1];
            }
            adBuf[0] = aadData[iY];
        }
        fclose(fpP);
        delete pQMH3;
        bOK = true;
    } else {
        printf("Couldn't open temporary UDEL file [%s] - no padding performed\n", pIn);
    }
    return bOK;
}



//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    char sInput[128];
    char sOutput[128];
    char sOutput2[128];
    bool bCirc = false;
    int iColumnStart = UDEL_ANN_START;
    int iColumnEnd   = UDEL_ANN_END;
    double dDef = dNaN;

    bool bOK = false;
    if (iArgC > 2) {
        int iP = 0;
        int iF = 0;
        if (iArgC > 3) {
            int i = 3;
            bOK = true;
            char *pEnd;
            while (bOK && (i < iArgC)) {
                if (strcmp(apArgV[i], "-p") == 0) {
                    i++;
                    if (i < iArgC) {
                        iP = strtol(apArgV[i], &pEnd, 10);
                        if (*pEnd =='c') {
                            bCirc = true;
                        }
                    } else {
                        bOK = false;
                    }
                } else if (strcmp(apArgV[i], "-f") == 0) {
                    i++;
                    if (i < iArgC) {
                        iF = strtol(apArgV[i], &pEnd, 10);
                        if (*pEnd =='\0') {
                            if ((iF >0 ) && (iF < 13)) {
                                iF--;
                                iColumnStart = UDEL_DATA_OFFSET + iF*UDEL_FIELD_SIZE;
                                iColumnEnd   = iColumnStart + UDEL_FIELD_SIZE - 1;
                            }
                        } else {
                            bOK = false;
                        }
                    } else {
                        bOK = false;
                    }
                } else if (strcmp(apArgV[i], "-d") == 0) {
                    i++;
                    if (i < iArgC) {
                        double dTemp = strtod(apArgV[i], &pEnd);
                        if (*pEnd =='\0') {
                            dDef = dTemp;
                        }
                    }
                } else {
                    printf("unknown param: %s\n", apArgV[i]); 
                    // bOK = false
                }
                ++i;
            }
        } else {
            bOK = true;
        }
        
   
        strcpy(sInput, apArgV[1]);
        if (iP > 0) {
            strcpy(sOutput, TEMP_NAME);
            strcpy(sOutput2,apArgV[2]);
        } else {
            strcpy(sOutput, apArgV[2]);
        }

        int iResult = UDelReader::convert2QMap(sInput, sOutput, iColumnStart, iColumnEnd);
        if (iResult == 0) {
            if (iP > 0) {
                printf("Now padding with %d: %s --> %s\n", iP, sOutput, sOutput2);
                bOK = doPadding(sOutput, sOutput2, iP, bCirc, dDef);
                if (bOK) {
                    // delete temp
                    remove(sOutput);
                } else {
                    // rename temp to output
                    rename(sOutput, sOutput2);
                }
            }
            if (bOK && (iResult == 0)) {
                printf("+++ success +++\n");
            }

        } else {
            printf("Couldn't extract/convert data\n");
        }
    }
    
    if (!bOK) {
        printf("%s - convert UDel to BMap.\n", apArgV[0]);
        printf("Usage:\n");
        printf("  %s <udelfile> <outputfile> [-p<padding>[c]] [-f <field>]\n", apArgV[0]);
        printf("where\n");
        printf("  udelfile :   a UDEL file\n");
        printf("  outputfile : name of output file\n");
        printf("  padding :    padding width (nan/inf cells get average of neighbors)\n");
        printf("  c :          use circular padding instead of square\n");
        printf("  field :      0 for Annual, 1->Jan, ..., 12->Dec\n");
    }

    return bOK?0:-1; 
}
