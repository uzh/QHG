#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"


//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pAppName) {
    printf("%s - extract a region from a QMap\n", pAppName);
    printf("Usage:\n");
    printf("  %s <QMapIn> <Range> <QMapOut>\n", pAppName);
    printf("where\n");
    printf("  QMapIn    Name of input QMap\n"); 
    printf("  Range     Range to extract <dLonMin>:<dLonMax>:<dLatMin>:<dLatMax>\n"); 
    printf("            any of those 4 items can be given as '*' to use\n");
    printf("            the original's item\n");
    printf("  QMapOut   Name of output QMap. If omitted, QMapIn is used\n"); 
    printf("\n");
    printf("%s lholdag_f.qmap -15:*:20:60\n", pAppName);
    printf("prune and extend lholdag_f.qmap such that its extents are as follows\n");
    printf("  minimum Longitude -15\n");
    printf("  maximum Longitude  maximum longitude of input file\n");
    printf("  minimum Latitude   20\n");
    printf("  maximum Latitude   60\n");
}


//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char sIn[LONG_INPUT];
    char sRange[LONG_INPUT];
    char sOut[LONG_INPUT];

    if (iArgC > 3) {
        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sIn);
        strcpy(sIn, apArgV[1]);

        strcpy(sRange, apArgV[2]);
        strcpy(sOut, apArgV[3]);
        
        int iType = -1;
        
        int iDataOffset  = 0;
        int iRowSize     = 0;
        int iRangeWidth  = 0;
        int iRangeHeight = 0;
        QMapHeader *pQMH = NULL;

        ValReader *pVR = QMapUtils::createValReader(sIn, sRange, false, false, &iType);
        if (pVR != NULL) {

            pQMH = new QMapHeader(iType, 
                                  pVR->getLonMin(), pVR->getLonMax()+pVR->getDLon()/16, pVR->getDLon(),
                                  pVR->getLatMin(), pVR->getLatMax()+pVR->getDLat()/16, pVR->getDLat(),
                                  pVR->getVName(),  pVR->getXName(),  pVR->getYName());
            iDataOffset  = pVR->getDataOffset();
            iRowSize     = pVR->getNLon();
            iRangeWidth  = pVR->getNRLon();
            iRangeHeight = pVR->getNRLat();
            printf("origWxH: %dx%d\n", iRangeWidth,iRangeHeight);
            printf("WxH: %dx%d\n", iRangeWidth,iRangeHeight);

        } else {
            printf("%s is not a QMapFile\n", sIn);
        }
        
        if (pQMH != NULL) {
            int iItemSize = QMapHeader::getTypeSize(iType);
            iDataOffset = iDataOffset*iItemSize + QMapHeader::getHeaderSize(sIn);
            FILE *fOut = fopen(sOut, "wb");
            if (fOut != NULL) {
                pQMH->addHeader(fOut);
                FILE *fIn = fopen(sIn, "rb");
                if (fIn != NULL) {
                    int iSumDat = 0;

                    // jump to start of subregion data (headersize+dataoffset)
                    fseek(fIn, iDataOffset, SEEK_SET);
                    unsigned char *pRow = new unsigned char[iRangeWidth*iItemSize];
                    bool bOK = true;
                    for (int i = 0; bOK && (i < iRangeHeight); ++i) {

                        int iRead = fread(pRow, iItemSize, iRangeWidth, fIn);
                        if (iRead == iRangeWidth) {
                            int iWritten = fwrite(pRow, iItemSize, iRangeWidth, fOut);
                            if (iWritten != iRangeWidth) {
                                bOK = false;
                                printf("Bad write at %d: %d instead of %d\n", i, iWritten, iRangeWidth);
                            }
                            iSumDat += iWritten;
                        } else {
                            bOK = false;
                            printf("Bad read at %d: %d instead of %d\n", i, iRead, iRangeWidth);
                        }
                        // fastforward to next start
                        fseek(fIn, (iRowSize-iRangeWidth)*iItemSize, SEEK_CUR);
                    }
                    delete[] pRow;
                    
                    fclose(fIn);
                    printf("written %d items (%d bytes)\n", iSumDat, iSumDat*iItemSize);
                } else {
                    printf("Couldn't open [%s] for reading", sIn);
                }
                fclose(fOut);
            } else {
                printf("Couldn't open [%s] for writing\n", sOut);
            }
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}

