#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "DynDEM.h"


static int s_iNew=0;
static int s_iReuse=0;
static int s_iReorder=0;


//----------------------------------------------------------------------------
// constructor
//
DynDEM::DynDEM(int iNumLonVals, int iNumLatVals, int iBufSize) 
:   DEM(iNumLonVals, iNumLatVals),
    m_aLineStarts(NULL),
    m_aCurLines(NULL),
    m_aiCurLines(NULL),
    m_aiRowOrder(NULL),
    m_iBufSize(iBufSize) {
    
}


//----------------------------------------------------------------------------
// destructor
//
DynDEM::~DynDEM() {
    /*
printf("#new     : %d\n", s_iNew);
printf("#reuse   : %d\n", s_iReuse);
printf("#reorder : %d\n", s_iReorder);
    */
    if (m_aCurLines != NULL) {
        for (int i = 0; i < m_iBufSize; i++) {
            if (m_aCurLines[i] != NULL) {
                delete[] m_aCurLines[i];
            }
        }
        delete[] m_aCurLines;
    }
    if (m_aiCurLines != NULL) {
        delete[] m_aiCurLines;
    }
    if (m_aiRowOrder != NULL) {
        delete[] m_aiRowOrder;
    }
    if (m_aLineStarts != NULL) {
        delete[] m_aLineStarts;
    }
    if (m_fData != NULL) {
        fclose(m_fData);
    }
}

//----------------------------------------------------------------------------
// createBuffers
//
void DynDEM::createBuffers() {
    m_aLineStarts = new long[m_iNumLatVals+1]; // including file end pos
    m_aCurLines = new double*[m_iBufSize];
    m_aiCurLines = new int[m_iBufSize];
    m_aiRowOrder = new int[m_iBufSize];
    for (int i =0; i < m_iBufSize; i++) {
        m_aCurLines[i] = new double[m_iNumLonVals];
        //        memset(&(m_aCurLines[i]), 0, m_iNumLongVals*sizeof(double));
        m_aiCurLines[i] = -1;
        m_aiRowOrder[i] = i;
    }
    
}

//----------------------------------------------------------------------------
// readLongitudes
//  expect longitudes in file to be in degrees
//
void DynDEM::readLongitudes(int iLatIndex) {
    int iOldestIndex = m_aiRowOrder[m_iBufSize-1];

    fseek(m_fData, m_aLineStarts[iLatIndex], SEEK_SET);
    char sLine[256];
    int iLon = 0;

    while (!feof(m_fData) && (iLon < m_iNumLonVals)) {
        char *p = fgets(sLine, 256, m_fData);
        if (p != NULL) {
           m_aCurLines[iOldestIndex][iLon] = splitLine(p);
        }
        ++iLon;
    }
    m_aiCurLines[iOldestIndex] = iLatIndex;

    reorder(iOldestIndex);
}

//----------------------------------------------------------------------------
// getAltitude
//   lon, lat in degrees
//
double DynDEM::getAltitude(double dLon, double dLat) {
    double d = NO_VAL;
    int iLatIndex;
    int iLonIndex;

    if (findIndex(dLon, dLat, iLonIndex, iLatIndex)) {
        int iLineIndex = findLineIndex(iLatIndex);
        if (iLineIndex < 0) {
            readLongitudes(iLatIndex); // new row is first
            iLineIndex = m_aiRowOrder[0];        
            s_iNew++;
        } else {
            s_iReuse++;
        }
        d = m_aCurLines[iLineIndex][iLonIndex];
    } else {
        printf("- d=%f\n", d);
    }
    return d;
}


//----------------------------------------------------------------------------
// findLineIndex
//
// TODO: binary search
//
int DynDEM::findLineIndex(int iSearch) {
    int iRes = -1;
    for (int i = 0; (i < m_iBufSize) && (iRes < 0); i++) {
        if (m_aiCurLines[i] == iSearch) {
            iRes = i;
        }
    }
    return iRes;
}


//----------------------------------------------------------------------------
// reorder
//
void DynDEM::reorder(int iIndex) {
    s_iReorder++;
    for (int i = 0; i < m_iBufSize; i++) {
        --m_aiRowOrder[i];
        if (m_aiRowOrder[i] < 0) {
            m_aiRowOrder[i] = m_iBufSize-1;
        }
    }
}

//----------------------------------------------------------------------------
// getFilePos
//
unsigned long DynDEM::getFilePos() {
    fpos_t fPos;
    fgetpos(m_fData, &fPos);
    unsigned long lPos;
#ifdef CSCS
    lPos = fPos;
#else
    lPos = fPos.__pos;
#endif
    return lPos;
}
