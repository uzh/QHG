#ifndef __CLIMATE_H__
#define __CLIMATE_H__

#include "types.h"
#include "Observable.h"

typedef uchar   climatecount;
typedef double  climatenumber;

class ValReader;
class SCellGrid;
class Geography;
class SeasonProvider;

#define EVT_CLIMATE_CHANGE 1001

class Climate : public Observable {
public:
    Climate(uint iNumCells, climatecount iNumSeasons, Geography *pGeography);

    virtual ~Climate();

    void prepareArrays();
    void initSeasons(SeasonProvider *pspTemps, SeasonProvider *pspRains);

    void setSeason(climatecount iSeason=0);  
    
    void updateAnnuals(ValReader *pVRTemp, ValReader *pVRRain);

    inline void resetUpdated() { m_bUpdated = false; };

    bool         m_bDynamic;
    bool         m_bUpdated;
    uint         m_iNumCells;        // number of cells
    climatecount m_iNumSeasons;      // number of seasons

    Geography   *m_pGeography;

    climatecount m_iSeasonMonths;    // season size in months (1,2,3,4,6,12)
    climatecount m_iCurSeason;       // current season 0,...m_iSeasonStep-1

    climatenumber  *m_adActualTemps;  // actual temperatures
    climatenumber  *m_adActualRains;  // actual seasonal rainfall

    
    climatenumber  *m_adAnnualMeanTemp;  // current annual mean temperature
    climatenumber  *m_adAnnualRainfall;  // current annual total rainfall
    climatenumber **m_aadSeasonalTempD;  // differences of seasonal temperature to mean annual temperature
    climatenumber **m_aadSeasonalRainR;  // ratios of seasonal rainfall to total annual rainfall

    climatenumber *m_adSeasTempDiff;
    climatenumber *m_adSeasRainRatio;
    // season

};

#endif
