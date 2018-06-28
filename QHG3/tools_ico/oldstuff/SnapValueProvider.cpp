#include <stdio.h>
#include <string.h>
#include <map>

#include "icoutil.h"
#include "utils.h"
#include "strutils.h"
#include "LineReader.h"
#include "SnapHeader.h"
#include "ValueProvider.h"
#include "SnapValueProvider.h"

#define NUM_ITEMS 1024

//----------------------------------------------------------------------------
// createSnapData
//
ValueProvider *SnapValueProvider::createValueProvider(const char *pFile) {
    ValueProvider *pVP = new SnapValueProvider();
    int iResult = pVP->init(pFile);
    if (iResult != 0) {
        delete pVP;
        pVP = NULL;
    }
    return pVP;
}

//----------------------------------------------------------------------------
// constructor
//
SnapValueProvider::SnapValueProvider()  {

}

//----------------------------------------------------------------------------
// 
//
int SnapValueProvider::init(const char *pFile) {
    int iResult = -1;
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
                fseek(fIn, lPos,  SEEK_SET);
                iResult = createSnapList(fIn, c);
                fclose(fIn);
                if (iResult == 0) {
                    m_iDataFileType = DATA_FILE_TYPE_LIST;
                }
            } else {
                printf("Don't support data format [%s]\n", pSH->m_sDataDesc);
                iResult = -1;
            }
        } else {
            printf("No Snap File? [%s]\n", pFile);
        }
        delete pSH;
        
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pFile);
    }
    return iResult;

}

//----------------------------------------------------------------------------
// getValue
//
double SnapValueProvider::getValue(gridtype lID) {
    double dVal = dNaN;
    std::map<gridtype, double>::const_iterator it = m_mNodeValues.find(lID);
    if (it != m_mNodeValues.end()) {
        dVal = it->second;
        //                        printf("have ID %lld\n", lID);
    } else {
        dVal = dNaN;
        //                printf("missing ID %lld\n", lID);
    }
    return dVal;
}
    
//----------------------------------------------------------------------------
// createSnapList
//
int SnapValueProvider::createSnapList(FILE *fIn, char cType) {
    int iResult = 0;
    switch (cType) {
    case 'u':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(unsigned char), (unsigned char) 0);
        break;
    case 's':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(short int), (short int) 0);
        break;
    case 'i':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(int), (int) 0);
        break;
    case 'l':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(long long), (long long) 0);
        break;
    case 'f':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(float),  (float) 0);
        break;
    case 'd':
        iResult = createSnapList(fIn, sizeof(gridtype)+sizeof(double),  (double) 0);
        break;
    default:
        iResult = -1;
        printf("Unknown type char: [%c]\n", cType);
    }

   return iResult;    
}

//----------------------------------------------------------------------------
// createSnapList
//
template<class T>
int SnapValueProvider:: createSnapList(FILE *fIn, int iItemSize, T tDummy) {
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
            double dV = (double) t;
            m_mNodeValues[lNode] = dV;
            if (t > m_dMaxData) {
                m_dMaxData = dV;
            }
            if (t < m_dMinData) {
                m_dMinData = dV;
            }
        }
    }
    delete[] pBuffer;
    return iResult;

}
