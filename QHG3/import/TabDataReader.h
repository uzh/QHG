/*****************************************************************************\
| TabDataReader
| derived from TabReader
| 
| The abstract base class TabDataReader saves the extracted data in a float array
| The method extractLocation must be implemented in derived 
\*****************************************************************************/

#ifndef __TABDATAREADER_H__
#define __TABDATAREADER_H__

#include "SeasonProvider.h"
#include "TabReader.h"

typedef float *PFLOAT;

class TabDataReader : public TabReader, public SeasonProvider {
	
protected:
    float      ***m_afData;
    float      m_fGridLonMin;
    float      m_fGridLatMin;
    int        m_iNLon;
    int        m_iNLat;
    int        m_iNumVals;
private:
    float      m_fGridPhaseLon; // positive gridpoint closest to 0
    float      m_fGridPhaseLat; // positive gridpoint closest to 0
public:
    TabDataReader(char *pFileName, 
                  float fDataLonMin, float fDLon, float fRangeLonMin, float fRangeLonMax,
                  float fDataLatMin, float fDLat, float fRangeLatMin, float fRangeLatMax,
                  int   iNumVals);
    virtual ~TabDataReader();
    
    static float getPhase(float fMin, float fD);
	
    float getNextGridPointLon(float fM);
    
    float getNextGridPointLat(float fM);

protected:
    virtual bool action(float fLon, float fLat, char *pLine);
    virtual bool extractLocation(char *pLine, float *pfLon, float *pfLat) = 0;

public:
    float getValue(float fLon, float fLat, int iIndex);
    int calcLonIndex(float fLon);

    float calcLon(int iLonIndex);
    int calcLatIndex(float fLat);
    float calcLat(int iLatIndex);
    float getValueBiLin(float fLon, float fLat, int iIndex=0);
    float **getData(int iIndex=0) { return m_afData[iIndex]; };
    float ***getAllData() { return m_afData; };
    int   getNLon() { return m_iNLon; };
    int   getNLat() { return m_iNLat; };
	
    float X2Lon(float iX);
    float Y2Lat(float iY);


};

#endif
