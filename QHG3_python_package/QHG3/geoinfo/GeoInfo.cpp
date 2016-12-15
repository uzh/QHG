#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"
#include "strutils.h"
#include "MessLogger.h"
#include "GeoInfo.h"

#include "Projector.h"
#include "EQRProjector.h"
#include "OrthoProjector.h"
#include "AEProjector.h"
#include "TCEAProjector.h"
#include "LAEAProjector.h"
#include "LCCProjector.h"
#include "CEAProjector.h"
#include "LINProjector.h"

#include "GeoProvider.h"
#include "PrGeoProvider.h"
#include "PlaneGeoProvider.h"

/*
static const char *asProjNames[] = {
    "Equirectangular",
    "Orthographic Projector",
    "Azimuthal Equidistant Projector",
    "Transverse Cylindrical Equal Area Projector",
    "Lambert Azimuthal Equal Area Projector",
    "Lambert Conformal Conical Projector",
};

*/

GeoInfo* GeoInfo::s_pGeoInfo = NULL;

const double EPS = 1E-6;
double static relativeError(double d1, double d2) {
    double dDiff = fabs(d1-d2);
    if (d1 != 0) {
        dDiff /= d1;
    } else if (d2 != 0) {
        dDiff /= d2;
    }
    return dDiff;
}

//----------------------------------------------------------------------------
// ProjType::ToString
//
char *ProjType::toString(bool bDegrees) const {
    static char s_sPrType[256];
    sprintf(s_sPrType, "%d [%s] %f %f %d",
            m_iProjType,
            GeoInfo::getName(m_iProjType),
            m_dLambda0*(bDegrees?180/M_PI:1),
            m_dPhi0*(bDegrees?180/M_PI:1),
            m_iNumAdd);
    char sNum[16];
    for (int i =0; i < m_iNumAdd; ++i) {
        sprintf(sNum, " %f", m_adAdd[i]);
        strcpy(s_sPrType, sNum);
    }
    return s_sPrType;
}

