#include <stdlib.h>
#include <string.h>

#include "strutils.h"
#include "LineReader.h"
#include "translations.h"

#define SWAP(x) (((x & 0xff00)>> 8) || ((x & 0x00ff)<<8))
//-----------------------------------------------------------------------------
// BWTranslation::BWTranslation
//
BWTranslation::BWTranslation(const char *pParam) {
    m_dThresh = atof(pParam);
    printf("Thresh is [%f]\n", m_dThresh);
}

//-----------------------------------------------------------------------------
// BWTranslation::convert
//
double BWTranslation::convert(uchar *pucFirst, int iNum, int iBitDepth) {
    double dS = 0;

    if ((iNum == 4) || (iNum == 6)) {
        iNum--;
    }

    if (iBitDepth == 8) {
        for (int k = 0; k < iNum; k++) {
            dS += pucFirst[k];
        }
    } else if (iBitDepth == 16) {
        unsigned short int *puiFirst = (unsigned short int *) pucFirst;
        for (int k = 0; k < iNum; k++) {
            dS += SWAP(puiFirst[k]);
        }
    }
    
    
    return (dS/iNum > 256*m_dThresh)?1.0:0.0;
}



//-----------------------------------------------------------------------------
// GrayTranslation::convert
//
double GrayTranslation::convert(uchar *pucFirst, int iNum, int iBitDepth) {
    if ((iNum == 4) || (iNum == 6)) {
        iNum--;
    }
    double dS = 0;

    if (iBitDepth == 8) {
        for (int k = 0; k < iNum; k++) {
            dS += pucFirst[k];
        }
    } else if (iBitDepth == 16) {
        unsigned short int *puiFirst = (unsigned short int *) pucFirst;
        for (int k = 0; k < iNum; k++) {
            dS += SWAP(puiFirst[k]);
        }
    }
    
    return dS/(256.0*iNum);
}

//-----------------------------------------------------------------------------
// TableTranslation::TableTranslation
//
TableTranslation::TableTranslation(const char *p) 
    : m_dDefVal(dNaN) {
    loadTranslation(p);
}

//-----------------------------------------------------------------------------
// TableTranslation::convert
//
double TableTranslation::convert(uchar *pucFirst, int iNum, int iBitDepth) {
    double dRes = m_dDefVal;
    
    if ((iNum == 4) || (iNum == 6)) {
        iNum--;
    }

    int iC = 0;
    for (int k = 0; k < iNum; k++) {
        iC <<= 8;
        iC += pucFirst[k];
    }
    
    MAP_TRANS::iterator mit = m_mapT.find(iC);
    if (mit != m_mapT.end()) {
        dRes = m_mapT[iC];
    }
    return dRes;
    

}

//-----------------------------------------------------------------------------
// TableTranslation::loadTranslation
//
int TableTranslation::loadTranslation(const char *pTrans) {
    bool bOK = false;

    int iMachineEndian=0;  
    int iE = 1;

    char *pE = (char *) &iE;
    if (pE[0] == 1) {
        iMachineEndian=0; //little endian
    } else {
        iMachineEndian=1; //big endian
    }
    //    printf("MachineEndian : %d\n", iMachineEndian);
 
    LineReader *pLR = LineReader_std::createInstance(pTrans, "rt");
    if (pLR != NULL) {
        bOK = true;
        char *p = pLR->getNextLine(GNL_IGNORE_ALL);
        if (p != NULL) {
            char *pCtx;
            char *p0 =  strtok_r(p, " ,;\t\n", &pCtx);
            if (strstr(p0, "FORMAT") == p0) {
                p0 = strtok_r(NULL, " ,;\t\n", &pCtx);
                int iFormatEndian = 0;
                if (strcmp(p0, "RGB") == 0) {
                    iFormatEndian = 0;
                } else if (strcmp(p0, "BGR") == 0) {
                    iFormatEndian = 1;
                } else {
                    bOK = false;
                }
                if (bOK) {
                    //                    printf("FormatEndian : %d\n", iFormatEndian);
                    bool bflip = (iMachineEndian!= iFormatEndian);
                    while (bOK && (p != NULL)) {
                        bOK = false;
                         
                        char *p0 = strtok_r(p, " ,;\t\n", &pCtx);
                        if (p0 != NULL) {
                            char *pEnd;
                            int iFrom;
                            bOK = readHex(trim(p0), &iFrom);
                            if (bOK) {

                                p0 = strtok_r(NULL, " ,;\t\n", &pCtx);
                                if (p0 != NULL) {

                                    if (bflip) {
                                        char *pC = (char *)&iFrom;
                                        char cT = pC[0];
                                        pC[0] = pC[2];
                                        pC[2] = cT;
                                    }

                                    double dTo = strtod(p0, &pEnd);
                                    m_mapT[iFrom] = dTo;
                                    bOK = true;
                                    //                                    printf("Loaded : %08x -> %f\n", iFrom, dTo);
                                }
                            }
                        }
                        if (!bOK) {
                            printf("Bad Format [%s]\n", p0);
                        }
                      p = pLR->getNextLine(GNL_IGNORE_ALL);
                    }
                } else {
                    printf("Unknown format (%s) - use 'RGBA' or 'ABGR'\n", p0);
                }
            } else {
                printf("No 'FORMAT' line\n");
            }
        } else {
            printf("empty file?\n");
        }
        delete pLR;
    } else {
        printf("couldn't open translation file [%s]\n", pTrans);
    }
    
    return bOK?0:-1;
}
