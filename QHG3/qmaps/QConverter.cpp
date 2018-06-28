#include "QMapReader.h"
#include "QMapHeader.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"

#include "QConverter.h"

int QConverter::convert(const char *pInput, const char *pOutput, int iTypeOut) {
    printf("Converting %s to type %d\n", pInput, iTypeOut);
    int iResult = -1;
    switch (iTypeOut) {
    case QMAP_TYPE_UCHAR: 
        iResult = convertToType(pInput, pOutput, iTypeOut, (uchar) 0xfe);
        break;
    case QMAP_TYPE_SHORT: 
        iResult = convertToType(pInput, pOutput, iTypeOut, (short int) 0x8000);
        break;
    case QMAP_TYPE_INT:
        iResult = convertToType(pInput, pOutput, iTypeOut, (int) 0x80000000);
        break;
    case QMAP_TYPE_LONG:
        iResult = convertToType(pInput, pOutput, iTypeOut, (long) 0x8000000000000000);
        break;
    case QMAP_TYPE_FLOAT: 
        iResult = convertToType(pInput, pOutput, iTypeOut, fNaN);
        break;
    case QMAP_TYPE_DOUBLE: 
        iResult = convertToType(pInput, pOutput, iTypeOut, dNaN);
        break;
    default:
        iResult = -3;
    }
    return iResult;
}


template <class T, class U>
int QConverter::convertAndWrite(QMapHeader *pQMH, T **aatData, T tDef, U uDef, const char *pOutput) {

    int iW = pQMH->m_iWidth;
    int iH = pQMH->m_iHeight;

    printf("[converter]size: %dx%d\n", iW, iH);
    U **aauData = QMapUtils::createArray(iW, iH, uDef);
    for (int i = 0; i < iH; i++) {
        for (int j = 0; j < iW; j++) {
            T tCur = aatData[i][j];
            U uCur = 0;
            if (!isfinite(tCur) || (tCur == uDef)) {
                uCur = uDef;
            } else {
                uCur = (U) tCur;
            }
            aauData[i][j] = uCur;
        }
    }
   
    FILE *fOut = fopen(pOutput, "wb");
    pQMH->addHeader(fOut);
    bool bOK =QMapUtils::writeArray(fOut, iW, iH, aauData);
    QMapUtils::deleteArray(iW, iH, aauData);
    fclose(fOut);
    return bOK?0:-1;
}


template<class U>
int QConverter::convertToType(const char *pInput, const char *pOutput, int iTypeOut, U uDef) {
    
    QMapHeader *pQMH = new QMapHeader();
    int iResult = pQMH->readHeader(pInput);
    if (iResult == 0) {
        ValReader *pVR0 = QMapUtils::createValReader(pInput, false);
        int iTypeIn = pQMH->m_iType;
        iResult = -2;
        switch (iTypeIn) {
        case QMAP_TYPE_UCHAR: {
            QMapReader<uchar> *pVR = dynamic_cast<QMapReader<uchar>*>(pVR0); 
            if (pVR != NULL) {
                uchar **auData = pVR->getData();
                pQMH->m_iType = iTypeOut;
                iResult = convertAndWrite(pQMH, auData, (uchar) 0xfe, uDef, pOutput);
            }
            break;
        }
        case QMAP_TYPE_SHORT: {
            QMapReader<short int> *pVR =  dynamic_cast<QMapReader<short int>*>(pVR0);
            if (pVR != NULL) {
                short int **asData = pVR->getData();
                pQMH->m_iType = iTypeOut;
                iResult = convertAndWrite(pQMH, asData, (short int) 0, uDef, pOutput);
            }
            break;
        }
        case QMAP_TYPE_INT: {
            QMapReader<int> *pVR =  dynamic_cast<QMapReader<int>*>(pVR0);
            if (pVR != NULL) {
                int **aiData = pVR->getData();
                pQMH->m_iType = iTypeOut;
                iResult = convertAndWrite(pQMH, aiData, 0, uDef, pOutput);
            }
            break;
        }
        case QMAP_TYPE_LONG: {
            QMapReader<long> *pVR =  dynamic_cast<QMapReader<long>*>(pVR0);
            if (pVR != NULL) {
                long **alData = pVR->getData();
                pQMH->m_iType = iTypeOut;
                iResult = convertAndWrite(pQMH, alData, 0L, uDef, pOutput);
            }
            break;
        }
        case QMAP_TYPE_FLOAT: {
            QMapReader<float> *pVR =  dynamic_cast<QMapReader<float>*>(pVR0);
            if (pVR != NULL) {
                float **afData = pVR->getData();
                pQMH->m_iType = iTypeOut;
                iResult = convertAndWrite(pQMH, afData, fNaN, uDef, pOutput);
            }
            break;
        }
        case QMAP_TYPE_DOUBLE: {
            QMapReader<double> *pVR =  dynamic_cast<QMapReader<double>*>(pVR0);
            if (pVR != NULL) {
                double **adData = pVR->getData();
                pQMH->m_iType = iTypeOut;
                iResult = convertAndWrite(pQMH, adData, dNaN, uDef, pOutput);
            }
            break;
        }
        default:
            iResult = -1;
        }
        
    }
    return iResult;
}

