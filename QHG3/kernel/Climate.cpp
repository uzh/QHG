#include <stdio.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "ValReader.h"
#include "Geography.h"

#include "SeasonProvider.h"

#include "Climate.h"



#ifndef NULL 
  #define NULL 0
#endif

//-----------------------------------------------------------------------------
// constructor
//
Climate::Climate(uint iNumCells, climatecount iNumSeasons, Geography *pGeography)
    : m_bDynamic(false),
      m_bUpdated(true),
      m_iNumCells(iNumCells),     
      m_iNumSeasons(iNumSeasons), 
      m_pGeography(pGeography),
      m_iSeasonMonths(0),
      m_iCurSeason(0),
      m_adActualTemps(NULL),
      m_adActualRains(NULL),    
      m_adAnnualMeanTemp(NULL),
      m_adAnnualRainfall(NULL),
      m_aadSeasonalTempD(NULL),
      m_aadSeasonalRainR(NULL),
      m_adSeasTempDiff(NULL),  
      m_adSeasRainRatio(NULL) {

    if (m_iNumSeasons > 0) {
        m_iSeasonMonths = (climatecount)(12/m_iNumSeasons);
    }
 
    prepareArrays();
 
}


//-----------------------------------------------------------------------------
// prepareArrays
//
void Climate::prepareArrays() {
    m_adActualTemps    = new climatenumber[m_iNumCells];
    m_adActualRains    = new climatenumber[m_iNumCells];

    m_adAnnualMeanTemp = new climatenumber[m_iNumCells];
    m_adAnnualRainfall = new climatenumber[m_iNumCells];

    if (m_iNumSeasons > 0) {
        m_adSeasTempDiff  = new climatenumber[m_iNumSeasons*m_iNumCells];
        m_adSeasRainRatio = new climatenumber[m_iNumSeasons*m_iNumCells];
  

        m_aadSeasonalTempD = new climatenumber*[m_iNumCells];
        m_aadSeasonalRainR = new climatenumber*[m_iNumCells];

        for (uint i = 0; i < m_iNumCells; i++) {
            m_aadSeasonalTempD[i] = m_adSeasTempDiff  + i*m_iNumSeasons;
            m_aadSeasonalRainR[i] = m_adSeasRainRatio + i*m_iNumSeasons;
        }
    }
}


//-----------------------------------------------------------------------------
// destructor
//
Climate::~Climate() {
    
    if (m_adAnnualMeanTemp != NULL) {
        delete[] m_adAnnualMeanTemp;
    }

    if (m_adAnnualRainfall != NULL) {
        delete[] m_adAnnualRainfall;
    }
  
    if (m_adActualTemps != NULL) {
        delete[] m_adActualTemps;
    }
        
    if (m_adActualRains != NULL) {
        delete[] m_adActualRains;
    }
    
    
    if (m_iNumSeasons > 0) {
        if (m_aadSeasonalTempD != NULL) {
            /*
            for (int i = 0; i < m_iNumCells; i++) {
                if (m_aadSeasonalTempD[i] != NULL) {
                    delete[] m_aadSeasonalTempD[i];
                }
            }
            */
            delete[] m_aadSeasonalTempD;
        }
        
        if (m_aadSeasonalRainR != NULL) {
            /*
            for (int i = 0; i < m_iNumCells; i++) {
                if (m_aadSeasonalRainR[i] != NULL) {
                    delete[] m_aadSeasonalRainR[i];
                }
            }
            */
            delete[] m_aadSeasonalRainR;
        }
        delete[] m_adSeasTempDiff;  
        delete[]  m_adSeasRainRatio;
    }
    
}


//-----------------------------------------------------------------------------
// setSeason
//   update actual temperatures and rainfalls for new season
//
void Climate::setSeason(climatecount iSeason) {

    if (m_iNumSeasons > 0) {
        m_iCurSeason = iSeason;
        
        for (uint i = 0; i < m_iNumCells; i++) {
            m_adActualTemps[i] = m_adAnnualMeanTemp[i] + m_aadSeasonalTempD[i][m_iCurSeason];
            m_adActualRains[i] = m_adAnnualRainfall[i] * m_aadSeasonalRainR[i][m_iCurSeason];
        }
    } else {
        for (uint i = 0; i < m_iNumCells; i++) {
            m_adActualTemps[i] = m_adAnnualMeanTemp[i];
            m_adActualRains[i] = m_adAnnualRainfall[i];
        }
    }
}



//-----------------------------------------------------------------------------
// updateAnnuals
//
void Climate::updateAnnuals(ValReader *pVRTemp, ValReader *pVRRain) {
    for (uint i = 0; i < m_iNumCells; i++) {
        double dLat = m_pGeography->m_adLatitude[i];
        double dLon = m_pGeography->m_adLongitude[i];
        m_adAnnualMeanTemp[i] = pVRTemp->getDValue(dLon, dLat);
        m_adAnnualRainfall[i] = pVRRain->getDValue(dLon, dLat);
     }
    setSeason(m_iCurSeason);
}

//-----------------------------------------------------------------------------
// initSeasons
//
void Climate::initSeasons(SeasonProvider *pspTemps, SeasonProvider *pspRains) {
    int iSeasonMonths = 12 / m_iNumSeasons;
    for (uint iIndex = 0; iIndex < m_iNumCells; iIndex++) {
        double dLongitude = m_pGeography->m_adLongitude[iIndex];
        double dLatitude  = m_pGeography->m_adLatitude[iIndex];

        for (uint j = 0; j < m_iNumSeasons; j++) {
            m_aadSeasonalTempD[iIndex][j] = 0;
            m_aadSeasonalRainR[iIndex][j] = 0;
        }
        
        double dAnnualMeanTemp = 0;
        double dAnnualRainfall = 0;
        for (int i = 0; i < 12; i++) {
            float fTemp = pspTemps->getValueBiLin(dLongitude, dLatitude, i);
            m_aadSeasonalTempD[iIndex][i/iSeasonMonths] += fTemp;
            dAnnualMeanTemp += fTemp;
            float fRain = pspRains->getValueBiLin(dLongitude, dLatitude, i);
            m_aadSeasonalRainR[iIndex][i/iSeasonMonths] += fRain;
            dAnnualRainfall += fRain;
        }
        // make mean
        dAnnualMeanTemp /= 12;
        
        for (uint j = 0; j < m_iNumSeasons; j++) {
            m_aadSeasonalTempD[iIndex][j] -= dAnnualMeanTemp;
            if (dAnnualRainfall > 0) {
                m_aadSeasonalRainR[iIndex][j] /= dAnnualRainfall;
            } else {
                m_aadSeasonalRainR[iIndex][j] = 0;
            }
        }
    }
}

