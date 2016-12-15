/*****************************************************************************\
| AltAsc reads a binary  raster file of altitude values for the entire globe 
\*****************************************************************************/

#ifndef __QMAPREADER_H__
#define __QMAPREADER_H__

#include <stdio.h>
#include "ValReader.h"

class QMapHeader;
class Interpolator;

/** ***************************************************************************\
*   \class  QMapReader
*   \brief  template class for the reading of a binary file and extraction
*           of a range.
*           
*
*   It is assumed that the binary file represents (geographical) data 
*   arranged on a grid of longitude and latitude.
*   Normally such data is given for a grid with half-degree resolution
*   and 0.25°phase shift for the entire globe, 
*   i.e. the minimal grid value is -179.5° and the maximal grid value is +179.5° 
*   for longitude, and -89.75° and +89.75 are the respective grtid values
*   for latitude.
*
*** ***************************************************************************/

template<class T> class QMapReader : public ValReader {
protected:
    T          **m_aatData;
    double     **m_aadData;
    double       m_dGridLonMin;
    double       m_dGridLatMin;
    unsigned int m_iNLon;
    unsigned int m_iNLat;
    unsigned int m_iNRLon;
    unsigned int m_iNRLat;
    unsigned int m_iStartCol;
    unsigned int m_iStartRow;
    bool       m_bInterpolate;
private:
    FILE      *m_fIn;
    double    m_dDLon;
    double    m_dDLat;

    double    m_dDataLonMin;
    double    m_dDataLonMax;
    double    m_dDataLatMin;
    double    m_dDataLatMax;
    double    m_dGridPhaseLon; // positive gridpoint closest to 0
    double    m_dGridPhaseLat; // positive gridpoint closest to 0

    double    m_dExtrLonMin;
    double    m_dExtrLonMax;
    double    m_dExtrLatMin;
    double    m_dExtrLatMax;

    double *m_pdLonGrids;
    double *m_pdLatGrids;
    Interpolator *m_pIP;
    bool m_bReverseLat;
    bool m_bNanOutside;

    unsigned int m_iHeaderSize;
    char *m_pFileName;

    double m_dMin;
    double m_dMax;

    char m_sVName[8];
    char m_sXName[8];
    char m_sYName[8];

    bool   m_bSubRegion;
    int    m_iNumIgnoreItems;

public:
 
    /** ***************************************************************************\
    *   \fn     QMapReader(char *pFileName, int iHeaderSize,
    *                     double fDLon, double fDataLonMin, double fDataLonMax, 
    *                     double fDLat, double fDataLatMin, double fDataLatMax, 
    *                     bool bInterpolate=false);
    *   \brief  reads a binary file of geographic data and uses entire data
    *
    *   \param  pFileName     name of input file
    *   \param  fDLon         grid resolution for longitude (in °)
    *   \param  fDataLonMin   smallest Longitude grid value  (in °)
    *   \param  fDataLonMax   value larger than the highest Longitude grid value (in °)
    *   \param  fDLat         grid resolution for latitude (in °)
    *   \param  fDataLatMin   smallest Latitude grid value (in °)
    *   \param  fDataLatMax   value larger than the highest Latitude grid value (in °)
    *   \param  bInterpolate  apply interpolation
    *   
    *** ***************************************************************************/
    QMapReader(const char *pFileName, int iHeaderSize,
               double fDLon, double fDataLonMin, double fDataLonMax, 
               double fDLat, double fDataLatMin, double fDataLatMax, 
               bool bInterpolate=false);

   /** ***************************************************************************\
    *   \fn     QMapReader(char *pFileName
    *                     double fDataLonMin, double fDataLonMax, 
    *                     double fDataLatMin, double fDataLatMax, 
    *                     bool bInterpolate=false);
    *   \brief  extracts given region from a QMAP file
    *
    *   \param  pFileName     name of input file
    *   \param  fDataLonMin   smallest Longitude grid value  (in °)
    *   \param  fDataLonMax   value larger than the highest Longitude grid value (in °)
    *   \param  fDataLatMin   smallest Latitude grid value (in °)
    *   \param  fDataLatMax   value larger than the highest Latitude grid value (in °)
    *   \param  bInterpolate  apply interpolation
    *   
    *** ***************************************************************************/
    QMapReader(const char *pFileName,
               double fDataLonMin, double fDataLonMax, 
               double fDataLatMin, double fDataLatMax, 
               bool bPrepareArrays=true,
               bool bInterpolate=false);

   /** ***************************************************************************\
    *   \fn     QMapReader(char *pFileName
    *                      const char *pRange, 
    *                      bool bInterpolate=false);
    *   \brief  extracts region defined in string pRange from a QMAP file
    *
    *   \param  pFileName     name of input file
    *   \param  pRange        region string; format <lonmin>:<lonmax>:<latmin><latmax>
    *   \param  bInterpolate  apply interpolation
    *   
    *** ***************************************************************************/
    QMapReader(const char *pFileName,
               const char *pRange, 
               bool bPrepareArrays=true,
               bool bInterpolate=false);

    /** ***************************************************************************\
    *   \fn     QMapReader(char *pFileName, bool bInterpolate=false);
    *   \brief  reads a qmap file and uses entire data
    *
    *   \param  pFileName     name of input file
    *   \param  bInterpolate  apply interpolation
    *   
    *   only to be used with valid QHG binary maps
    *** ***************************************************************************/
    QMapReader(const char *pFileName,
               bool bInterpolate=false);


    virtual ~QMapReader();
    
    void initialize(const char *pFileName, bool bPrepareArrays, bool bInterpolate);

    int splitRange(const char *pRnge);
    //    static double getPhase(double fMin, double fD);
	
    //   static ValReader *createValReader(const char *pFile, bool bInterp);

    
    virtual double getNextGridPointLon(double fM);
    
    virtual double getNextGridPointLat(double fM);

    double X2Lon(double iX) const;
    double Y2Lat(double iY) const;
    double Lon2X(double fLon) const;
    double Lat2Y(double fLat) const;
    virtual bool isOK() { return (m_fIn != NULL);};
    virtual char *getFileName() const {return m_pFileName;};
    virtual double getDLon() {return m_dDLon;};
    virtual double getDLat() {return m_dDLat;};
    virtual double getLonMin() { return m_dDataLonMin;};
    virtual double getLonMax() { return m_dDataLonMax;};
    virtual double getLatMin() { return m_dDataLatMin;};
    virtual double getLatMax() { return m_dDataLatMax;};


    T      **getData() {return m_aatData;};
    virtual T      getValue(double dLon, double dLat);
    virtual double getDValue(double dLon, double dLat);
    virtual double getDValue(unsigned int iX, unsigned int iY);
    
    virtual unsigned int getNLon() const { return m_iNLon; };
    virtual unsigned int getNLat() const { return m_iNLat; };
    virtual unsigned int getNRLon() const { return m_iNRLon; };
    virtual unsigned int getNRLat() const { return m_iNRLat; };
    virtual bool   extractData();
   

    virtual void   scanValues(bool bNormal=true);
    virtual double getMin() const { return m_dMin; };
    virtual double getMax() const { return m_dMax; };

    virtual char  *getVName() { return m_sVName;};
    virtual char  *getXName() { return m_sXName;};
    virtual char  *getYName() { return m_sYName;};
    
    virtual int    getDataOffset() { return m_iNumIgnoreItems;};

    virtual bool   sameFormat(ValReader *pVR, bool bStrict=false);


};


#endif
