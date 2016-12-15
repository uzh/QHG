#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "types.h"
#include "strutils.h"
#include "QConverter.h"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"


const int OP_PLUS   =  0;
const int OP_MINUS  =  1;
const int OP_TIMES  =  2;
const int OP_DIV    =  3;
const int OP_MIN    =  4;
const int OP_MAX    =  5;
const int OP_EQ     =  6;
const int OP_THR    =  7;
const int OP_SET    =  8;
const int OP_ISNAN  =  9;
const int OP_SETNAN = 10;
const int OP_INVDIV = 11;
const int OP_LOG    = 12;
const int NUM_OP    = 13;

// operator names
static const char *sOps[] = {
    "plus",
    "minus",
    "times",
    "div",
    "min",
    "max",
    "eq",
    "thr",
    "set",
    "isnan",
    "setnan",
    "invdiv",
    "log",
};

const char *TEMP_FILE = "__TEMP_QCOMBINE__";

//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - combine 2 QMaps of equal geometry with an operation\n", pApp);
    printf("usagee:\n");
    printf("  %s <QMap1> <op> <QMap2> <QMapOut>\n", pApp);
    printf("or\n");
    printf("  %s <QMap1> <op> <number> <QMapOut>\n", pApp);
    printf("where\n");
    printf("  QMap1    : name of first input QMap\n");
    printf("  QMap2    : name of second input QMap\n");
    printf("  QMapOut  : name for result QMap\n");
    printf("  op       : an operator:\n");
    for (unsigned int i = 0; i < sizeof(sOps)/sizeof(char *); i++) {
        printf("               %s\n", sOps[i]);
    }
    printf("In the first case the operator is applied to each element of QMap1 and\n");
    printf("the equivalent element of QMap2.\n");
    printf("In the second case the operator is applied to each element o QMap1 and the number\n");
    printf("\n");
    printf("If the two maps don't have the same type, the resulting map will have the 'higher' of the two types\n");
    printf("The type hierarchy is: uchar < short < int < long < float < double\n");
    printf("\n");
    printf("Example1 (Adding the elements of A to the elements of B and store in C\n");
    printf("  %s A.qmap plus B.qmap C.qmap\n", pApp);
    printf("Example2 (Doubling all elements of A and store in C\n");
    printf("  %s A.qmap times 2 C.qmap\n", pApp);

}

//-------------------------------------------------------------------------------------------------
// getOperation
//
int getOperation(char *pOp) {
    int iOp = -1;
    for (int i = 0; (iOp < 0) && (i < NUM_OP); i++) {
        if (strcmp(pOp, sOps[i]) == 0) {
            iOp = i;
        }
    }
    return iOp;
}

