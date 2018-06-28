#include <stdio.h>
#include <string.h>

#include "strutils.h"
#include "Surface.h"
#include "EQsahedron.h"
#include "GridGroupReader.h"
#include "QDFUtils.h"
#include "SCellGrid.h"

#include "SurfaceGrid.h"

//----------------------------------------------------------------------------
// createInstance
//
SurfaceGrid *SurfaceGrid::createInstance(const char *pQDF) {
    SurfaceGrid *pSG = new SurfaceGrid();
    int iResult = pSG->init(pQDF);
    if (iResult != 0) {
        delete pSG;
        pSG = NULL;
    }
    return pSG;
}


//----------------------------------------------------------------------------
// constructor
//
SurfaceGrid::SurfaceGrid() 
    : m_pCG(NULL),
      m_pSurf(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
SurfaceGrid::~SurfaceGrid() {

    if (m_pCG != NULL) {
        // delete[]  m_pCG->m_aCells;
        delete m_pCG->m_pGeography;
        delete m_pCG->m_pClimate;
        delete m_pCG->m_pVegetation;
        delete m_pCG;
    }

    if (m_pSurf != NULL) {
        delete m_pSurf;
    }
}


//----------------------------------------------------------------------------
// init
//
int SurfaceGrid::init(const char *pQDF) {
    int iResult = 0;

    iResult = createCellGrid(pQDF);
    if (iResult == 0) {
        iResult = createSurface();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createCelLGrid
//
int SurfaceGrid::createCellGrid(const char *pQDF) {
    int iResult = -1;
    hid_t hFile = qdf_openFile(pQDF);

    GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
    if (pGR != NULL) {
        GridAttributes gridatt;
        char sTime[32];
        // get the timestamp of the initial qdf file (grid)
        iResult = qdf_extractSAttribute(hFile,  ROOT_TIME_NAME, 31, sTime);
        if (iResult != 0) {
            printf("[createCellGrid] Couldn't read time attribute from grid file\n");
            iResult = 0;
        } else {
            float fStartTime;
            if (strToNum(sTime, &fStartTime)) {
                iResult = 0;
                //                printf("Have timestamp %f\n", fStartTime);
            } else {
                printf("[createCellGrid] Timestamp not valid [%s]\n", sTime);
                iResult = -1;
            }
        }

        if (iResult == 0) {
            iResult = pGR->readAttributes(&gridatt);
            if (iResult == 0) {
                m_pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
                m_pCG->m_aCells = new SCell[gridatt.m_iNumCells];
                iResult = pGR->readData(m_pCG);

                if (iResult == 0) {
                    //                    printf("[setGrid] Grid read successfully: %p\n", m_pCG);
                }
            } else {
                printf("[createCellGrid] GridReader couldn't read  celldata\n");
            }
            if (iResult != 0) {
                delete m_pCG;
                m_pCG = NULL;
            }
        } else {
            printf("[createCellGrid] GridReader couldn't read attributes\n");
        }
    
        delete pGR;
        
    } else {
        printf("[createCellGrid] Couldn't create GridReader\n");
    }


    qdf_closeFile(hFile);

    return iResult;
}


//-----------------------------------------------------------------------------
// createSurface
//
int SurfaceGrid::createSurface() {
    int iResult = -1;

    if (m_pSurf == NULL) {
        if (m_pCG != NULL) {
            stringmap &smSurf = m_pCG->m_smSurfaceData;
            const char *pSurfType = smSurf["SURF_TYPE"].c_str();


            if (strcmp(pSurfType, SURF_EQSAHEDRON) == 0) {
                int iSubDivs = -1;
                const char *pSubDivs = smSurf[SURF_IEQ_SUBDIVS].c_str();
                if (strToNum(pSubDivs, &iSubDivs)) {
                    if (iSubDivs >= 0) {
                        EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true, NULL);
                        if (pEQ != NULL) {
                            pEQ->relink();
                            m_pSurf = pEQ;
                            iResult = 0;
                            //printf("[createSurface] Have EQsahedron\n");
                        }
                    } else {
                        printf("[createSurface] subdivs must be positive [%s]\n", pSubDivs);
                    }
                } else {
                    printf("[createSurface] subdivs is not a number [%s]\n", pSubDivs);
                }

            } else {
                printf("[createSurface] unknown surface type [%s] - we only do EQsahedron\n", pSurfType);
            }
            
        } else {
            printf("[createSurface] can't create surface without CellGrid data\n");
        }
    }
    return iResult;
};


