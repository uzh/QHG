#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "types.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "ValReader.h"


void usage(char *pName) {
    printf("%s - create raw file from QMap\n", pName);
    printf("usage:\n");
    printf("  %s  <qmap_in> <raw_out>\n", pName);
    printf("where\n");
    printf("  qmap_in      name of input qmap-file\n");
    printf("  raw_out      name of output raw file\n");
    printf("\n");
    printf("Creates a raw data file from the contents in the QMap\n");
    printf("The data type of the QMap is used\n");
    printf("\n");
}




template <class T>
bool doRaw(QMapReader<T> *pQMR, char *pOut) {
    int iResult = -1;
    // create array
    
    T **aatData = pQMR->getData();
    
              
    // save the array
    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {
        bool bOK = QMapUtils::writeArray(fOut, pQMR->getNRLon(), pQMR->getNRLat(), aatData);
        if (bOK) {
            fclose(fOut);
            printf("Success!\n");
            iResult = 0;
        } else {
            printf("Couldn't write data to output file [%s]\n", pOut);
                iResult = -4;
        }
            
    } else {
        printf("Couldn't open output file [%s]\n", pOut);
        iResult = -2;
    }
    return iResult;
}

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 2) {
        char sIn[LONG_INPUT];
        char sOut[LONG_INPUT];

        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sIn);
        searchFile(apArgV[2], pDataDir, sOut);
            
        
        int iType = QMapHeader::getQMapType(sIn);
        if (QMAP_TYPE_OK(iType)) {
            ValReader *pVR = QMapUtils::createValReader(sIn, true);
            if (pVR != NULL) {
                switch (iType) {
                case QMAP_TYPE_UCHAR: {
                    QMapReader<uchar> *pQMR = dynamic_cast<QMapReader<uchar>*> (pVR);
                    iResult = doRaw(pQMR, sOut);
                }
                    break;
                case QMAP_TYPE_SHORT:  {
                    QMapReader<short int> *pQMR = dynamic_cast<QMapReader<short int>*> (pVR);
                    iResult = doRaw(pQMR, sOut);
                }
                    break;
                case QMAP_TYPE_INT : {
                    QMapReader<int> *pQMR = dynamic_cast<QMapReader<int>*> (pVR);
                    iResult = doRaw(pQMR, sOut);
                }
                    break;
                case QMAP_TYPE_LONG : {
                    QMapReader<long> *pQMR = dynamic_cast<QMapReader<long>*> (pVR);
                    iResult = doRaw(pQMR, sOut);
                }
                    break;
                case QMAP_TYPE_FLOAT: {
                    QMapReader<float> *pQMR = dynamic_cast<QMapReader<float>*> (pVR);
                    iResult = doRaw(pQMR, sOut);
                }
                    break;
                case QMAP_TYPE_DOUBLE: {
                    QMapReader<double> *pQMR = dynamic_cast<QMapReader<double>*> (pVR);
                    iResult = doRaw(pQMR, sOut);
                }
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
            
        } else {
            printf("Input file is not a qmap [%s]\n", apArgV[1]);
        }
 
    } else {
        usage(apArgV[0]);
    }
    return iResult;

}






