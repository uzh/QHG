#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "strutils.h"
#include "BufReader.h"
#include "BufWriter.h"

#include "icoutil.h"
#include "IcoHeader.h"

static const char *sStartTags[2][3] = {
    {    KEY_START_ICO, KEY_START_OCT, KEY_START_TET, },
    {    KEY_START_ICO3, KEY_START_OCT3, KEY_START_TET3, },

};


IcoHeader::IcoHeader(int iPolyType, gridtype lID, int iSubLevel, tbox &tBox) 
    : m_iPolyType(iSubLevel),
      m_lID(lID),
      m_iSubLevel(iSubLevel),
      m_tBox(tBox) {
}

IcoHeader::IcoHeader(int iPolyType, int iSubLevel, tbox &tBox) 
    : m_iPolyType(iPolyType),
      m_lID(createID()),
      m_iSubLevel(iSubLevel),
      m_tBox(tBox) {
}

IcoHeader::IcoHeader()
    : m_iPolyType(POLY_TYPE_NONE),
      m_lID(-1),
      m_iSubLevel(-1) {
}

void IcoHeader::getBox(tbox &tBox) {
    tBox = m_tBox;
}

gridtype IcoHeader::createID() {
    return (gridtype)time(NULL);
}

int IcoHeader::read(BufReader *pBR) {
    printf("read\n");
    int iResult = 0;
    int iHSize = 256;
    char sLine[iHSize];
    // expect "ICO", "OCT", "TET"
    uint iSize = iHSize;
    iResult = pBR->getBlock(sLine, &iSize, "\n");
    if ((iResult == 0) && (iSize > 0)) {
        // delete the CR
        sLine[iSize-1] = '\0';
        if (sLine[iSize-2] == '*') {
            sLine[iSize-2]  ='\0';
        }
        char *pEnd;
        for (int j = 0; (m_iPolyType == POLY_TYPE_NONE) && (j < 2); j++) {
            for (int i = 0; (m_iPolyType == POLY_TYPE_NONE) && (i < 3); i++) {
                if (strcmp(sLine, sStartTags[j][i]) == 0) {
                    m_iPolyType = i;
                } 
            }
        }
        printf("poly type:%d\n", m_iPolyType);
        if (m_iPolyType != POLY_TYPE_NONE) {
            bool bInHeader = true;

            while (bInHeader && (iResult == 0)) {

                iSize = iHSize;
                iResult = pBR->getBlock(sLine, &iSize, "\n");

                if ((iResult == 0) && (iSize > 0)) {
                    sLine[iSize-1] = '\0';
                    if (strncasecmp(sLine, KEY_ID, strlen(KEY_ID)) == 0) {
                        m_lID = (gridtype)strtol(trim(sLine+strlen(KEY_ID)), &pEnd, 10);
                        if (*pEnd != '\0') {
                            // ID is not a number
                            iResult = -1;
                        }
                    } else  if (strncasecmp(sLine, KEY_LEVEL, strlen(KEY_LEVEL)) == 0) {
                        m_iSubLevel = (int)strtol(trim(sLine+strlen(KEY_LEVEL)), &pEnd, 10);
                        if (*pEnd != '\0') {
                            // level is not a number
                            iResult = -1;
                        }
                    } else  if (strncasecmp(sLine, KEY_BOX, strlen(KEY_BOX)) == 0) {
                        int iCount = sscanf(sLine+strlen(KEY_BOX), "%lf %lf %lf %lf", &(m_tBox.dLonMin), &(m_tBox.dLonMax), &(m_tBox.dLatMin), &(m_tBox.dLatMax));
                        printf("Have box %lf %lf %lf %lf\n", m_tBox.dLonMin, m_tBox.dLonMax, m_tBox.dLatMin, m_tBox.dLatMax);
                        if (iCount != 4) {
                            // box not properly read
                            iResult = -1;
                        }
                    } else  if (strncasecmp(sLine, KEY_END, strlen(KEY_END)) == 0) {
                        bInHeader = false;
                    }
                }
            }
        } else {
            // not an ico header
            iResult = -1;
        }
    }


    return iResult;
}

int IcoHeader::write(BufWriter *pBW) {
    int iResult = 0;

    char sLine[256];
    sprintf(sLine, "%s\n%s %d\n%s %d\n%s %24.21f %24.21f %24.21f %24.21f\n%s\n", 
            sStartTags[1][m_iPolyType],
            KEY_ID, m_lID, 
            KEY_LEVEL, m_iSubLevel, 
            KEY_BOX, m_tBox.dLonMin, m_tBox.dLonMax, m_tBox.dLatMin, m_tBox.dLatMax,
            KEY_END); 
    
    iResult = pBW->addLine(sLine);
    return iResult;
}
