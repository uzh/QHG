#ifndef __VEGFACTORY_H__
#define __VEGFACTORY_H__

#include "LineReader.h"
#include "Geography.h"
#include "Climate.h"

class Vegetation;

class VegFactory {
public:
    VegFactory(const char *sDefFile, int iNumCells, Geography* pGeo, Climate* pClimate);
    ~VegFactory();

    int readDef();

    Vegetation *getVeg() { return m_pVeg; };

protected:
    int m_iNumCells;
    int m_iNumVegSpecies;

    Geography* m_pGeo;
    Climate* m_pClimate;
    Vegetation *m_pVeg;
    LineReader *m_pLR;

    void setVegMassFromDensity(ValReader *pVRDens, int iSpecies);
    void setVegANPP(ValReader *pVRANPP, int iSpecies);
    void setVegMassFromDensity(double dDens, int iSpecies);
    void setVegANPP(double dANPP, int iSpecies);
    void setDynamic(int iSpecies, bool bChoose);
};

#endif
