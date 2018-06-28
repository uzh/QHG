#include <stdio.h>
#include <hdf5.h>
#include <vector>

#include "SCellGrid.h"
#include "QDFUtils.h"
#include "GridWriter.h"
#include "PopWriter.h"
#include "GeoWriter.h"
#include "ClimateWriter.h"
#include "VegWriter.h"
#include "MoveStatWriter.h"
#include "NavWriter.h"
#include "StatusWriter.h"

//-----------------------------------------------------------------------------
// createInstance
//
StatusWriter *StatusWriter::createInstance(SCellGrid *pCG, std::vector<PopBase *> vPops) {
    StatusWriter *pSW = new StatusWriter();
    int iResult = pSW->init(pCG, vPops);
    if (iResult < 0) {
        delete pSW; 
        pSW = NULL;
    }
    return pSW;
}


//-----------------------------------------------------------------------------
// destructor
//
StatusWriter::~StatusWriter() {
    if (m_pPopW != NULL) {
        delete m_pPopW;
    }
    if (m_pGridW != NULL) {
        delete m_pGridW;
    }

    if (m_pGeoW != NULL) {
        delete m_pGeoW;
    }

    if (m_pCliW != NULL) {
        delete m_pCliW;
    }

    if (m_pVegW != NULL) {
        delete m_pVegW;
    }

    if (m_pMStatW != NULL) {
        delete m_pMStatW;
    }

    if (m_pNavW != NULL) {
        delete m_pNavW;
    }
}


//-----------------------------------------------------------------------------
// constructor
//
StatusWriter::StatusWriter()    
    : m_hFile(H5P_DEFAULT),
      m_pPopW(NULL),
      m_pGridW(NULL),
      m_pGeoW(NULL),
      m_pCliW(NULL),
      m_pVegW(NULL),
      m_pMStatW(NULL),
      m_pNavW(NULL) {
    m_sError = "";
}


//-----------------------------------------------------------------------------
// init
//
int StatusWriter::init(SCellGrid *pCG, std::vector<PopBase *> vPops) {
    int iResult = 0;
    if (vPops.size() != 0) {
        m_pPopW   = new PopWriter(vPops);
    }
    m_pGridW  = new GridWriter(pCG, &pCG->m_smSurfaceData);
    m_pGeoW   = new GeoWriter(pCG->m_pGeography);
    m_pCliW   = new ClimateWriter(pCG->m_pClimate);
    m_pVegW   = new VegWriter(pCG->m_pVegetation);
    m_pMStatW = new MoveStatWriter(pCG->m_pMoveStats);
    m_pNavW   = new NavWriter(pCG->m_pNavigation);

    return iResult;
}


//-----------------------------------------------------------------------------
// write
//   write elements specified by iWhat=ored combination of WR_XXX constants
//
int StatusWriter::write(const char*pFileName, float fTime, int iWhat, int iDumpMode) {
    int iResult = write(pFileName, fTime, NULL, iWhat, iDumpMode);
    return iResult;
}


//-----------------------------------------------------------------------------
// write
//   write elements specified by iWhat=ored combination of WR_XXX constants
//
int StatusWriter::write(const char*pFileName, float fTime, char *pSub, int iWhat, int iDumpMode) {
    int iResult = iWhat;
    int iTemp = 0;
    m_hFile = qdf_createFile(pFileName, fTime);
    if (m_hFile > 0) {
        if ((iWhat & WR_POP) != 0 && (m_pPopW != NULL)) {
            printf("writing pop\n");
            iTemp = m_pPopW->write(m_hFile, pSub, iDumpMode);
            iResult -= (iTemp == 0)?WR_POP:0;
        }
        if ((iWhat & WR_GRID) != 0) {
            iTemp = m_pGridW->write(m_hFile);
            iResult -= (iTemp == 0)?WR_GRID:0;
        }
        if ((iWhat & WR_GEO) != 0) {
            iTemp = m_pGeoW->write(m_hFile);
            iResult -= (iTemp == 0)?WR_GEO:0;
        }
        if ((iWhat & WR_CLI) != 0) {
            iTemp = m_pCliW->write(m_hFile);
            iResult -= (iTemp == 0)?WR_CLI:0;
        }
        if ((iWhat & WR_VEG) != 0) {
            iTemp = m_pVegW->write(m_hFile);
            iResult -= (iTemp == 0)?WR_VEG:0;
        }
        if ((iWhat & WR_STAT) != 0) {
            iTemp = m_pMStatW->write(m_hFile);
            iResult -= (iTemp == 0)?WR_STAT:0;
        }
        if ((iWhat & WR_NAV) != 0) {
            iTemp = m_pNavW->write(m_hFile);
            iResult -= (iTemp == 0)?WR_NAV:0;
        }

        // report unwritten stuff
        if (iResult > 0) {
            m_sError = "Warning : Couldn't write: ";
            
            if ((iResult & WR_POP) != 0) {
                m_sError += " pop";
            }
            if ((iResult & WR_GRID) != 0) {
                m_sError += " grid";
            }
            if ((iResult & WR_GEO) != 0) {
                m_sError += " geo";
            }
            if ((iResult & WR_CLI) != 0) {
                m_sError += " climate";
            }
            if ((iResult & WR_VEG) != 0) {
                m_sError += " veg";
            }
            if ((iResult & WR_STAT) != 0) {
                m_sError += " stat";
            }
            if ((iResult & WR_NAV) != 0) {
                m_sError += " nav";
            }

            if (iResult == iWhat) {
                iResult = -1;
            }
        }

        qdf_closeFile(m_hFile);  
        m_hFile = H5P_DEFAULT;
    }
    return iResult;
}

