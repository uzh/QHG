#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "types.h"
#include "SimpleMapper.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "ValReader.h"
#include "SimpleMapper.h"
#include "SimpleMapper.cpp"


void usage(char *pName) {
    printf("%s - translate qmap values according to table\n", pName);
    printf("usage:\n");
    printf("  %s  <qmap_in> <transtable> <qmap_out>\n", pName);
    printf("where\n");
    printf("  qmap_in      name of input qmap-file\n");
    printf("  transtable   text file containing two column table\n");
    printf("                 <fromValue> <toValue>\n");
    printf("  qmap_out     name of output qmap-file\n");
    printf("\n");
    printf("Creates a qmap resulting by translating the values according to the table\n");
    printf("\n");
}

template <class T>
bool doTranslate(ValReader *pVR, const char *pTransFile, QMapHeader *pQMHOut, char *pOut, T defVal) {
    int iResult = -1;
    QMapReader<T> *pQMR = dynamic_cast<QMapReader<T> *>(pVR);
    if (pQMR != NULL) {
        ValueMapper<T> *pVM = new SimpleMapper<T>(pTransFile, defVal);


        T **aadDataIn = pQMR->getData();
        // create array
        T **aadDataOut = QMapUtils::createArray(pQMHOut->m_iWidth, pQMHOut->m_iHeight, defVal);
        // fill it
        for (uint i = 0; i < pQMHOut->m_iHeight; i++) {
            for (uint j = 0; j < pQMHOut->m_iWidth; j++) {
                aadDataOut[i][j] =  pVM->mapValue(aadDataIn[i][j]);
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
        delete pVM;
        QMapUtils::deleteArray(pQMHOut->m_iWidth, pQMHOut->m_iHeight, aadDataOut);
    } else {
        printf("Couldn't cast to QMapReader\n");
        iResult = -2;
    }
    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 3) {
        char sIn[LONG_INPUT];
        char sOut[LONG_INPUT];
        char sTrans[LONG_INPUT];

        char *pDataDir = getenv("DATA_DIR");

        searchFile(apArgV[1], pDataDir, sIn);
        
        strcpy(sTrans,   apArgV[2]);
        strcpy(sOut,     apArgV[3]);
        

        FILE * fIn = fopen(sIn, "rb");
        if (fIn != NULL) {

            QMapHeader *pQMHOut = new QMapHeader();
            iResult = pQMHOut->readHeader(fIn);
            if (iResult == 0) {
                ValReader *pVR = QMapUtils::createValReader(sIn, true);
                if (pVR != NULL) {
                    
                    switch ( pQMHOut->m_iType) {
                    case QMAP_TYPE_UCHAR: 
                        iResult = doTranslate(pVR, sTrans, pQMHOut, sOut, (uchar) 0xff);
                        break;
                    case QMAP_TYPE_SHORT: 
                        iResult = doTranslate(pVR, sTrans, pQMHOut, sOut, (short int) -1);
                        break;
                    case QMAP_TYPE_INT : 
                        iResult = doTranslate(pVR, sTrans, pQMHOut, sOut, (int) -1);
                        break;
                    case QMAP_TYPE_LONG : 
                        iResult = doTranslate(pVR, sTrans, pQMHOut, sOut, (long) -1);
                        break;
                    case QMAP_TYPE_FLOAT: 
                        iResult = doTranslate(pVR, sTrans, pQMHOut, sOut, fNaN);
                        break;
                    case QMAP_TYPE_DOUBLE: 
                        iResult = doTranslate(pVR, sTrans, pQMHOut, sOut, dNaN);
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
