#include <stdio.h>
#include "QMapUtils.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapHeader.h"

#include "TopoTemp.h"

//------------------------------------------------------------------------------
// constructor
//
TopoTemp::TopoTemp(char *pAltFile, char *pTempFile) 
    : m_pQMRAltitude(NULL),
      m_pQMRTemp(NULL),
      m_ppData(NULL),
      m_iW(0),
      m_iH(0),
      m_bReady(false),
      m_bDelAltReader(true),
      m_bDelTempReader(true),
      m_bVerbose(false) {

    m_pQMRAltitude = QMapUtils::createValReader(pAltFile, true); // with interpolation
    if (m_pQMRAltitude != NULL) {
        m_pQMRTemp = QMapUtils::createValReader(pTempFile, true); // with interpolation
        if (m_pQMRTemp != NULL) {
            init();
        } else {
            printf("couldn't create reader for %s\n", pTempFile);
        }
    } else {
        printf("couldn't create reader for %s\n", pAltFile);
    }
}


//------------------------------------------------------------------------------
// constructor
//
TopoTemp::TopoTemp(ValReader *pAltReader, char *pTempFile) 
    : m_pQMRAltitude(pAltReader),
      m_pQMRTemp(NULL),
      m_ppData(NULL),
      m_iW(0),
      m_iH(0),
      m_bReady(false),
      m_bDelAltReader(false),
      m_bDelTempReader(true),
      m_bVerbose(false) {

    if (m_pQMRAltitude != NULL) {
        m_pQMRTemp = QMapUtils::createValReader(pTempFile, true); // with interpolation
        if (m_pQMRTemp != NULL) {
            init();
        } else {
            printf("couldn't create reader for %s\n", pTempFile);
        }
    } else {
        printf("got NULL for Alt reader\n");
    }
}

//------------------------------------------------------------------------------
// constructor
//
TopoTemp::TopoTemp(char *pAltFile, ValReader *pTempReader) 
    : m_pQMRAltitude(NULL),
      m_pQMRTemp(pTempReader),
      m_ppData(NULL),
      m_iW(0),
      m_iH(0),
      m_bReady(false),
      m_bDelAltReader(true),
      m_bDelTempReader(true),
      m_bVerbose(false) {

    m_pQMRAltitude = QMapUtils::createValReader(pAltFile, false); // no interpolation
    if (m_pQMRAltitude != NULL) {
        if (m_pQMRTemp != NULL) {
            init();
        } else {
            printf("got NULL for temp reader\n");
        }
    } else {
        printf("couldn't create reader for %s\n", pAltFile);
    }
}


//------------------------------------------------------------------------------
// constructor
//
TopoTemp::TopoTemp(ValReader *pAltReader, ValReader *pTempReader) 
    : m_pQMRAltitude(pAltReader),
      m_pQMRTemp(pTempReader),
      m_ppData(NULL),
      m_iW(0),
      m_iH(0),
      m_bReady(false),
      m_bDelAltReader(false),
      m_bDelTempReader(false),
      m_bVerbose(false) {

    if ((m_pQMRAltitude != NULL) && (m_pQMRTemp != NULL)) {
        init();
    } else {
        printf("got NULL for a reader\n");
    }
}

//------------------------------------------------------------------------------
// init
//
void TopoTemp::init() {
    m_iW = m_pQMRAltitude->getNRLon();
    m_iH = m_pQMRAltitude->getNRLat();
    
    m_ppData = new float*[m_iH];
    for (unsigned int i = 0; i < m_iH; i++) {
        m_ppData[i] = new float[m_iW]; 
    }
    m_bReady = true;
}

//------------------------------------------------------------------------------
// destructor
//
TopoTemp::~TopoTemp() {
    
    if (m_bDelAltReader && (m_pQMRAltitude != NULL)) {
        delete m_pQMRAltitude;
    }
    if (m_bDelTempReader && (m_pQMRTemp != NULL)) {
        delete m_pQMRTemp;
    }
    if (m_ppData != NULL) {
        for (unsigned int i = 0; i < m_iH; i++) {
            if (m_ppData[i] != NULL) {
                delete[] m_ppData[i];
            }
        }
        delete[] m_ppData;
    }
}

//------------------------------------------------------------------------------
// writeToFile
//
int TopoTemp::writeToFile(char *pOutputFile) {
    int iResult = -1;
    // create header
    FILE * fOut = fopen(pOutputFile, "wb");
    if (fOut != NULL) {
        QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_FLOAT,
                                          m_pQMRAltitude->getLonMin(),
                                          m_pQMRAltitude->getLonMax(), m_pQMRAltitude->getDLon(),
                                          m_pQMRAltitude->getLatMin(),
                                          m_pQMRAltitude->getLatMax(), m_pQMRAltitude->getDLat(),
                                          "Temp", "Lon", "Lat");
        bool bOK = pQMH->addHeader(fOut);
        
        delete pQMH;

        if (bOK) {
            iResult = 0;
            for (unsigned int iY = 0; (iResult == 0) && (iY < m_iH); iY++) {
                unsigned int iWritten = fwrite(m_ppData[iY], sizeof(float), m_iW, fOut);
                if (iWritten != m_iW) {
                    iResult = -2;
                }
            }
                    
        }
        fclose(fOut);
    }
    return iResult;

}