//-------------------------------------------------------------------------------------------------
// binCombine
//  combine the two QMaps with the specified operator and write to pOut
//
template<class T>
int  binCombine(int iOp, ValReader *pVR1, ValReader *pVR2, char *pOut, T undef, QMapHeader *pQMH) {
    int iResult = 0;
    int iH = pVR1->getNRLat();
    int iW = pVR2->getNRLon();

    QMapReader<T> *pQMR1 =dynamic_cast<QMapReader<T> *> (pVR1);
    T **ppData1 = pQMR1->getData();
    QMapReader<T> *pQMR2 =dynamic_cast<QMapReader<T> *> (pVR2);
    T **ppData2 = pQMR2->getData();
    T **ppData3 = new T*[iH];
    for (int i = 0; i < iH; i++) {
        ppData3[i] = new T[iW];
     
        for (int j = 0; j < iW; j++) {
            switch (iOp) {
            case OP_PLUS:
                ppData3[i][j] = ppData1[i][j]+ppData2[i][j];  
                break;
            case OP_MINUS:
                ppData3[i][j] = ppData1[i][j]-ppData2[i][j];  
                break;
            case OP_TIMES:
                ppData3[i][j] = ppData1[i][j]*ppData2[i][j];  
                break;
            case OP_DIV:
                if (ppData2[i][j] == 0) {
                    ppData3[i][j] = undef;
                } else {
                    ppData3[i][j] = ppData1[i][j]/ppData2[i][j];  
                }
                break;
            case OP_MAX:
                if (!isnan(ppData1[i][j]) && !isnan(ppData2[i][j])) {
                    ppData3[i][j] = (ppData1[i][j]>=ppData2[i][j])?ppData1[i][j]:ppData2[i][j];  
                } else {
                    ppData3[i][j] = undef;
                }
                break;
            case OP_MIN:
                if (!isnan(ppData1[i][j]) && !isnan(ppData2[i][j])) {
                    ppData3[i][j] = (ppData1[i][j]<=ppData2[i][j])?ppData1[i][j]:ppData2[i][j];  
                } else {
                    ppData3[i][j] = undef;
                }
                break;
            case OP_EQ:
                ppData3[i][j] = (ppData1[i][j]==ppData2[i][j])?ppData1[i][j]:undef;  
                break;
            case OP_THR:
                ppData3[i][j] = (ppData1[i][j]>=ppData2[i][j])?1:0;
                break;
            case OP_SET:
                if (!isnan(ppData2[i][j])) {
                    ppData3[i][j] = ppData2[i][j];
                } else { 
                    ppData3[i][j] = ppData1[i][j];
                }
                //     ppData3[i][j] = undef;
                break;
            case OP_ISNAN:
                ppData3[i][j] = isnan(ppData1[i][j]);
                break;
            case OP_SETNAN:
                ppData3[i][j] = isnan(ppData1[i][j])?ppData2[i][j]:ppData1[i][j];
                break;
            case OP_INVDIV:
                ppData3[i][j] = ppData2[i][j]/ppData1[i][j];  
                break;
            case OP_LOG:
                ppData3[i][j] = log(ppData1[i][j]);
                break;
            default:
                ppData3[i][j] = undef;

            }
        }
    }
    
    // write output

    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {

        bool bOK = pQMH->addHeader(fOut);
        if (bOK) {
            bOK = QMapUtils::writeArray(fOut, iW, iH, ppData3);
            /*
              for (int i = 0; i < iH; i++) {
              fwrite(ppData3[i], sizeof(T), iW, fOut);
              }
            */
        }
        fclose(fOut);
        iResult = bOK?0:-1;
    } else {
        printf("Couldn't open %s\n", pOut);
    }
    
    QMapUtils::deleteArray(iW, iH, ppData3);

    return iResult;
}


//-------------------------------------------------------------------------------------------------
// uniCombine
//  combine the QMaps with the specified number and operator and write to pOut
//
template<class T>
int  uniCombine(int iOp, ValReader *pVR1, T tOperand, char *pOut, T undef, QMapHeader *pQMH) {
    int iResult = 0;
    int iH = pVR1->getNRLat();
    int iW = pVR1->getNRLon();

    printf("[unicombine] %s %f %dx%d\n", sOps[iOp], tOperand*1.0, iW, iH);
    QMapReader<T> *pQMR1 =dynamic_cast<QMapReader<T> *> (pVR1);
    T **ppData1 = pQMR1->getData();
    T **ppData3 = new T*[iH];
    for (int i = 0; i < iH; i++) {
        ppData3[i] = new T[iW];
        for (int j = 0; j < iW; j++) {
            switch (iOp) {
            case OP_PLUS:
                ppData3[i][j] = ppData1[i][j]+tOperand;  
                break;
            case OP_MINUS:
                ppData3[i][j] = ppData1[i][j]-tOperand;  
                break;
            case OP_TIMES:
                ppData3[i][j] = ppData1[i][j]*tOperand;
                break;
            case OP_DIV:
                if (tOperand == 0) {
                    ppData3[i][j] = ppData1[i][j];
                } else {
                    ppData3[i][j] = ppData1[i][j]/tOperand;
                }
                break;
            case OP_MAX:
                if (!isnan(ppData1[i][j]) && !isnan(tOperand)) {
                    ppData3[i][j] = (ppData1[i][j]>=tOperand)?ppData1[i][j]:tOperand;
                } else {
                    ppData3[i][j] = undef;
                }
                break;
            case OP_MIN:
                if (!isnan(ppData1[i][j]) && !isnan(tOperand)) {
                    ppData3[i][j] = (ppData1[i][j]<=tOperand)?ppData1[i][j]:tOperand;
                } else {
                    ppData3[i][j] = undef;
                }
                break;
            case OP_EQ:
                ppData3[i][j] = (ppData1[i][j]==tOperand)?ppData1[i][j]:undef;
                break;
            case OP_THR:
                ppData3[i][j] = (ppData1[i][j]>=tOperand)?1:0;
                break;
            case OP_SET:
                ppData3[i][j] = tOperand;
                break;
            case OP_ISNAN:
                ppData3[i][j] = isnan(ppData1[i][j]);
                //                ppData3[i][j] = isnan(tOperand);
                break;
            case OP_SETNAN:
                ppData3[i][j] = isnan(ppData1[i][j])?tOperand:ppData1[i][j];
                break;
            case OP_INVDIV:
                ppData3[i][j] = tOperand/ppData1[i][j];
                break;
            case OP_LOG:
                ppData3[i][j] = log(ppData1[i][j]);
                break;
            default:
                ppData3[i][j] = 0;

            }
        }
    }
    
    // write output

    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {
        iResult = 0;
        pQMH->addHeader(fOut);
        for (int i = 0; (iResult == 0) && (i < iH); i++) {
            unsigned int iWritten = fwrite(ppData3[i], sizeof(T), iW, fOut);
            if (iWritten ==0) {    //!= sizeof(T)*iW) {
                iResult = -1;
            }
        }
        fclose(fOut);
    } else {
        printf("Couldn't open %s\n", pOut);
    }
    

    return iResult;
}

