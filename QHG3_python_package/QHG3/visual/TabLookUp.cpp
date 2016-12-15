
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "LineReader.h"
#include "LookUp.h"
#include "SegCenters.h"

#include "LookUp.h"
#include "TabLookUp.h"

//-----------------------------------------------------------------------------
// constructor
//  
TabLookUp::TabLookUp(char *pTabFile) 
    : LookUp(0,0) {
    SegCenters *pSC = new SegCenters(0,0); // no need to give dimension and tolerance
    // is the file a segment file?
    bool bOK = pSC->read(pTabFile, false); // read vals/cols only
    if (bOK) {
        // yes: copy the map
        MAP_DOUBLE_INT mm = pSC->getColorMap();
        MAP_DOUBLE_INT::iterator it;
        for (it = mm.begin(); it != mm.end(); ++it) {
            m_mapLookUp[it->first] = it->second;
        }
        
    } else {
        // is it a simple table?
        bOK = readTable(pTabFile);
    }
    if (!bOK) {
        printf("couldn't read file %s\n", pTabFile);
    }
    delete pSC;
}


//-----------------------------------------------------------------------------
// getColor
//
void TabLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    MAP_DOUBLE_INT::iterator it = m_mapLookUp.find(dValue);
    if (it != m_mapLookUp.end()) {
        int iColor = m_mapLookUp[dValue];
        dAlpha = (1.0*(iColor & 0xff))/255;
        iColor >>= 8; 
        dBlue  = (1.0*(iColor & 0xff))/255;
        iColor >>= 8; 
        dGreen = (1.0*(iColor & 0xff))/255;
        iColor >>= 8; 
        dRed   = (1.0*(iColor & 0xff))/255;
    } else {
        dRed   = dNaN;
        dBlue  = dNaN;
        dGreen = dNaN;
        dAlpha = dNaN;
    }
}

//-----------------------------------------------------------------------------
// readTable
//
bool TabLookUp::readTable(char *pSegTabFile) {

    bool bOK = true;
    LineReader *pLR = LineReader_std::createInstance(pSegTabFile, "rt");
    if (pLR != NULL) {
        char *p = pLR->getNextLine(GNL_IGNORE_ALL);
        while (bOK && (p != NULL)) {
            bOK = false;
            char *pCtx;
            char *p0 = strtok_r(p, " ,;\t\n", &pCtx);
            if (p0 != NULL) {
                char *pEnd;
                double dVal = strtod(p0, &pEnd);
                if (*pEnd == '\0') {
                    p0 = strtok_r(NULL, " ,;\t\n", &pCtx); 
                    if (p0 != NULL) {
                        if (*p0 == '#') {
                            ++p0;
                            int iCol;
                            bOK = readHex(trim(p0), &iCol);
                            if (bOK) {
                                m_mapLookUp[dVal] = iCol;
                                bOK = true;
                            }
                        }
                    }
                }
            }
            p = pLR->getNextLine(GNL_IGNORE_ALL);
        }
        delete pLR;
    }
    return bOK;
}
