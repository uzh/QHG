#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DynDEM.h"
#include "DynSeqDEM.h"

//----------------------------------------------------------------------------
// constructor
//
DynSeqDEM::DynSeqDEM(int iNumLonVals, int iNumLatVals, int iBufSize) 
:   DynDEM(iNumLonVals, iNumLatVals, iBufSize) {
 
    createBuffers();   
}



//----------------------------------------------------------------------------
// Load
//  expect coordinatres in degrees
//
bool DynSeqDEM::load(char *pFileName) {
    bool bOK = false;
    m_fData = fopen(pFileName, "rt");
    if (m_fData != NULL) {
        char sLine[256];
        int iLat = 0;
        int iLon = 0;

        m_aLineStarts[0] = getFilePos();        
printf("Loading geo data...\n");
printf("  CurLat: %4d", 0);        
        while (!feof(m_fData) && (iLat < m_iNumLatVals)) {
            char *p = fgets(sLine, 256, m_fData);
            if (p != NULL) {
                // not interested in return value
                splitLine(p);
            }
            ++iLon;
            if (iLon >= m_iNumLonVals) {
                iLon = 0;
                ++iLat;

                m_aLineStarts[iLat] = getFilePos();
                if (iLat%10 == 0) {
                    printf("\b\b\b\b%4d", iLat);fflush(stdout);
                }
            }
        }
printf("\b\b\b\b%4d\n", iLat);fflush(stdout);
printf("Done.\n");
        
        
        
//@@        printf("[StatDEM]#Lat:  %d\n", m_iNumLatVals);
//@@        printf("[StatDEM]#Lon:  %d\n", m_iNumLongVals);

        m_dDeltaLat  = (m_dMaxLat-m_dMinLat)/(m_iNumLatVals-1);
        m_dDeltaLon  = (m_dMaxLon-m_dMinLon)/(m_iNumLonVals-1);

//@@        printf("[StatDEM]Lat:  [%f, %f], D:%f\n", m_dMinLat, m_dMaxLat, m_dDeltaLat);
//@@        printf("[StatDEM]Lon:  [%f, %f], D:%f\n", m_dMinLon, m_dMaxLon, m_dDeltaLon);
        bOK = true;
    } else {
        bOK = false;
    }
    return bOK;
}

//----------------------------------------------------------------------------
// destructor
//
DynSeqDEM::~DynSeqDEM() {
}

