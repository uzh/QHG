#include <stdio.h>
#include <string.h>

#include "strutils.h"
#include "fileutils.h"
#include "LineReader.h"
#include "types.h"
#include "QMapHeader.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "PNGImage.h"
#include "ParamReader.h"
#include "translations.h"



double translate(int iTransMode, uchar *auD, int iL) {
    double dOut = dNaN;

    // summed intensity
    if (iTransMode == 0) {
        int iS = 0;
        for (int i = 0; i < iL; i++) {
            iS += auD[i];
        }
        dOut = (1.0*iS)/(iL*255);
        //        dOut =  (iS > iL*128)?1.0:0.0;
    }
    return dOut;
}


int init(char *pPNGIn, ColorTranslation *pCT, char *pOut) {
    int iResult = -1;
    PNGImage *pPNG0 = new PNGImage();
    bool bOK = pPNG0->readPNGFile(pPNGIn);
    if (bOK) {
        int iW = pPNG0->getWidth();
        int iH = pPNG0->getHeight();
        int iCT = pPNG0->getColorType();
        int iNC = 0;
        switch (iCT) {
        case PNG_COLOR_TYPE_GRAY:
            iNC = 1;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            iNC = 2;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            iNC = 1;
            break;
        case PNG_COLOR_TYPE_RGB:
            iNC = 3;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            iNC = 4;
            break;
        }

        printf("image %dx%d, bitdepth:%d, colortype: %d, bytewidth %d, numchan: %d\n", 
               iW, iH, pPNG0->getBitDepth(), iCT, pPNG0->getByteWidth(), iNC);
        printf("coltype GRAY:%d, GRAY_ALPHA %d, PALETTE %d, RGB %d, RGBA %d\n", 
               PNG_COLOR_TYPE_GRAY,PNG_COLOR_TYPE_GRAY_ALPHA,PNG_COLOR_TYPE_PALETTE,PNG_COLOR_TYPE_RGB,PNG_COLOR_TYPE_RGB_ALPHA);
        double **ppdData = QMapUtils::createArray(iW, iH, dNaN);
        iNC *= pPNG0->getBitDepth()/8;
        for (int i = 0; i < iH; ++i) {
            uchar *pRow = pPNG0->getRow(i);
            for (int j = 0; j < iW; ++j) {

                
                ppdData[i][j] = pCT->convert(pRow+iNC*j, iNC, pPNG0->getBitDepth());
            }
        }

        FILE *fOut = fopen(pOut, "wb");
        if (fOut != NULL) {
            iResult = 0;
            QMapHeader *pQMH =  new QMapHeader(QMAP_TYPE_DOUBLE,
                                               0, iW, 1.0,
                                               0, iH, 1.0);
            pQMH->addHeader(fOut);
            
            bOK = QMapUtils::writeArray(fOut, iW, iH, ppdData);
            
            fclose(fOut);
            delete pQMH;
        } else {
            iResult = -3;
        }
        QMapUtils::deleteArray(iW, iH, ppdData);
        delete pPNG0;
    }
    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult=-1;
    if (iArgC >2) {
        ColorTranslation *pCT = NULL;
        int iOut = 2;
        if (iArgC > 3) {
            iOut = 3;
            char *p = strchr(apArgV[2], ':');
            if (p != NULL) {
                *p = '\0';
                p++;
            } else {
                p = apArgV[3]+strlen(apArgV[3]);
            }
            if (strcasecmp(apArgV[2], "bw") == 0) {
                pCT = new BWTranslation(p);
            } else if (strcasecmp(apArgV[2], "gray") == 0) {
                pCT = new GrayTranslation(p);
            } else if (strcasecmp(apArgV[2], "tab") == 0) {
                pCT = new TableTranslation(p);
            }
        }
        if (pCT == NULL) {
            pCT = new BWTranslation("0.5");
        }
        iResult = init(apArgV[1], pCT, apArgV[iOut]);
        delete pCT;
    }
    if (iResult < 0) {
        switch(iResult) {
        case -1:
            printf("Usage :\n  %s <input-png> <output-qmap>\n", apArgV[0]);
            break;
        case -2:
            printf("Couldn't open input PNG [%s]\n", apArgV[1]);
            break;
        case -3:
            printf("Couldn't open output QMap [%s]\n", apArgV[2]);
            break;
        default: 
            printf("Unknown error [%d]\n", iResult);
        }
    }
    return iResult;
}
