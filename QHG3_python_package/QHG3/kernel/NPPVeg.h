#ifndef __VEGETATION_H__
#define __VEGETATION_H__

#include "WELL512.h"

class Geography;
class Climate;
class SCellGrid;

typedef double         veginumber;


class Vegetation {
 public:
    Vegetation(uint iNumCells, int iNumVegSpecies, Geography *pGeography, Climate *pClimate);
    virtual ~Vegetation();
    
    bool *m_abDynamic;
    bool m_bUpdated;
    uint m_iNumCells;
    int m_iNumVegSpecies;
    
    double **m_adMass;
    double **m_adANPP;
	double *m_adVariance;
    
    int update(float fTime);
    int climateUpdate(float fTime);

    inline void resetUpdated() { m_bUpdated = false; };

    void writeOutput(float fTime, int iStep, SCellGrid* pCG);
    
 protected:
    void updateClimateANPP(int iSpecies);
    double computeDetritusP(double dANPP);
    double m_fPreviousTime;    
    double **createArray();
    void deleteArray(double **adData);
    Geography *m_pGeography;
    Climate   *m_pClimate;

    void writeVegQMap(float fTime, int iStep);
    void writeVegSnap(float fTime, int iStep, SCellGrid* pCG);
	WELL512 **m_apWELL;    

};




#endif
