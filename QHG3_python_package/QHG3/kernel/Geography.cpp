#include <stdio.h>
#include <string.h>

#include "types.h"
#include "utils.h"
#include "strutils.h"

#include "Geography.h"

#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "QMapHeader.h"
#include "SnapHeader.h"
#include "SCellGrid.h"

#ifndef NULL 
  #define NULL 0
#endif


//-----------------------------------------------------------------------------
// constructor
//
Geography::Geography(uint iNumCells, uint iMaxNeighbors, geonumber dRadius, geonumber dSeaLevel) 
    : m_bUpdated(true),
	  m_iNumCells(iNumCells),
      m_iMaxNeighbors(iMaxNeighbors),
      m_dRadius(dRadius),
      m_dSeaLevel(0), 
      m_adLatitude(NULL),
      m_adLongitude(NULL),
      m_adAltitude(NULL), 
      m_adDistances(NULL),
      m_adArea(NULL),     
      m_abIce(NULL),
      m_adWater(NULL),     
      m_adAngles(NULL) {
    
    init(iNumCells, iMaxNeighbors, dRadius, dSeaLevel);
}
//-----------------------------------------------------------------------------
// constructor
//
Geography::Geography() 
    : m_iNumCells(0),
      m_iMaxNeighbors(0),
      m_dRadius(0),
      m_dSeaLevel(0), 
      m_adLatitude(NULL),
      m_adLongitude(NULL),
      m_adAltitude(NULL), 
      m_adDistances(NULL),
      m_adArea(NULL),     
      m_abIce(NULL),
      m_adWater(NULL),     
      m_adAngles(NULL) {
    
}

