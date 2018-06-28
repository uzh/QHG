#include <stdio.h>
#include <string.h>

#include "types.h"
#include "utils.h"
#include "strutils.h"

#include "Geography.h"

#include "IcoGridNodes.h"
#include "IcoNode.h"

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
    m_abCoastal    = new bool[m_iNumCells];      
    memset(m_adWater, 0, m_iNumCells*sizeof(double));

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

    if (m_abCoastal != NULL) {
        delete[] m_abCoastal;
    }

    if (m_adAngles != NULL) {
        delete[] m_adAngles;
    }
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
    

