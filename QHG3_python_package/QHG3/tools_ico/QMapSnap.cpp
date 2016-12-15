#include <stdio.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "strutils.h"
#include "Vec3D.h"
#include "SnapHeader.h"
#include "ValReader.h"
#include "QMapUtils.h"
#include "icoutil.h"
#include "Lattice.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "VertexLinkage.h"

#define BUF_SIZE 10000

const int STYPE_NONE = -1;
const int STYPE_ICO  =  0;
const int STYPE_EQ   =  1;
const int STYPE_FLAT =  2;

const int DAT_NUL    = -1;
const int DAT_LON    =  0;
const int DAT_LAT    =  1;

Lattice *s_pLattice = NULL;
Icosahedron *s_pIco = NULL;
EQsahedron *s_pEQ   = NULL;


//-----------------------------------------------------------------------------
// getIcoVL
//
VertexLinkage *getIcoVL(const char *pFile) {
    VertexLinkage *pVL = NULL;

    int iResult = 0;
    int iType = STYPE_NONE;
    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        char s[4];
        int iRead = fread(s, 1, 4, fIn);
        if (iRead == 4) {
            if ((s[3] == '3') || s[3] == '\n') {
                s[3] = '\0';
            }
            if (strcmp(s, "ICO") == 0) {
                iType = STYPE_ICO;
            } else if (strcmp(s, "LTC") == 0) {
                iType = STYPE_FLAT;
            } else if (strcmp(s, "IEQ") == 0) {
                iType = STYPE_EQ;
            }
        } else {
            *s = '\0';
            printf("Couldn't read from [%s]\n", pFile);
            iResult = -1;
        }
        
        fclose(fIn);

        if (iResult == 0) {
            switch (iType) {
            case STYPE_ICO:
                s_pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                s_pIco->merge(0);
                s_pIco->setStrict(true);
                s_pIco->setPreSel(false);
                iResult = s_pIco->load(pFile);
                if (iResult == 0) {
                    printf("oinkk\n");
                    s_pIco->relink();
                    pVL = s_pIco->getLinkage();
                }
                break;
            case STYPE_FLAT:
                s_pLattice = new Lattice();
                iResult = s_pLattice->load(pFile);
                if (iResult == 0) {
                    pVL = s_pLattice->getLinkage();
                }
                break;
            case STYPE_EQ:
                s_pEQ = EQsahedron::createEmpty();
                iResult = s_pEQ->load(pFile);
                if (iResult == 0) {
                    s_pEQ->relink();
                    pVL = s_pEQ->getLinkage();
                }
                break;
            }
        }

    }
    return pVL;
}


//-----------------------------------------------------------------------------
// cleanup
//
void cleanup() {
    if (s_pIco != NULL) {
        delete s_pIco;
    }
    if (s_pEQ != NULL) {
        delete s_pEQ;
    }
    if (s_pLattice != NULL) {
        delete s_pLattice;
    }
}

//-----------------------------------------------------------------------------
// fillQMap 
//
int fillQMap(FILE *fOut, VertexLinkage *pVL, ValReader *pVR) {
    int iResult = 0;
    unsigned char *pBuf = new unsigned char[BUF_SIZE*(sizeof(gridtype)+sizeof(double))];
    unsigned char *p = pBuf;
    int iCount = 0;
    iResult = 0;
    printf("Doing %zd nodes\n", pVL->m_mI2V.size());
    std::map<gridtype, Vec3D *>::const_iterator it;
    for (it = pVL->m_mI2V.begin(); (iResult == 0) && (it != pVL->m_mI2V.end()); ++it) {
        gridtype id = it->first;
        double dLon = dNaN;
        double dLat = dNaN;
        cart2Sphere(it->second, &dLon, &dLat);
        double dV = pVR->getDValue(dLon*180/M_PI, dLat*180/M_PI);
        p = putMem(p, &id, sizeof(gridtype));
        p = putMem(p, &dV, sizeof(double));
        iCount++;
        if (iCount >= BUF_SIZE) {
            printf("writing %d entries\n", BUF_SIZE);
            int iWritten = fwrite(pBuf, sizeof(gridtype)+sizeof(double), BUF_SIZE, fOut);
            if (iWritten == BUF_SIZE) {
                iCount =0;
                p = pBuf;
            } else {
                printf("Error while writing to output file\n");
                iResult = -1;
            }
        }
    }
    if (iCount >= 0) {
        printf("writing last %d entries\n", iCount);
        int iWritten = fwrite(pBuf, sizeof(gridtype)+sizeof(double), iCount, fOut);
        if (iWritten == iCount) {
            iCount =0;
        } else {
            printf("Error while writing last data to output file\n");
            iResult = -1;
        }
    }
    delete[] pBuf;

    return iResult;
}