//-------------------------------------------------------------------------------------------------
// operate
//  call the correct binCombine depending on the type
//
int operate(int iOp, ValReader *pVR1, ValReader *pVR2, char *pOut, QMapHeader *pQMH) {
    int iResult = -1; 

    switch (pQMH->m_iType) {
    case QMAP_TYPE_UCHAR: {
        uchar uudef = 0xff;
        iResult = binCombine(iOp, pVR1, pVR2, pOut, uudef, pQMH);
    }
        break;
    case QMAP_TYPE_SHORT: {
        short int s = -1;
        iResult = binCombine(iOp, pVR1, pVR2, pOut, s, pQMH);
    }
        break;
    case QMAP_TYPE_INT : {
        int s = -1;
        iResult = binCombine(iOp, pVR1, pVR2, pOut, s, pQMH);
    }
        break;
    case QMAP_TYPE_LONG : {
        long s = -1;
        iResult = binCombine(iOp, pVR1, pVR2, pOut, s, pQMH);
    }
        break;
    case QMAP_TYPE_FLOAT: {
        iResult = binCombine(iOp, pVR1, pVR2, pOut, fNaN, pQMH);
    }
        break;
    case QMAP_TYPE_DOUBLE: {
        iResult = binCombine(iOp, pVR1, pVR2, pOut, dNaN, pQMH);
    }
        break;
    default:
        iResult = -1;
    }

    return iResult;
}


