/** ***************************************************************************\
*   \file   DEM.h
*   \author jody
*   \brief  Header file for class DEM
*
*   DEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.

*** ***************************************************************************/
#ifndef __DEM_H__
#define __DEM_H__

#include "utils.h"
const double NO_VAL = dNaN;


//const double NO_VAL = -9999999;

/** ***************************************************************************\
*   \class  DEM
*   \brief  Digital Elevation Model
*
*   DEM is an abstract base class for Digital Elevation Models.
*   Derived classes differ in data acquisition.
*   DEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   The data is read from a file of the form
*   <long>, <lat>, <alt>
*** ***************************************************************************/
class DEM {
public:
    /** ***************************************************************************\
    *   \fn     DEM()
    *   \brief  constructor
    *
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    DEM(int iNumLonVals, int iNumLatVals);
    virtual ~DEM() {};

    /** ***************************************************************************\
    *   \fn     bool load(char *pName)=0;
    *   \brief  read data from file or analyze file
    *
    *   \param  pName           file name
    *   
    *   \return false if read error
    *
    *   It is asasumed that the data describes a "rectangular" area, i.e
    *   for each lonitude value there is the same number of latitude values 
    *   and vice versa. These numbers are specified by iNumLatVals and iNumLongVals
    *
    *   This method must be implemented in derived classes.
    *** ***************************************************************************/
    virtual bool load(char *pName)=0;
    
    /** ***************************************************************************\
    *   \fn     double getAltitude(double dLon, double dLat)=0;
    *   \brief  get altitude for specified location
    *
    *   \param  dLon    longitude (Degrees)
    *   \param  dLat    latitude  (Degrees)
    *   
    *   \return altitude or NO_VAL, if location outside map
    *
    *
    *   This method must be implemented in derived classes.
    *** ***************************************************************************/
    virtual double getAltitude(double dLon, double dLat)=0;

    /** ***************************************************************************\
    *   \fn     double splitLine(char *pLine)
    *   \brief  split a line to get lat, long and alt
    *
    *   \param  pLine   current line read from file
    *   
    *   \return altitude or NO_VAL, if location outside map
    *
    *   This method additionally sets min and max values for lat and lon
    *** ***************************************************************************/
    double splitLine(char *pLine);

    /** ***************************************************************************\
    *   \fn     bool findIndex(double dLon, 
    *                          double dLat, 
    *                          int &iLonIndex, 
    *                          int &iLatIndex);
    *   \brief  find latitude and longitude index for lat and lon
    *
    *   \param  dLon        longitude (Degrees)
    *   \param  dLat        latitude  (Degrees)
    *   \param  iLonIndex   longitude index (output)
    *   \param  iLatIndex   latitude index (output)
    *   
    *   \return true if index found
    *
    *** ***************************************************************************/
    bool findIndex(double dLon, double dLat, int &iLonIndex, int &iLatIndex);

    bool hadBadIndex() {return m_bBadIndex; };

protected:    
    double m_dMinLat;
    double m_dMaxLat;
    double m_dMinLon;
    double m_dMaxLon;
    double m_dDeltaLat;
    double m_dDeltaLon;
    int    m_iNumLatVals;
    int    m_iNumLonVals;

    bool   m_bBadIndex;
};


#endif
