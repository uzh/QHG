#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include "strutils.h"
#include "QDFUtils.h"
#include "QDF2MoveStatSnap.h"

#define BUF_SIZE 65536

//----------------------------------------------------------------------------
// constructor
//
QDF2MoveStatSnap::QDF2MoveStatSnap() 
    : QDF2SnapBase(),
      m_pParam(NULL) {


};

//----------------------------------------------------------------------------
// destructor
//
QDF2MoveStatSnap::~QDF2MoveStatSnap() {
}


//----------------------------------------------------------------------------
// createInstance
//
QDF2MoveStatSnap *QDF2MoveStatSnap::createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    QDF2MoveStatSnap *pQM = new QDF2MoveStatSnap();
    int iResult = pQM->init(hFile, iNumCells, pAttr, pParam);
    if (iResult != 0) {
        delete pQM;
        pQM = NULL;
    }
    return pQM;
}

//----------------------------------------------------------------------------
// init
//
int QDF2MoveStatSnap::init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    int iResult = QDF2SnapBase::init(hFile, iNumCells, VEGGROUP_NAME, pAttr);

    if (iResult == 0) {
        m_pParam = pParam;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillSnapData
//
int QDF2MoveStatSnap::fillSnapData(char *pBuffer) {
    int iResult = -1;
    if (strcasecmp(m_pAttr, MSTAT_DS_HOPS) == 0) {
        iResult = fillSimpleSnapDataInt(pBuffer);
    } else {
        iResult = fillSimpleSnapDataDouble(pBuffer);
    }
    return iResult;
}


