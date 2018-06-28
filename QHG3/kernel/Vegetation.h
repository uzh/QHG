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
    
    uint m_iNumCells;
    int m_iNumVegSpecies;
    
    double *m_adBaseANPP;
    double *m_adTotalANPP;
    
    int update(float fTime);
    int climateUpdate(float fTime);
    void resetUpdated() {};
 protected:
    double m_fPreviousTime;    
    Geography *m_pGeography;
    Climate   *m_pClimate;

    WELL512 **m_apWELL;    

};




#endif
