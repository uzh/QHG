/** ***************************************************************************\
*   \file   GeoInfo.h
*   \author jody
*   \brief  Header file for classes ProjType, ProjGrid and GeoInfo
*
*   GeoInfo is a factory for Projector obnjects
*** ***************************************************************************/
#ifndef __PROJECTIONS_H__
#define __PROJECTIONS_H__

#include "utils.h"

class Projector;
class GeoProvider;

// indexes for projector names
static const int PR_EQUIRECTANGULAR                   = 0;
static const int PR_ORTHOGRAPHIC                      = 1;
static const int PR_AZIMUTHAL_EQUIDISTANT             = 2;
static const int PR_TRANSVERSE_CYLINDRICAL_EQUAL_AREA = 3;
static const int PR_LAMBERT_AZIMUTHAL_EQUAL_AREA      = 4;
static const int PR_LAMBERT_CONFORMAL_CONIC           = 5;
static const int PR_CYLINDRICAL_EQUAL_AREA            = 6;
static const int PR_LINEAR                            = 7;


// projector names
static const char *asProjNames[] = {
    "Equirectangular Projector",
    "Orthographic Projector",
    "Azimuthal Equidistant Projector",
    "Transverse Cylindrical Equal Area Projector",
    "Lambert Azimuthal Equal Area Projector",
    "Lambert Conformal Conical Projector",
    "Cylindrical Equal Area Projector",
    "Linear",
};

#define MAX_ADD 2

/** ***************************************************************************\
*   \class  ProjType
*   \brief  Container for projector parameters
*** ***************************************************************************/
class ProjType {
public:
    ProjType();
    ProjType(int iProjType, double dLambda0, double dPhi0, int iNumAdd, double *pdAdd);
    void copy(const ProjType *pt);
    static ProjType *createPT(const char *pPDData, bool bDegrees);

    int     m_iProjType; // -> make sure JavaProg has same indexes
    double  m_dLambda0;
    double  m_dPhi0;
    int     m_iNumAdd;
    double  m_adAdd[MAX_ADD];
    static unsigned int memReq() {return (MAX_ADD+2)*sizeof(double)+2*sizeof(int);};
    unsigned char *serialize(unsigned char *p) const;
    unsigned char *deserialize(unsigned char *p);
    char *toString(bool bDegrees) const;
    int fromString(const char *pLine, bool bDegrees);

    bool isEqual(ProjType *pPT);
};

/** ***************************************************************************\
*   \class  ProjGrid
*   \brief  Container for projector-related parameters
*** ***************************************************************************/
class ProjGrid {
public:
    ProjGrid();
    ProjGrid(int iGridW, int iGridH, double dRealW, double dRealH, double dOffsX, double dOffsY, double dRadius);
    void copy(const ProjGrid *ppg);
    static ProjGrid *createPG(const char *pPGData);

    int    m_iGridW;
    int    m_iGridH;
    double m_dRealW;
    double m_dRealH;
    double m_dOffsX;
    double m_dOffsY;
    double m_dRadius;
    static unsigned int memReq() {return 7*sizeof(double);};
    unsigned char *serialize(unsigned char *p) const;
    unsigned char *deserialize(unsigned char *p);
    char *toString() const;
    int fromString(const char *pLine);

    bool isEqual(ProjGrid *pPG);
};    


/** ***************************************************************************\
*   \class  GeoInfo
*   \brief  A factory for Projector objects
*** ***************************************************************************/
class GeoInfo {

public:
    /** ***************************************************************************\
    *   \fn     instance();
    *   \brief  get instance of GeoInfo
    *
    *   \return GeoInfo singleton instance
    *   Returns GeoInfo singleton. Creates it if necessary
    *** ***************************************************************************/
    static GeoInfo *instance();

    /** ***************************************************************************\
    *   \fn     free();
    *   \brief  frees GeoInfo singleton
    *
    *   Frees GeoInfo singleton. Sets it to NULL
    *** ***************************************************************************/
    static void free();

    /** ***************************************************************************\
    *   \fn     Projector *createProjector(int iType, 
    *                                      double dLambda0,
    *                                      double dPhi);
    *   \brief  factory method
    *
    *   \param  iType       type of desired Projector (one of the PR_XXX constants)
    *   \param  dLambda0    longitude of projection center
    *   \param  dPhi0       latitude  of projection center
    *
    *   \return Projector object
    *** ***************************************************************************/
    Projector   *createProjector(int iType, double dLambda0, double dPhi0);

    /** ***************************************************************************\
    *   \fn     Projector *createProjector(ProjType *pPD);
    *   \brief  factory method
    *
    *   \param  pPD         a ProjGrid object describing the desired Projector
    *
    *   \return Projector object
    *** ***************************************************************************/
    Projector   *createProjector(const ProjType *pPT);

    
    /** ***************************************************************************\
    *   \fn     const char *getName(int iIndex)
    *   \brief  get name of projection with index iIndex
    *
    *   \param  iIndex      one of the PR_XXX constants
    *
    *   \return Name of Projector class defined by iIndex
    *** ***************************************************************************/
    static const char *getName(int iIndex) {
        return asProjNames[iIndex];
    }

    /** ***************************************************************************\
    *   \fn     static int getNumProj()
    *   \brief  get number of projections
    *
    *   \return number of available projections
    *** ***************************************************************************/
    static int getNumProj() {
        return sizeof(asProjNames)/sizeof(char *);
    }

    /** ***************************************************************************\
    *   \fn     static GeoProvider *createGeoProvider(const ProjType *pPT, const ProjGrid *pPG);
    *   \brief  create a GeoProvider
    *
    *   \param  pPT             projection type
    *   \param  pPG             projection grid
    *
    *   \return GeoProvider created from params
    *** ***************************************************************************/
    static GeoProvider *createGeoProvider(const ProjType *pPT, const ProjGrid *pPG);

protected:       
    GeoInfo();
    static GeoInfo *s_pGeoInfo; 
};




#endif
