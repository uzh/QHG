#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DynDEM.h"
#include "DynXYZDEM.h"


//----------------------------------------------------------------------------
// constructor
//
DynXYZDEM::DynXYZDEM(int iBufSize) 
:   DynDEM(0, 0, iBufSize),
    m_iRecordSize(0) {
}


//----------------------------------------------------------------------------
// load
//  expect coordinates to be given in degrees
//
bool DynXYZDEM::load(char *pFileName) {
    bool bOK = false;
    m_fData = fopen(pFileName, "rt");
    if (m_fData != NULL) {
        double dDeltaLonTemp = 0;
        char sLine[256];

        printf("Loading geo data...\n");
        
        // read first line to get minlon and max lat
        char *p = fgets(sLine, 256, m_fData);
        if (p != NULL) {
            // we're not interested in the return value
            splitLine(p);
        
        
            // get position: record length
            m_iRecordSize = getFilePos();

            // read second line -> delta lat
            p = fgets(sLine, 256, m_fData);
            if (p != NULL) {
                splitLine(p);  // ignore return value
                dDeltaLonTemp = m_dMaxLon - m_dMinLon;
            

                // go to end : file size -> Num recs
                fseek(m_fData, 0, SEEK_END);
                unsigned long lNumRecs = getFilePos()/m_iRecordSize;

                // read last line: max lon, min lat
                fseek(m_fData, -m_iRecordSize, SEEK_END);
                p = fgets(sLine, 256, m_fData);
                if (p != NULL) {
                    splitLine(p);  // ignore return value
                    printf("num recs: %ld\n", lNumRecs);
                    m_iNumLonVals = 1+ (int)((m_dMaxLon - m_dMinLon)/dDeltaLonTemp);
                    m_iNumLatVals = lNumRecs / m_iNumLonVals; 

                    // now prepare buffers        
                    createBuffers();

                    for (int iLat = 0; iLat <= m_iNumLatVals; iLat++) {
                        m_aLineStarts[iLat] = m_iNumLonVals*m_iRecordSize*iLat;    
                    }     

                    m_dDeltaLat = (m_dMaxLat-m_dMinLat)/(m_iNumLatVals-1);
                    m_dDeltaLon = (m_dMaxLon-m_dMinLon)/(m_iNumLonVals-1);

                    bOK = true;
                } else {
                    printf("Couldn't read last line of DEM-File %s\n", pFileName);
                    bOK = false;
                }
            } else {
                printf("Couldn't read second line of DEM-File %s\n", pFileName);
                bOK = false;
            }
        } else {
            printf("Couldn't read first line of DEM-File %s\n", pFileName);
            bOK = false;
        }
        printf("Done.\n");
    } else {
        printf("Couldn't open DEM-file %s\n", pFileName);
        bOK = false;
    }
    return bOK;
}

//----------------------------------------------------------------------------
// destructor
//
DynXYZDEM::~DynXYZDEM() {
}