//----------------------------------------------------------------------------
// ProjType::fromString
//
int ProjType::fromString(const char *pLine, bool bDegrees) {
    int iResult = 0;
    char sCopy[256];
    strcpy(sCopy, pLine);
    char *pCtx=NULL;
    // remove the brackets and their contents (human readable type)
    char *p0 = strchr(sCopy, '[');
    if (p0 != NULL) {
        char *p1 = strchr(sCopy, ']');
        if (p1 != NULL) {
            p1++;
            memmove(p0, p1, strlen(sCopy)-(p1-sCopy)+1);
        } else {
            // no closing bracket
            iResult = -1;
        }
    }
    
    if (iResult == 0) {
        iResult = -1;
        char sRest[256];
        int iRead = sscanf(sCopy, "%d %lf %lf %d %256c", &m_iProjType, &m_dLambda0, &m_dPhi0, &m_iNumAdd, sRest);
        if (iRead >= 4) {
            if (bDegrees) {
                m_dLambda0 *= M_PI/180;
                m_dPhi0    *= M_PI/180;
            }
            iResult = 0;
            if (m_iNumAdd <= MAX_ADD) {
                if (m_iNumAdd > 0) {
                    char *pEnd;
                    int iC = 0;
                    char *p = strtok_r(sRest, " \t", &pCtx);
                    while ((iResult == 0) && (iC < m_iNumAdd) && (p != NULL)) {
                        m_adAdd[iC++] = strtod(p, &pEnd);
                        if (*pEnd != '\0') {
                            iResult = -1;
                        } else {
                            p = strtok_r(NULL, " \t", &pCtx);
                        }
                    }
                    if (iResult == 0) {
                        if (iC < m_iNumAdd) {
                            iResult = -1;
                            // not enough params foun
                        }
                    }
                }
            } else {
                // too many params
                iResult = -1;
            }
        } else {
            // couldn't read params
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// ProjType constructor
//
ProjType::ProjType()
    : m_iProjType(-1),
      m_dLambda0(dNaN),
      m_dPhi0(dNaN),
      m_iNumAdd(0) {
    for (int i =0; i < MAX_ADD; ++i) {
        m_adAdd[i] = dNaN;
    }
}

//----------------------------------------------------------------------------
// ProjType constructor
//
ProjType::ProjType(int iProjType, 
                   double dLambda0, 
                   double dPhi0, 
                   int iNumAdd, 
                   double *pdAdd)
    : m_iProjType(iProjType),
      m_dLambda0(dLambda0),
      m_dPhi0(dPhi0),
      m_iNumAdd(iNumAdd) {
    for (int i = 0; i < MAX_ADD; i++) {
        if (i < iNumAdd) {
            m_adAdd[i] = pdAdd[i];
        } else {
            m_adAdd[i] = dNaN;
        }
    }
}

//----------------------------------------------------------------------------
// ProjType copy constructor
//
void ProjType::copy(const ProjType *pt) {
    m_iProjType = pt->m_iProjType;
    m_dLambda0  = pt->m_dLambda0;
    m_dPhi0     = pt->m_dPhi0;
    m_iNumAdd   = pt->m_iNumAdd;
    memcpy(m_adAdd, pt->m_adAdd, MAX_ADD*sizeof(double));
}

//----------------------------------------------------------------------------
// ProjType comparator isEqual
//
bool ProjType::isEqual(ProjType *pPT) {
    
    bool bEqual = (m_iProjType == pPT->m_iProjType) &&
        (relativeError(m_dLambda0, pPT->m_dLambda0) < EPS) &&
        (relativeError(m_dPhi0, pPT->m_dPhi0) < EPS)  &&
        (m_iNumAdd == pPT->m_iNumAdd);
    if (bEqual) {
        for (int i = 0; bEqual && (i < m_iNumAdd); ++i) {
            bEqual = bEqual && (relativeError(m_adAdd[i], pPT->m_adAdd[i]) < EPS);
        }
    }
    return bEqual;
}

//---------------------------------------------------
// serialize
//
unsigned char *ProjType::serialize(unsigned char *p) const {
    p = putMem(p, &m_iProjType, sizeof(int));
    p = putMem(p, &m_dLambda0, sizeof(double));
    p = putMem(p, &m_dPhi0, sizeof(double));
    p = putMem(p, &m_iNumAdd, sizeof(int));
    p = putMem(p, m_adAdd, MAX_ADD*sizeof(double));

    return p;
}

//---------------------------------------------------
// deserialize
//
unsigned char *ProjType::deserialize(unsigned char *p) {
    p = getMem(&m_iProjType, p, sizeof(int));
    p = getMem(&m_dLambda0,  p, sizeof(double));
    p = getMem(&m_dPhi0,     p, sizeof(double));
    p = getMem(&m_iNumAdd,   p, sizeof(int));
    p = getMem(m_adAdd,      p, MAX_ADD*sizeof(double));

    return p;
}

//---------------------------------------------------
// createPT
//   assumes input of form
//   <Type>:<long0>:<lat0>:<NumPar>(:<Par>)*
//   ptDef is a valid projection type to be used if string is invalid
//   bDegrees - if true inputs are degrees else radians
//
ProjType *ProjType::createPT(const char *pPDData, bool bDegrees) {
    char *pEnd;
    bool bOK=true;
    ProjType *pPT   = NULL;
    char *pPDData2  = strdup(pPDData);
    int iPType      = -1;
    double dLambda0 = dNaN;
    double dPhi0    = dNaN;
    int iNumAdd     = 0;
    double *dVals   = NULL;

    char *pCtx;
    char *p0 = strtok_r(pPDData2, ":", &pCtx);
    if (p0 != NULL) {
        iPType = strtol(p0, &pEnd, 10);
        if (*pEnd != '\0') {
            LOG_ERROR("[createPT] invalid number for projection type [%s]\n", p0);
            bOK = false;
        }
    } else {
        LOG_ERROR("[createPT] expected projection type\n");
        bOK = false;
    }

    if (bOK) {
        p0 = strtok_r(NULL, ":", &pCtx);
        
        if (p0 != NULL) {  
            dLambda0 = strtod(p0, &pEnd);
            if (*pEnd == '\0') {
                
                if (bDegrees) {
                    dLambda0 *= M_PI/180;
                }
            } else {
                LOG_ERROR("[createPT] invalid number for projection center longitude [%s]\n", p0);
                bOK = false;
            }
        } else {
            LOG_ERROR("[createPT] expected projection center longitude\n");
            bOK = false;
        }
    }
    printf("Lambda: %s -> %f\n", p0, dLambda0);
    if (bOK) {
        p0 = strtok_r(NULL, ":", &pCtx);

        if (p0 != NULL) {  
            dPhi0 = strtod(p0, &pEnd);
            if (*pEnd == '\0') {
                
                if (bDegrees) {
                    dPhi0 *= M_PI/180;
                }
            } else {
                LOG_ERROR("[createPT] invalid number for projection center latitude [%s]\n", p0);
                bOK = false;
            }
        } else {
            LOG_ERROR("[createPT] expected projection center latitude\n");
            bOK = false;
        }
    }
    printf("Phi: %s -> %f\n", p0, dPhi0);

    if (bOK) {
        p0 = strtok_r(NULL, ":", &pCtx);

        if (p0 != NULL) {  
            iNumAdd = strtol(p0, &pEnd, 10);
            if (*pEnd != '\0') {
                LOG_ERROR("[createPT] invalid number for numer of additional parametersprojection center latitude [%s]\n", p0);
                bOK = false;
            }
        } else {
            LOG_ERROR("[createPT] expected number of additional parameters\n");
            bOK = false;
        }
    }

   
    if (bOK) {
        if (iNumAdd > 0) {
            dVals = new double[iNumAdd];
            int iC = 0;
            p0 = strtok_r(NULL, ":", &pCtx);
            if (p0 != NULL) {
                while (bOK) {
                
                    dVals[iC] = strtod(p0, &pEnd);
                    if (*pEnd != '\0') {
                        LOG_ERROR("[createPT] invalid number for parameter #%d [%s]\n", iC, p0);
                        bOK = false;
                    } else {
                        ++iC;
                        p0 = strtok_r(NULL, ":", &pCtx);
                    }
                }
                if (iC != iNumAdd) {
                    LOG_ERROR("[createPT] number of parameters (%d) doesn't match iNumAdd (%d)\n", iC, iNumAdd);
                    bOK = false;
                }
            } else {
                LOG_ERROR("[createPT] invalid number for parameter #%d [%s]\n", iC, p0);
                bOK = false;
            }
        }
    }

    if (bOK) {
        pPT = new ProjType(iPType, dLambda0, dPhi0, iNumAdd, dVals);
    }
    free(pPDData2);
    if (dVals != NULL) {
        delete[] dVals;
    }
    return pPT;
}



//----------------------------------------------------------------------------
// ProjGrid::toString
//
char *ProjGrid::toString() const {
    static char s_sPrData[256];
    sprintf(s_sPrData, "%d %d %f %f %f %f %f", 
            m_iGridW,
            m_iGridH,
            m_dRealW,
            m_dRealH,
            m_dOffsX,
            m_dOffsY,
            m_dRadius);
    return s_sPrData;
}
//----------------------------------------------------------------------------
// ProjGrid::fromString
//
int ProjGrid::fromString(const char *pLine) {
    int iResult = 0;
    int iRead = sscanf(pLine, "%d %d %lf %lf %lf %lf %lf",
            &m_iGridW,
            &m_iGridH,
            &m_dRealW,
            &m_dRealH,
            &m_dOffsX,
            &m_dOffsY,
            &m_dRadius);
    if (iRead != 7) {
        iResult = -1;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// ProjGrid constructor
//
ProjGrid::ProjGrid() 
    :    m_iGridW(0),
         m_iGridH(0),
         m_dRealW(0),
         m_dRealH(0),
         m_dOffsX(0),
         m_dOffsY(0),
         m_dRadius(0) {
}

//----------------------------------------------------------------------------
// ProjGrid constructor
//
ProjGrid::ProjGrid(int iGridW, 
                   int iGridH, 
                   double dRealW, 
                   double dRealH, 
                   double dOffsX,
                   double dOffsY,
                   double dRadius) 
    :    m_iGridW(iGridW),
         m_iGridH(iGridH),
         m_dRealW(dRealW),
         m_dRealH(dRealH),
         m_dOffsX(dOffsX),
         m_dOffsY(dOffsY),
         m_dRadius(dRadius) {
}

//----------------------------------------------------------------------------
// ProjGrid copy constructor
//
void ProjGrid::copy(const ProjGrid *ppg) {
    m_iGridW  = ppg->m_iGridW;
    m_iGridH  = ppg->m_iGridH;
    m_dRealW  = ppg->m_dRealW;
    m_dRealH  = ppg->m_dRealH;
    m_dOffsX  = ppg->m_dOffsX;
    m_dOffsY  = ppg->m_dOffsY;
    m_dRadius = ppg->m_dRadius;

}

//----------------------------------------------------------------------------
// ProjGrid comparator isEqual
//
bool ProjGrid::isEqual(ProjGrid *pPG) {
    
    bool bEqual = (m_iGridW == pPG->m_iGridW) &&
        (m_iGridH  == pPG->m_iGridH) &&
        (relativeError(m_dRealW, pPG->m_dRealW) < EPS) &&
        (relativeError(m_dRealH, pPG->m_dRealH) < EPS)  &&
        (relativeError(m_dOffsX, pPG->m_dOffsX) < EPS)  &&
        (relativeError(m_dOffsY, pPG->m_dOffsY) < EPS)  &&
        (relativeError(m_dRadius, pPG->m_dRadius) < EPS);
    return bEqual;
}
//---------------------------------------------------
// serialize
//
unsigned char *ProjGrid::serialize(unsigned char *p) const {
    p = putMem(p, &m_iGridW,  sizeof(double));
    p = putMem(p, &m_iGridH,  sizeof(double));
    p = putMem(p, &m_dRealW,  sizeof(double));
    p = putMem(p, &m_dRealH,  sizeof(double));
    p = putMem(p, &m_dOffsX,  sizeof(double));
    p = putMem(p, &m_dOffsY,  sizeof(double));
    p = putMem(p, &m_dRadius, sizeof(double));
    return p;
}

//---------------------------------------------------
// deserialize
//
unsigned char *ProjGrid::deserialize(unsigned char *p) {
    p = getMem(&m_iGridW,  p, sizeof(double));
    p = getMem(&m_iGridH,  p, sizeof(double));
    p = getMem(&m_dRealW,  p, sizeof(double));
    p = getMem(&m_dRealH,  p, sizeof(double));
    p = getMem(&m_dOffsX,  p, sizeof(double));
    p = getMem(&m_dOffsY,  p, sizeof(double));
    p = getMem(&m_dRadius, p, sizeof(double));

    return p;
}

//---------------------------------------------------
// createPD
//   assumes input of form
//   <GridW>:<GridH>:<RealW>:<RealH>:<OffX>:<OffY>:<R>
//   pdDef is a valid ProjGrid, to be used if string is invalid
//   (pd does not have any input in degrees/radians)
//
ProjGrid * ProjGrid::createPG(const char *pPGData) {
    char *pPGData2 = strdup(pPGData);
    ProjGrid *pPG = NULL;
    double dVals[7];
    dVals[0] = dNaN;
    dVals[1] = dNaN;
    dVals[2] = dNaN;
    dVals[3] = dNaN;
    dVals[4] = dNaN;
    dVals[5] = dNaN;
    dVals[6] = dNaN;
    
    bool bOK = true;
    char *pEnd;
    int iC = 0;

    bool bCenterX = false;
    bool bCenterY = false;

    char *pCtx;
    char *p0 = strtok_r(pPGData2, ":", &pCtx);
    if (p0 != NULL) {
        while (bOK && (p0 != NULL)) {

            if (*p0 == 'c') {
                if (iC == 4) {
                    bCenterX = true;
                } else if (iC == 5) {
                    bCenterY = true;
                } else {
                    LOG_ERROR("[createPG] 'c' is only allowed for X or Y offset\n");
                    bOK = false;
                }
            } else {
                dVals[iC] = strtod(p0, &pEnd);
                if (*pEnd == '\0') {
                    bOK = true;
                } else {
                    LOG_ERROR("[createPG] invalid number for parameter %d: [%s]\n", iC, p0);
                    bOK = false;
                }
            } 
            ++iC;
            p0 = strtok_r(NULL, ":",&pCtx);
            
        }
        if (bOK && (iC != 7)) {
            LOG_ERROR("[createPG] only %d parameters instead of 7\n", iC);
            bOK = false;
        }
    } else {
        LOG_ERROR("[createPG] expected projection grid width latitude\n");
        bOK = false;
    }
    if (bOK) {
        if (bCenterX) {
            dVals[4] = -dVals[0]/2;
        }
        if (bCenterY) {
            dVals[5] = -dVals[1]/2;
        }
        //    printf("R: %f\n", dVals[6]);
        pPG = new ProjGrid((int)dVals[0], (int)dVals[1],
                           dVals[2], dVals[3], dVals[4], dVals[5], dVals[6]);
    }
    free(pPGData2);
    return pPG;
}


//===========================================================================

//----------------------------------------------------------------------------
// constructor
//
GeoInfo::GeoInfo() {
}

//----------------------------------------------------------------------------
// instance
//
GeoInfo *GeoInfo::instance() {
    if (s_pGeoInfo == NULL) {
        s_pGeoInfo = new GeoInfo();
    }
    return s_pGeoInfo;
}

//----------------------------------------------------------------------------
// free
//
void GeoInfo::free() {
    if (s_pGeoInfo != NULL) {
        delete s_pGeoInfo;
        s_pGeoInfo = NULL;
    }
}

//----------------------------------------------------------------------------
// createProjector
//
Projector *GeoInfo::createProjector(int iType, double dLambda0, double dPhi0) {
    Projector *pr = NULL;
    
    switch (iType) {
    case PR_EQUIRECTANGULAR :
        pr = new EQRProjector(dLambda0, dPhi0);
        break;
    case PR_ORTHOGRAPHIC :
        pr = new OrthographicProjector(dLambda0, dPhi0);
        break;
    case PR_AZIMUTHAL_EQUIDISTANT :
        pr = new AzimuthalEquidistantProjector(dLambda0, dPhi0);
        break;
    case PR_TRANSVERSE_CYLINDRICAL_EQUAL_AREA :
        pr = new TransverseCylindricalEqualAreaProjector(dLambda0, dPhi0);
        break;
    case PR_LAMBERT_AZIMUTHAL_EQUAL_AREA :
        pr = new LambertAzimuthalEqualAreaProjector(dLambda0, dPhi0);
        break;
    case PR_LAMBERT_CONFORMAL_CONIC :
        pr = new LambertConformalConicalProjector(dLambda0, dPhi0);
        break;
    case PR_CYLINDRICAL_EQUAL_AREA :
        pr = new CylindricalEqualAreaProjector(dLambda0, dPhi0);
        break;
    case PR_LINEAR :
        pr = new LINProjector(dLambda0, dPhi0);
        break;
    }

    return pr;
}

//----------------------------------------------------------------------------
// createProjector
//
Projector *GeoInfo::createProjector(const ProjType *pPT) {

    Projector *pr = createProjector(pPT->m_iProjType, pPT->m_dLambda0, pPT->m_dPhi0);
  
    if (pr != NULL) {
        pr->setAdditional(pPT->m_iNumAdd, pPT->m_adAdd);
    }

    return pr;
}

//----------------------------------------------------------------------------
// createProjector
// 
GeoProvider *GeoInfo::createGeoProvider(const ProjType *pPT, const ProjGrid *pPG) {
    GeoProvider *pGP = NULL;
    if (pPT->m_iProjType == PR_LINEAR) {
        pGP = new PlaneGeoProvider(pPG);
    } else {
        Projector *pr = GeoInfo::instance()->createProjector(pPT);
        pGP = new PrGeoProvider(pPG, pr);
    }
    return pGP;
}