//-----------------------------------------------------------------------------
// fillLat 
//
int fillLat(FILE *fOut, VertexLinkage *pVL, int iInternal) {
    int iResult = 0;
    unsigned char *pBuf = new unsigned char[BUF_SIZE*(sizeof(gridtype)+sizeof(double))];
    unsigned char *p = pBuf;
    int iCount = 0;
    iResult = 0;
    printf("Doing %zd nodes\n", pVL->m_mI2V.size());
    std::map<gridtype, Vec3D *>::const_iterator it;
    for (it = pVL->m_mI2V.begin(); (iResult == 0) && (it != pVL->m_mI2V.end()); ++it) {
        gridtype id = it->first;
        double dGeo[2];
        dGeo[0] = dNaN;
        dGeo[1] = dNaN;

        cart2Sphere(it->second, &(dGeo[0]), &(dGeo[1]));
        p = putMem(p, &id, sizeof(gridtype));
        p = putMem(p, &(dGeo[iInternal]), sizeof(double));
        iCount++;
        if (iCount >= BUF_SIZE) {
            printf("writing %d entries\n", BUF_SIZE);
            int iWritten = fwrite(pBuf, sizeof(gridtype)+sizeof(double), BUF_SIZE, fOut);
            if (iWritten == BUF_SIZE) {
                iCount =0;
                p = pBuf;
            } else {
                printf("Error while writing to output file\n");
                iResult = -1;
            }
        }
    }
    if (iCount >= 0) {
        printf("writing last %d entries\n", iCount);
        int iWritten = fwrite(pBuf, sizeof(gridtype)+sizeof(double), iCount, fOut);
        if (iWritten == iCount) {
            iCount =0;
        } else {
            printf("Error while writing last data to output file\n");
            iResult = -1;
        }
    }
    delete[] pBuf;

    return iResult;
}


//-----------------------------------------------------------------------------
// main 
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 3) {
        // get ico
        
        VertexLinkage *pVL = getIcoVL(apArgV[1]);
        /*
        EQsahedron *pEQ = EQsahedron::createEmpty();
        iResult = pEQ->load(apArgV[1]);
        if (iResult == 0) {
            pEQ->relink();
            VertexLinkage *pVL = pEQ->getLinkage();
            */
        if (pVL != NULL) {
            
            
            int iInternal = DAT_NUL;
            ValReader *pVR = QMapUtils::createValReader(apArgV[2], true, NULL);
            if (pVR == NULL) {
                if (strcmp(apArgV[2], "LAT") == 0) {
                    iInternal = DAT_LAT;
                } else if (strcmp(apArgV[2], "LON") == 0) {
                    iInternal = DAT_LON;
                }
            }
            if ((pVR != NULL) || (iInternal != DAT_NUL)) {
                FILE *fOut = fopen(apArgV[3], "wb");
                if (fOut != NULL) {
                    SnapHeader *pSH = new SnapHeader("3.0", COORD_NODE, 0, 0, "ld", apArgV[1], false, 0,"created", 0, NULL);
                    if (pSH != NULL) {
                        pSH->write(fOut,true);
                        if (pVR != NULL) {
                            iResult = fillQMap(fOut, pVL, pVR);
                        } else {
                            
                            switch (iInternal) {
                            case DAT_LAT:
                            case DAT_LON:
                                iResult = fillLat(fOut, pVL, iInternal);
                                break;
                            default:
                                break;
                            } 
                        }
                        delete pSH;
                    } else {
                        printf("Couldn't create snap header\n");
                        iResult = -1;
                    }
                    
                    fclose(fOut);
                    
                } else {
                        printf("Couldn't open output file for writing [%s]\n", apArgV[3]);
                        iResult = -1;
                    }
                    delete pVR;
            } else {
                printf("Couldn't load qmap file [%s], or invalid internal specifier\n", apArgV[2]);
                iResult = -1;
            }

            cleanup();
        } else {
            printf("Couldn't load ico file [%s]\n", apArgV[1]);
            iResult = -1;
        }
    } else {
        printf("Usage: %s <icofile> <qmapfile> <outputfile>\n", apArgV[0]);
    }
    return iResult;
}



