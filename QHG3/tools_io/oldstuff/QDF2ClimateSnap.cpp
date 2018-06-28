#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include "strutils.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDF2SnapBase.h"
#include "QDF2ClimateSnap.h"

#define BUF_SIZE 65536

//----------------------------------------------------------------------------
// constructor
//
QDF2ClimateSnap::QDF2ClimateSnap() 
    : QDF2SnapBase(),
      m_pParam(NULL) {


};

//----------------------------------------------------------------------------
// destructor
//
QDF2ClimateSnap::~QDF2ClimateSnap() {
}


//----------------------------------------------------------------------------
// createInstance
//
QDF2ClimateSnap *QDF2ClimateSnap::createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    QDF2ClimateSnap *pQG = new QDF2ClimateSnap();
    int iResult = pQG->init(hFile, iNumCells, pAttr, pParam);
    if (iResult != 0) {
        delete pQG;
        pQG = NULL;
    }
    return pQG;
}

//----------------------------------------------------------------------------
// init
//
int QDF2ClimateSnap::init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    int iResult = QDF2SnapBase::init(hFile, iNumCells, CLIGROUP_NAME, pAttr);

    if (iResult == 0) {
        m_pParam = pParam;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillSnapData
//
int QDF2ClimateSnap::fillSnapData(char *pBuffer) {
    int iResult = -1;
    if ((strcasecmp(m_pAttr, CLI_DS_ACTUAL_TEMPS) == 0) ||
        (strcasecmp(m_pAttr, CLI_DS_ACTUAL_RAINS) == 0) ||
        (strcasecmp(m_pAttr, CLI_DS_ANN_MEAN_TEMP) == 0) ||
        (strcasecmp(m_pAttr, CLI_DS_ANN_TOT_RAIN) == 0)) {
        iResult = fillSimpleSnapDataDouble(pBuffer);
    } else {
        printf("Unknown attribute [%s]\n", m_pAttr);
    }
    return iResult;
}
