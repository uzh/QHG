/******************************************************************************
| Calculation of biomass distribution.
|   The assumption is that the biomass for a plant type is given as
|     B(loc, plant) = B_0(plant)*f(loc)*A(loc,plant)
|   where f(loc) represents a dependence of biomass on location (e.g. latitude)
|   and A(loc,plant) is the percentage of area at location loc covered
|   by the plant.
|   
|   The class BiomassDist claculates the value B_0 from given f and A
|   with the help of a biome-map
|   covering the entire globe  (lon[-180,180], lat[90, -90]
|
\*****************************************************************************/

#ifndef __BIOMASSDIST_H__
#define __BIOMASSDIST_H__

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include "types.h"
#include "TrinaryFunc.h"
#include "QMapReader.h"
#include "PercentageReader.h"


const int  TYPE_NONE = -1;
const int  TYPE_MASS =  0;
const int  TYPE_DENS =  1;

const char NAME_MASS[] = "mass";
const char NAME_DENS[] = "density";



typedef std::map<std::string, double> MAP_STRINGDOUBLE;


class QMapHeader;

class BiomassDist {
public:
    BiomassDist(char *pBiomeBaseFile);
    virtual ~BiomassDist();


    int    calcGlobalDensity(MAP_STRINGDOUBLE &mTotalMass, VEC_STRINGS &vSpecies,  TrinaryFunc *pFuncLocDep, char *pPercentageFile);
    int    calcGlobalDensity(MAP_STRINGDOUBLE &mTotalMass, VEC_STRINGS &vSpecies,  TrinaryFunc *pFuncLocDep, PercentageReader *pPR);
    int    calcMass(char *pBiomeTargetFile, const char *pSpecies, char *pOutput, int iType);
    double getBaseMass(char *pSpecies) { return m_mDensity[pSpecies];};
    double calcBiomass(double dLon, double dLat);
    double getBiomass(int iX, int iY);
    
    const QMapReader<uchar> *getQMapReader() {return m_pQMRBiomeBase;};

    void setVerbosity(bool bVerbose) {m_bVerbose = bVerbose;}
    QMapHeader *getBMH() { return m_pQMH;};

    bool setParams(TrinaryFunc *pfuncLocDep,  char *pPercentageFile);
    bool setParams(TrinaryFunc *pfuncLocDep,  PercentageReader *pPR);
private:
      
    int calcGlobalDensity(MAP_STRINGDOUBLE &mTotalMass, VEC_STRINGS &vSpecies);

    QMapReader<uchar> *m_pQMRBiomeBase;
    bool m_bReady;
    PercentageReader *m_pPR;
    double *m_adLon;
    MAP_STRINGDOUBLE m_mDensity;
    TrinaryFunc  *m_pfuncLocDep;
 
    double      **m_aadMass;
    double        m_dLastLat;
    double        m_dLastCos;
    int           m_iLastY;
    bool          m_bVerbose;
    QMapHeader   *m_pQMH;
};


#endif
