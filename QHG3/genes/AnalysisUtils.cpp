#include <string.h>

#include "LineReader.h"
#include "colors.h"
#include "AnalysisUtils.h"


//----------------------------------------------------------------------------
// fillLocData
//
int fillLocData(const locspec *pLocSpec, loc_data &mLocData, stringvec *pvNames) {
    int iResult = -1;
    
    LineReader *pLR = LineReader_std::createInstance(pLocSpec->pLocFile, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        iResult = 0;
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            char *pName;

            int iReq = 5;
            int iC = 0;
            locitem li;
            char *pNEnd = strchr(pLine, ']');
            if (pNEnd != NULL) {
                pNEnd++;
                *pNEnd = '\0';
                pName = pLine;
                pLine = pNEnd+1;
                iReq = 4;
                iC = sscanf(pLine, "%lf %lf %lf %d", &li.dLon, &li.dLat, &li.dDist, &li.iNum);
            } else {
                char sName[512];
                iC = sscanf(pLine, "%s %lf %lf %lf %d", sName, &li.dLon, &li.dLat, &li.dDist, &li.iNum);
                pName = sName;
            }
            if (pLocSpec->dDistance > 0) {
                li.dDist = pLocSpec->dDistance;
            }
            if (pLocSpec->iNum > 0) {
                li.iNum = pLocSpec->iNum;
            }
            li.iPadding = 11;
            if (iC == iReq) {
                // valid data: save it
                if (pvNames != NULL) {
                    pvNames->push_back(pName);
                }
                mLocData[pName]  =  li;
            } else {
                fprintf(stderr, "%sCouldn't read enough items from line [%s]%s\n", RED, pLine, OFF);
                iResult = -1;
            }
            pLine = pLR->getNextLine();
        }

        delete pLR;
    } else {
        fprintf(stderr, "%sCouldn't open file [%s]%s\n", RED, pLocSpec->pLocFile, OFF);
    }

    return iResult;
}

