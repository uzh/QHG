#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "types.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "ValReader.h"


void usage(char *pName) {
    printf("%s - \"roll\" qmap\n", pName);
    printf("usage:\n");
    printf("  %s  <qmap_in> <xroll> <yroll> <qmap_out>\n", pName);
    printf("where\n");
    printf("  qmap_in      name of input qmap-file\n");
    printf("  xroll        number of pixels to roll in x direction\n");
    printf("               (positive:right, negative:left)\n");
    printf("  yroll        number of pixels to roll in y direction\n");
    printf("               (positive:down, negative:up)\n");
    printf("  qmap_out     name of output qmap-file\n");
    printf("\n");
    printf("Creates a qmap resulting from rolling the original by\n");
    printf(" xroll pixels to the right and yroll pixels down\n");
    printf("Especially useful to place the bering strait to the center\n");
    printf("\n");
}

template <class T>
bool doRoll(ValReader *pVR, int iXRoll, int iYRoll, QMapHeader *pQMHOut, char *pOut, T defVal) {
    int iResult = -1;
    QMapReader<T> *pQMR = dynamic_cast<QMapReader<T> *>(pVR);
    if (pQMR != NULL) {
        T **aadDataIn = pQMR->getData();
        // create array
        T **aadDataOut = QMapUtils::createArray(pQMHOut->m_iWidth, pQMHOut->m_iHeight, defVal);
        // fill it
        for (unsigned int i = 0; i < pQMHOut->m_iHeight; i++) {
            unsigned int i2 = (i+pQMHOut->m_iHeight+iYRoll)%pQMHOut->m_iHeight;
            unsigned int iSize0 = 0;
            unsigned int iSize1 = 0;
 
            if (iXRoll > 0) {
                iXRoll %=  pQMHOut->m_iWidth;
            } else {
                iXRoll +=  pQMHOut->m_iWidth;
            }
            iSize0 = iXRoll;
            iSize1 = pQMHOut->m_iWidth - iXRoll;

            if (iSize0 > 0) {
                memcpy(aadDataOut[i2], aadDataIn[i]+iSize1, iSize0*sizeof(T));
            }
            if (iSize1 > 0) {
                memcpy(aadDataOut[i2]+iSize0, aadDataIn[i], iSize1*sizeof(T));
            }
        }
              
        // save the array
        FILE *fOut = fopen(pOut, "wb");
        if (fOut != NULL) {
            bool bOK = pQMHOut->addHeader(fOut);
            if (bOK) {
                bOK = QMapUtils::writeArray(fOut, pQMHOut->m_iWidth, pQMHOut->m_iHeight, aadDataOut);
                if (bOK) {
                fclose(fOut);
                printf("Success!\n");
                iResult = 0;
                } else {
                    printf("Couldn't write data to output file [%s]\n", pOut);
                    iResult = -4;
                }
                
            } else {
                printf("Couldn't write header to output file [%s]\n", pOut);
                iResult = -3;
            }
            
        } else {
            printf("Couldn't open output file [%s]\n", pOut);
            iResult = -2;
        }
        QMapUtils::deleteArray(pQMHOut->m_iWidth, pQMHOut->m_iHeight, aadDataOut);
    } else {
        printf("Couldn't cast to QMapReader\n");
        iResult = -2;
    }
    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 4) {
        char sIn[LONG_INPUT];
        char sOut[LONG_INPUT];
        int iXRoll;
        int iYRoll;

        char *pDataDir = getenv("DATA_DIR");

        searchFile(apArgV[1], pDataDir, sIn);
        
        iXRoll = atoi(apArgV[2]);
        iYRoll = atoi(apArgV[3]);
        strcpy(sOut,     apArgV[4]);
        

        FILE * fIn = fopen(sIn, "rb");
        if (fIn != NULL) {

            QMapHeader *pQMHOut = new QMapHeader();
            iResult = pQMHOut->readHeader(fIn);
            if (iResult == 0) {
                ValReader *pVR = QMapUtils::createValReader(sIn, true);
                if (pVR != NULL) {
                    switch ( pQMHOut->m_iType) {
                    case QMAP_TYPE_UCHAR: 
                        iResult = doRoll(pVR, iXRoll, iYRoll, pQMHOut, sOut, (uchar) 0xff);
                        break;
                    case QMAP_TYPE_SHORT: 
                        iResult = doRoll(pVR, iXRoll, iYRoll, pQMHOut, sOut, (short int) -1);
                        break;
                    case QMAP_TYPE_INT : 
                        iResult = doRoll(pVR, iXRoll, iYRoll, pQMHOut, sOut, (int) -1);
                        break;
                    case QMAP_TYPE_LONG : 
                        iResult = doRoll(pVR, iXRoll, iYRoll, pQMHOut, sOut, (long) -1);
                        break;
                    case QMAP_TYPE_FLOAT: 
                        iResult = doRoll(pVR, iXRoll, iYRoll, pQMHOut, sOut, fNaN);
                        break;
                    case QMAP_TYPE_DOUBLE: 
                        iResult = doRoll(pVR, iXRoll, iYRoll, pQMHOut, sOut, dNaN);
                        break;
                    default:
                        iResult = -1;
                    }
                    
                    if (iResult == 0) {
                        printf("+++ success +++\n");
                    }
                } else {
                    printf("Couldn't read input file [%s]\n", apArgV[1]);
                }
                delete pVR;
                delete pQMHOut;


            } else {
                printf("Couldn't read header from [%s]\n", sIn);
            }
            fclose(fIn);
        } else {
            printf("Couldn't open [%s] for reading\n", sIn);
        }
 



    } else {
        usage(apArgV[0]);
    }
    return iResult;
}