//-------------------------------------------------------------------------------------------------
// main
//  arguments: 
//    <QMap1> <OpName> <QMap2> <QmapOut>
//  or
//    <QMap1> <OpName> <number> <QmapOut>
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sInput1[LONG_INPUT];
    char sInput2[LONG_INPUT];
    
    if (iArgC > 4) {
        int iType1;
        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sInput1);
        searchFile(apArgV[3], pDataDir, sInput2);
        

        ValReader *pVR1 = QMapUtils::createValReader(sInput1, false, &iType1);
        if (pVR1 != NULL) {
            int iOpCode = getOperation(apArgV[2]);
            if (iOpCode >= 0) {
                QMapHeader *pQMH = new QMapHeader(iType1,
                                                  pVR1->getLonMin(), pVR1->getLonMax(), pVR1->getDLon(),
                                                  pVR1->getLatMin(), pVR1->getLatMax(), pVR1->getDLat(),
                                                  pVR1->getVName(),  pVR1->getXName(),  pVR1->getYName());

                
                int iType2;
                ValReader *pVR2 = QMapUtils::createValReader(sInput2, false, &iType2);
                if (pVR2 != NULL) {
                    iResult = 0;
                    // sizes must be equal
                    if ((pVR1->getNRLon() == pVR2->getNRLon()) && 
                        (pVR1->getNRLat() == pVR2->getNRLat())) {
                        printf("sizes: %dx%d\n", pVR1->getNRLon(), pVR1->getNRLat());
                        if (iType1 != iType2) {
                            printf("Different types: %s <->%s\n",  
                                   QMapHeader::getTypeName(iType1),  
                                   QMapHeader::getTypeName(iType2));
                            
                            printf("Result type will be %s\n",  QMapHeader::getTypeName(iType1>iType2?iType1:iType2));

                            if (iType1 < iType2) {
                                delete pVR1;
                                iResult = QConverter::convert(apArgV[1], TEMP_FILE, iType2);
                                printf("Conversion result1 of %s:%d\n", apArgV[1], iResult);
                                pVR1 = QMapUtils::createValReader(TEMP_FILE, false, &iType1);
                                iResult = (pVR1 != NULL)?0:-1;
                                pQMH->forceType(iType2);
                            } else if (iType2 < iType1) {
                            
                                delete pVR2;
                                iResult = QConverter::convert(apArgV[3], TEMP_FILE, iType1);
                                printf("Conversion result2 of >%s:%d\n", apArgV[3], iResult);
                                pVR2 = QMapUtils::createValReader(TEMP_FILE, false, &iType2);
                                iResult = (pVR2 != NULL)?0:-1;
                                printf("VR result2:%d (%p), %d\n", iResult, pVR2, (pVR2 != NULL));
                            }

                            remove(TEMP_FILE);

                        } 
                        
                        if (iResult == 0) {
                            iResult = operate(iOpCode, pVR1, pVR2, apArgV[4], pQMH);

                        } else {
                            printf("Problem with converting types\n");
                        }
                        if (iResult == 0) {
                            printf("success!\n");
                        }


                    } else {
                        printf("Different geometries: %dx%d <-> %dx%d\n", 
                               pVR1->getNRLon(),
                               pVR1->getNRLat(), 
                               pVR2->getNRLon(), 
                               pVR2->getNRLat());
                    }


                } else {
                    printf("2nd operand is number: %s (%s)\n", apArgV[3],QMapHeader::getTypeName(iType1));
                    // convert it to a number
                    switch (iType1) {
                    case QMAP_TYPE_UCHAR: {
                        uchar uudef = 0xff;
                        uchar uOp = (uchar) atoi(apArgV[3]);
                        iResult = uniCombine(iOpCode, pVR1, uOp, apArgV[4], uudef, pQMH);
                    }
                        break;
                    case QMAP_TYPE_SHORT: {
                        short int s = -1;
                        short int sOp = (short int) atoi(apArgV[3]);
                        iResult = uniCombine(iOpCode, pVR1, sOp, apArgV[4], s, pQMH);
                    }
                        break;
                    case QMAP_TYPE_INT : {
                        int s = -1;
                        int iOp =  atoi(apArgV[3]);
                        iResult = uniCombine(iOpCode, pVR1, iOp, apArgV[4], s, pQMH);
                    }
                        break;
                    case QMAP_TYPE_LONG : {
                        long s = -1;
                        long lOp =  atol(apArgV[3]);
                        iResult = uniCombine(iOpCode, pVR1, lOp, apArgV[4], s, pQMH);
                    }
                        break;
                    case QMAP_TYPE_FLOAT: {
                        float fOp = (float) atof(apArgV[3]);
                        iResult = uniCombine(iOpCode, pVR1, fOp, apArgV[4], fNaN, pQMH);
                    }
                        break;
                    case QMAP_TYPE_DOUBLE: {
                        double dOp = atof(apArgV[3]);
                        iResult = uniCombine(iOpCode, pVR1, dOp, apArgV[4], dNaN, pQMH);
                    }
                        break;
                    default:
                        iResult = -1;
                    }             

                }
                delete pQMH;
                if (iResult == 0) {
                    printf("+++ success +++\n");
                }

            } else {
                printf("%s is not an operator\n", apArgV[2]);
                
            }
        } else {
            printf("%s is not a QMap\n", apArgV[1]);
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
