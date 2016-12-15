#include "TabDataReader.h"
#include "FAOReader.h"

const double BAD_VAL = dNaN;

//------------------------------------------------------------------------------
// constructor
//
FAOReader::FAOReader(char *pFileName, 
            float fRangeLonMin, float fRangeLonMax,
            float fRangeLatMin, float fRangeLatMax)
    : TabDataReader(pFileName, 
                    -179.75f, 0.5f, fRangeLonMin, fRangeLonMax,
                    -55.75f, 0.5f, fRangeLatMin, fRangeLatMax,1) {
}

//------------------------------------------------------------------------------
// destructor
//
FAOReader::~FAOReader() {
}

//------------------------------------------------------------------------------
// extractLocation
//   In these files, longitude and latitude are in the 2nd and 3rd field,
//   and are given as multiples of .01 degrees 
//   Fields:
//     - gridpoint number, 
//     - longitude in .01 degrees, 
//     - latitude in .01 degrees, 
//     - land fraction of the grid cell in %, 
//     - NPP, 
//     - NPP if precipitation were limiting (NPPP), 
//     - NPP if temperature were limiting (NPPT).
//
bool FAOReader::extractLocation(char *pLine, float *pfLon, float *pfLat) {
    bool bOK = false;
    char sLine[MAX_LINE];
    strcpy(sLine, pLine);
    char *pp;
    char *p = strtok_r(pLine, ", \t\n", &pp);
    if (p != NULL) {
        // "gridpoint number"
        char *p = strtok_r(NULL, ", \t\n", &pp);
        if (p != NULL) {
            bOK = false;
            char *pEnd;
            // longitude
            *pfLon = strtof(p, &pEnd);
            if (*pEnd=='\0') {
                *pfLon /= 100;
                p = strtok_r(NULL, ", \t\n", &pp);
                if (p != NULL) {
                    // latitude
                    *pfLat = strtof(p, &pEnd);
                    if (*pEnd=='\0') {
                        *pfLat /= 100;
                        bOK = true;
                    }
                }
            }
        }
    }
    return bOK;
}


