#include <stdio.h>

#include "utils.h"
#include "types.h"
#include "ranges.h"
#include "QMapUtils.h"
#include "QMapHeader.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"

const double BAD_VAL = dNegInf;
const int    DEF_FIELD = 4;

//----------------------------------------------------------------------------
// createValReader
//   creates a QMapReader of the appropriate type for a file with BMAP header
//
ValReader *QMapUtils::createValReader(const char *pFile, bool bInterp,int *piDataType) {
    ValReader *pVR = NULL;
    
    int iType = QMapHeader::getQMapType(pFile);
    switch (iType) {
    case QMAP_TYPE_UCHAR:
        pVR =  new QMapReader<unsigned char>(pFile, bInterp); 
        break;
    case QMAP_TYPE_SHORT:
        pVR =  new QMapReader<short int>(pFile, bInterp); 
        break;
    case QMAP_TYPE_INT:
        pVR =  new QMapReader<int>(pFile, bInterp); 
        break;
    case QMAP_TYPE_FLOAT:
        pVR =  new QMapReader<float>(pFile, bInterp); 
        break;
    case QMAP_TYPE_DOUBLE:
        pVR =  new QMapReader<double>(pFile, bInterp); 
        break;
    default:
        pVR = NULL;
    }
    if (pVR != NULL) {
 
        bool bOK  = pVR->extractData();
        if (!bOK) {
            delete pVR;
            pVR = NULL;
        }
    }
    if (piDataType != NULL) {
        *piDataType = iType;
    }
    return pVR;    
}


//----------------------------------------------------------------------------
// createValReader
//   creates a QMapReader of the appropriate type for a file with BMAP header
//
ValReader *QMapUtils::createValReader(const char *pFile, const char *pRange, bool bPrepareArrays, bool bInterp,int *piDataType) {
    ValReader *pVR = NULL;
    
    int iType = QMapHeader::getQMapType(pFile);
    switch (iType) {
    case QMAP_TYPE_UCHAR:
        pVR =  new QMapReader<uchar>(pFile, pRange, bPrepareArrays, bInterp); 
        break;
    case QMAP_TYPE_SHORT:
        pVR =  new QMapReader<short int>(pFile, pRange, bPrepareArrays, bInterp); 
        break;
    case QMAP_TYPE_INT:
        pVR =  new QMapReader<int>(pFile, pRange, bPrepareArrays, bInterp); 
        break;
    case QMAP_TYPE_FLOAT:
        pVR =  new QMapReader<float>(pFile, pRange, bPrepareArrays, bInterp); 
        break;
    case QMAP_TYPE_DOUBLE:
        pVR =  new QMapReader<double>(pFile, pRange, bPrepareArrays, bInterp); 
        break;
    default:
        pVR = NULL;
    }
    if ((pVR != NULL) && bPrepareArrays) {
        
        bool bOK  = pVR->extractData();
        if (!bOK) {
            delete pVR;
            pVR = NULL;
        }
        
    }
    if (piDataType != NULL) {
        *piDataType = iType;
    }
    return pVR;    
}
