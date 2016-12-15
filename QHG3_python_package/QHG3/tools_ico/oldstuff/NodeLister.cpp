#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "strutils.h"
#include "LineReader.h"
#include "SnapHeader.h"
#include "PopHeader.h"
#include "PopLoader.h"
#include "icoutil.h"
#include "NodeLister.h"

#define NUM_ITEMS 1024

int NodeLister::createList(const char *pFile, nodelist &mNodeValues, double *pdMin, double *pdMax) {
    int iResult = -1;
    *pdMin = dPosInf;
    *pdMax = dNegInf;
    LineReader *pLR = LineReader_std::createInstance(pFile, "rt");
    if (pLR != NULL) {
        // try snap
        SnapHeader *pSH = new SnapHeader();
        iResult = pSH->read(pLR, BIT_ICOFILE);
        if (iResult == 0) {
            
            if ((strlen(pSH->m_sDataDesc) == 2) && (pSH->m_sDataDesc[0] == 'l')) {
                char c = pSH->m_sDataDesc[1];
                gridtype lPos = pLR->tell();
                FILE *fIn = fopen(pFile, "rb"); // file should be "openable"
                fseek(fIn, lPos, SEEK_SET);
                iResult = createSnapList(fIn, c, mNodeValues, pdMin, pdMax);
                fclose(fIn);
            } else {
                printf("Don't support data format [%s]\n", pSH->m_sDataDesc);
                iResult = -1;
            }
        } else {
            pLR->seek(SEEK_SET, 0);
            PopHeader *pPH = new PopHeader();
            iResult = pPH->read(pLR);
            delete pPH;
            if (iResult == 0) {
                iResult = createPopList(pFile, mNodeValues);
            } else {
                printf("Nothing at all! [%s]\n", pSH->m_sError);
            }
        }
        delete pSH;
        
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pFile);
    }
    return iResult;
}


int NodeLister::createSnapList(FILE *fIn, char cType, nodelist &mNodeValues, double *pdMin, double *pdMax) {
    int iResult = 0;
    switch (cType) {
    case 'u':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(unsigned char), mNodeValues, pdMin, pdMax, (unsigned char) 0);
        break;
    case 's':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(short int), mNodeValues, pdMin, pdMax, (short int) 0);
        break;
    case 'i':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(int), mNodeValues, pdMin, pdMax, (int) 0);
        break;
    case 'l':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(long long), mNodeValues, pdMin, pdMax, (long long) 0);
        break;
    case 'f':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(float), mNodeValues, pdMin, pdMax,  (float) 0);
        break;
    case 'd':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(double), mNodeValues, pdMin, pdMax,  (double) 0);
        break;
    default:
        iResult = -1;
        printf("Unknown type char: [%c]\n", cType);
    }

    /*
    nodelist::iterator it;
    for (it = mNodeValues.begin(); it != mNodeValues.end(); ++it) {
        printf("%lld %f\n", it->first, it->second);
    }
    */
    return iResult;    
}

template<class T>
int NodeLister::createSnapList(FILE *fIn, int iItemSize, nodelist &mNodeValues, double *pdMin, double *pdMax, T tDummy) {
    int iResult   = 0;

    // create a buffer (size: multiple of 2*sizeof(coord)+iItemSize
    unsigned char *pBuffer = new unsigned char[NUM_ITEMS*iItemSize];
    while (!feof(fIn)) {
        int iRead = fread(pBuffer, iItemSize, NUM_ITEMS, fIn);
        unsigned char *pCur = pBuffer;
        T t;
        for (int i = 0; i < iRead; i++) {
            gridtype lNode;
            
            pCur = getMem(&lNode, pCur, sizeof(gridtype)); 
            pCur = getMem(&t, pCur, sizeof(T));
            //            printf("nl: %lld -> %f\n", lNode, (double) t);
            mNodeValues[lNode] = (double)t;
            if (t > *pdMax) {
                *pdMax = (double) t;
            }
            if (t < *pdMin) {
                *pdMin = (double) t;
            }
        }
    }
    delete[] pBuffer;
    return iResult;
}

int NodeLister::createPopList(const char *pFile, nodelist &mNodeValues) {
    int iResult = -1;
    return iResult;
}