//-----------------------------------------------------------------------------
// destructor
//
int Geography::init(uint iNumCells, uint iMaxNeighbors, geonumber dRadius, geonumber dSeaLevel) {
    m_iNumCells     = iNumCells;
    m_iMaxNeighbors = iMaxNeighbors;
    m_dRadius       = dRadius;
    m_dSeaLevel     = dSeaLevel;

    m_adLatitude   = new geonumber[m_iNumCells];
    m_adLongitude  = new geonumber[m_iNumCells];
    m_adAltitude   = new geonumber[m_iNumCells]; 
    m_adDistances  = new geonumber[m_iNumCells*m_iMaxNeighbors];
    m_adArea       = new geonumber[m_iNumCells];     
    m_abIce        = new bool[m_iNumCells];      
    m_adWater      = new geonumber[m_iNumCells];     
    //    m_adAngles     = new geonumber[m_iNumCells*m_iMaxNeighbors];
    for(int i=0; i<6; i++) {
        m_adQMapHeadData[i] = 0;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// destructor
//
Geography::~Geography() {
    if (m_adLatitude != NULL) {
        delete[] m_adLatitude;
    }
    if (m_adLongitude != NULL) {
        delete[] m_adLongitude;
    }

    if (m_adAltitude != NULL) {
        delete[] m_adAltitude;
    }

    if (m_adDistances != NULL) {
        delete[] m_adDistances;
    }

    if (m_adArea != NULL) {
        delete[] m_adArea;
    }

    if (m_abIce != NULL) {
        delete[] m_abIce;
    }

    if (m_adWater != NULL) {
        delete[] m_adWater;
    }

    if (m_adAngles != NULL) {
        delete[] m_adAngles;
    }
}


//-----------------------------------------------------------------------------
//  outputGeo
//  writes snap or qmap of altitudes
//
void Geography::writeOutput(float fTime, int iStep, SCellGrid* pCG) {

    if (m_dRadius == 0) {  // if grid is flat, output QMAP directly
        writeAltQMap(fTime, iStep);
    } else {  // if grid is ico, output SNAP to view with IQApp
        writeAltSnap(fTime, iStep, pCG);
    }
    
}


//-----------------------------------------------------------------------------
//  writeGeoQMap
//
void Geography::writeAltQMap(float fTime, int iStep) {
    
    QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_DOUBLE,
                                      m_adQMapHeadData,
                                      "Alt", "X", "Y"); 
    
    char sOutFile[128]; 
    sprintf(sOutFile,"alt_%05d.qmap",iStep);

    FILE *fOut = fopen(sOutFile, "wb");

    if (fOut != NULL) {

        pQMH->addHeader(fOut);
        size_t iW = fwrite(m_adAltitude, sizeof(geonumber), m_iNumCells, fOut);
        if (iW < m_iNumCells) {
            printf("Error at writeALtQMap\n");
        }
        fclose(fOut);

    } else {
        fprintf(stderr,"writing file %s failed\n",sOutFile);
    }
}


//-----------------------------------------------------------------------------
// writeSnapGeo
//  only to test correct creation of snap files
//
void Geography::writeAltSnap(float fTime, int iStep, SCellGrid* pCG) {
    SnapHeader *pSH = new SnapHeader("3.0", COORD_NODE, iStep , fTime, "ld", "dummy.ico",false, 7774, "Alt",0,NULL);
    char sOut[128];
    sprintf(sOut, "geo_%05d.snap", iStep);
    FILE *fOut = fopen(sOut, "wb");
    pSH->write(fOut, true);

    uchar *pBuffer = new uchar[m_iNumCells*(sizeof(gridtype)+1*sizeof(geonumber))];
    uchar *p = pBuffer;
    for (uint j = 0; j < m_iNumCells; j++) {
        p = putMem(p, &(pCG->m_aCells[j].m_iGlobalID), sizeof(gridtype));
        p = putMem(p, &(m_adAltitude[j]), sizeof(geonumber));
        //        p = putMem(p, &(m_pGeography->m_adArea[j]), sizeof(geonumber));
    }

    size_t iW = fwrite(pBuffer, sizeof(gridtype)+sizeof(double), m_iNumCells, fOut);
    if (iW < m_iNumCells) {
        printf("Error at writeAltSnap\n");
    }
    fclose(fOut);

}

#define EPS 1.0e-8
//-----------------------------------------------------------------------------
// calcAngles
//  calculate orientation of the directions:
//    0     : east
//    pi/2  : north
//    pi    : west
//   -pi/2  : south
//
void Geography::calcAngles(SCellGrid* pCG) {
    m_adAngles = new geonumber[m_iNumCells*m_iMaxNeighbors];
    memset(m_adAngles, 111, m_iNumCells*m_iMaxNeighbors*sizeof(geonumber));
    for (uint i = 0; i < m_iNumCells; i++) {
        int iIndex = pCG->m_mIDIndexes[i];
        SCell &sc = pCG->m_aCells[iIndex];
        /*********/
        double dLon0 = m_adLongitude[iIndex];
        double dLat0 = m_adLatitude[iIndex];
        if (fabs(dLat0) > 90-EPS) {
            for (uint j  = 0; j < m_iMaxNeighbors; j++) {
                m_adAngles[iIndex*m_iMaxNeighbors+j] = dNaN;
            }
        } else {

            for (uint j  = 0; j < m_iMaxNeighbors; j++) {
                if (j < sc.m_iNumNeighbors) {
                    // sc.m_aNeighbors: indexes, not IDs
                    int k = sc.m_aNeighbors[j];
                    double dLon = (m_adLongitude[k] - dLon0)*M_PI/180;
                    if (dLon > M_PI) {
                        dLon -= 2*M_PI;
                    } else if (dLon < -M_PI) {
                        dLon += 2*M_PI;
                    } 
                    
                    double dLat = (m_adLatitude[k]  - dLat0)*M_PI/180;
                    m_adAngles[iIndex*m_iMaxNeighbors+j] = atan2(dLat, dLon);
                } else {
                    m_adAngles[iIndex*m_iMaxNeighbors+j] = dNaN;
                }
            }
        }
    }

}
    

